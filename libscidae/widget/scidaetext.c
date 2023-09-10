#include "scidaetext.h"

#include "scidaewidget-alt.h"
#include "scidaewidget-private.h"
#include <grapheme.h>

typedef struct {
	GString* body;

	GArray* char_breaks;
	GArray* word_breaks;
	GArray* sentence_breaks;

	glong primary_cursor;
	glong secondary_cursor;
} ScidaeTextPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (ScidaeText, scidae_text, SCIDAE_TYPE_WIDGET)

enum {
	PROP_BODY = 1,
	N_PROPERTIES
};
static GParamSpec* obj_properties[N_PROPERTIES] = { NULL, };

static const glong SCIDAE_TEXT_BREAK_ARR_START = 0;
static void scidae_text_regenerate_break(ScidaeText* self, GArray* breaks, size_t(*break_fun)(const char*, size_t)) {
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(SCIDAE_TEXT(self));
	g_array_set_size(breaks, 0);
	g_array_append_val(breaks, SCIDAE_TEXT_BREAK_ARR_START);
	for (gsize i = 0; i < priv->body->len; ) {
		i += break_fun(&priv->body->str[i], priv->body->len - i);
		g_array_append_val(breaks, i);
	}
}

static void scidae_text_body_modified(ScidaeText* self) {
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(SCIDAE_TEXT(self));
	if (priv->body->len == 0) {
		g_info("Text widget %p is empty, dropping widget.\n", (void*)self);
		scidae_widget_set_parent((ScidaeWidget*)self, NULL);
		return;
	}

	scidae_text_regenerate_break(self, priv->char_breaks, grapheme_next_character_break_utf8);
	scidae_text_regenerate_break(self, priv->word_breaks, grapheme_next_word_break_utf8);
	scidae_text_regenerate_break(self, priv->sentence_breaks, grapheme_next_sentence_break_utf8);

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_BODY]);
	ScidaeWidget* widget = SCIDAE_WIDGET(self);
	ScidaeContainer* parent = scidae_widget_get_parent(widget);
	if (parent)
		scidae_container_mark_child_remeasure(parent, widget);
}

static void scidae_text_object_finalize(GObject* object) {
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(SCIDAE_TEXT(object));

	g_string_free(priv->body, TRUE);
	g_array_free(priv->char_breaks, TRUE);
	g_array_free(priv->word_breaks, TRUE);
	g_array_free(priv->sentence_breaks, TRUE);

	G_OBJECT_CLASS(scidae_text_parent_class)->finalize(object);
}

static void scidae_text_object_get_property(GObject* object, guint prop_id, GValue* val, GParamSpec* pspec) {
	ScidaeText* self = SCIDAE_TEXT(object);
	
	switch (prop_id) {
		case PROP_BODY:
			g_value_set_string(val, scidae_text_get_body(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void scidae_text_object_set_property(GObject* object, guint prop_id, const GValue* val, GParamSpec* pspec) {
	ScidaeText* self = SCIDAE_TEXT(object);
	
	switch (prop_id) {
		case PROP_BODY:
			scidae_text_set_body(self, g_value_get_string(val));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static gchar* scidae_text_widget_get_markdown(ScidaeWidget* widget) {
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(SCIDAE_TEXT(widget));
	return g_strdup(priv->body->str);
}

static void scidae_text_widget_merge_markdown_start(ScidaeWidget* widget, const gchar* text) {
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(SCIDAE_TEXT(widget));
	g_string_prepend(priv->body, text);
	
	scidae_text_body_modified(SCIDAE_TEXT(widget));
}

typedef enum {
	SCIDAE_TEXT_MOVEMENT_BACKWARD = -1,
	SCIDAE_TEXT_MOVEMENT_NONE = 0,
	SCIDAE_TEXT_MOVEMENT_FORWARD = 1
} ScidaeTextMovementDirection;

static void scidae_text_widget_merge_markdown_end(ScidaeWidget* widget, const gchar* text) {
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(SCIDAE_TEXT(widget));
	g_string_append(priv->body, text);
	
	scidae_text_body_modified(SCIDAE_TEXT(widget));
}

static void scidae_text_widget_set_cursor_start(ScidaeWidget* widget) {
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(SCIDAE_TEXT(widget));
	priv->primary_cursor = 0;
	priv->secondary_cursor = 0;
}

static void scidae_text_widget_set_cursor_end(ScidaeWidget* widget) {
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(SCIDAE_TEXT(widget));
	priv->primary_cursor = priv->body->len;
	priv->secondary_cursor = priv->body->len;
}

static void scidae_text_widget_drop_cursor(ScidaeWidget* widget) {
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(SCIDAE_TEXT(widget));
	priv->primary_cursor = -1;
	priv->secondary_cursor = -1;
}

static ScidaeTextMovementDirection scidae_text_cursor_move_fun(G_GNUC_UNUSED ScidaeText* self, ScidaeTextMovementDirection direction, glong* target, GArray* breaks) {
	g_assert(direction != SCIDAE_TEXT_MOVEMENT_NONE);

	gint low = 0;
	gint high = breaks->len;

	while (low <= high) {
		gint mid = low + (high - low) / 2;
		glong cur = g_array_index(breaks, glong, mid);

		if (cur == *target) {
			if (direction == SCIDAE_TEXT_MOVEMENT_FORWARD) {
				if (mid + 1 < (gint)breaks->len) {
					*target = g_array_index(breaks, glong, mid + 1);
					return SCIDAE_TEXT_MOVEMENT_NONE;
				} else {
					return SCIDAE_TEXT_MOVEMENT_FORWARD;
				}
			} else if (direction == SCIDAE_TEXT_MOVEMENT_BACKWARD) {
				if (mid - 1 >= 0) {
					*target = g_array_index(breaks, glong, mid - 1);
					return SCIDAE_TEXT_MOVEMENT_NONE;
				} else {
					return SCIDAE_TEXT_MOVEMENT_BACKWARD;
				}
			} else {
				__builtin_unreachable();
			}
		} else if (cur < *target) {
			low = mid + 1;
		} else {
			high = mid - 1;
		}
	}

	if (direction == SCIDAE_TEXT_MOVEMENT_FORWARD) {
		if (low < (gint)breaks->len) {
			*target = g_array_index(breaks, glong, low);
			return SCIDAE_TEXT_MOVEMENT_NONE;
		} else {
			return SCIDAE_TEXT_MOVEMENT_FORWARD;
		}
	} else if (direction == SCIDAE_TEXT_MOVEMENT_BACKWARD) {
		if (high >= 0) {
			*target = g_array_index(breaks, glong, high);
			return SCIDAE_TEXT_MOVEMENT_NONE;
		} else {
			return SCIDAE_TEXT_MOVEMENT_BACKWARD;
		}
	} else {
		__builtin_unreachable();
	}
}

static void scidae_text_move_cursor_generic(ScidaeText* self, ScidaeTextMovementDirection direction, ScidaeWidgetMovementModifier modifiers, ScidaeWidgetCursorType cursor) {
	g_assert(direction != SCIDAE_TEXT_MOVEMENT_NONE);
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);
	glong* cursor_pos;
	switch (cursor) {
		case SCIDAE_CURSOR_TYPE_PRIMARY:
			cursor_pos = &priv->primary_cursor;
			break;
		case SCIDAE_CURSOR_TYPE_SECONDARY:
			cursor_pos = &priv->secondary_cursor;
			break;
		default:
			g_critical("Found invalid cursor type %d.", cursor);
			return;
	}

	ScidaeTextMovementDirection movdir = SCIDAE_TEXT_MOVEMENT_NONE;
	if (*cursor_pos < 0) {
		if (direction == SCIDAE_TEXT_MOVEMENT_BACKWARD)
			*cursor_pos = priv->body->len;
		else if (direction == SCIDAE_TEXT_MOVEMENT_FORWARD) {
			*cursor_pos = 0;
			movdir = scidae_text_cursor_move_fun(self, direction, cursor_pos, priv->char_breaks);
		}
	} else if (
		(direction == SCIDAE_TEXT_MOVEMENT_BACKWARD && *cursor_pos == 0) ||
		(direction == SCIDAE_TEXT_MOVEMENT_FORWARD && *cursor_pos == (glong)priv->body->len)
	) {
		movdir = direction;
	} else
		movdir = scidae_text_cursor_move_fun(self, direction, cursor_pos, priv->char_breaks);

	ScidaeWidget* widget = SCIDAE_WIDGET(self);
	switch (movdir) {
		case SCIDAE_TEXT_MOVEMENT_BACKWARD: {
			ScidaeContainer* parent = scidae_widget_get_parent(widget);
			ScidaeWidget* prev = scidae_container_get_prev(parent, widget);
			if (prev) {
				scidae_widget_move_cursor_backward(prev, modifiers, cursor);
				*cursor_pos = -1;
				scidae_container_update_cursor(parent, prev);
			} else {
				*cursor_pos = 0;
			}
		} break;
		case SCIDAE_TEXT_MOVEMENT_FORWARD: {
			ScidaeContainer* parent = scidae_widget_get_parent(widget);
			ScidaeWidget* next = scidae_container_get_next(parent, widget);
			if (next) {
				scidae_widget_move_cursor_forward(next, modifiers, cursor);
				*cursor_pos = -1;
				scidae_container_update_cursor(parent, next);
			} else {
				*cursor_pos = priv->body->len;
			}
		} break;
		default:
			(void)0;
	}

	scidae_container_mark_child_remeasure(scidae_widget_get_parent(widget), widget);
}

static void scidae_text_widget_move_cursor_backward(ScidaeWidget* widget, ScidaeWidgetMovementModifier modifiers, ScidaeWidgetCursorType cursor) {
	ScidaeText* self = SCIDAE_TEXT(widget);
	scidae_text_move_cursor_generic(self, SCIDAE_TEXT_MOVEMENT_BACKWARD, modifiers, cursor);
}
static void scidae_text_widget_move_cursor_forward(ScidaeWidget* widget, ScidaeWidgetMovementModifier modifiers, ScidaeWidgetCursorType cursor) {
	ScidaeText* self = SCIDAE_TEXT(widget);
	scidae_text_move_cursor_generic(self, SCIDAE_TEXT_MOVEMENT_FORWARD, modifiers, cursor);
}

static void scidae_text_widget_insert_at_cursor(ScidaeWidget* widget, const gchar* text, gssize len) {
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(SCIDAE_TEXT(widget));
	if (priv->primary_cursor < 0) {
		g_critical("Attempt to insert into widget without cursor.");
		return;
	}

	g_string_insert_len(priv->body, priv->primary_cursor, text, len);
	priv->primary_cursor += len >= 0 ? len : strlen(text);

	scidae_text_body_modified(SCIDAE_TEXT(widget));
}

static void scidae_text_delete_from_neighbor_widget(ScidaeText* self, ScidaeTextMovementDirection direction, ScidaeWidgetMovementModifier modifiers) {
	g_assert(direction != SCIDAE_TEXT_MOVEMENT_NONE);

	ScidaeContainer* parent = scidae_widget_get_parent((ScidaeWidget*)self);
	switch (direction) {
		case SCIDAE_TEXT_MOVEMENT_BACKWARD: {
			ScidaeWidget* prev = scidae_container_get_prev(parent, (ScidaeWidget*)self);
			if (!prev)
				return;
			scidae_widget_set_cursor_end(prev);
			scidae_container_update_cursor(parent, prev);
			scidae_widget_delete_backward(prev, modifiers);
		} break;
		case SCIDAE_TEXT_MOVEMENT_FORWARD: {
			ScidaeWidget* next = scidae_container_get_next(parent, (ScidaeWidget*)self);
			if (!next)
				return;
			scidae_widget_set_cursor_start(next);
			scidae_container_update_cursor(parent, next);
			scidae_widget_delete_backward(next, modifiers);
		} break;
		default:
			g_return_if_reached();
	}
}

static void scidae_text_delete_section_from_breaks(ScidaeText* self, ScidaeTextMovementDirection direction, ScidaeWidgetMovementModifier modifiers, glong target, GArray* breaks) {
	g_assert(direction != SCIDAE_TEXT_MOVEMENT_NONE);
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);

	gsize left;
	gsize right;

	gint low = 0;
	gint high = breaks->len;

	while (low <= high) {
		guint mid = low + (high-low)/2;
		glong cur = g_array_index(breaks, glong, mid);

		if (cur == target) {
			if (direction == SCIDAE_TEXT_MOVEMENT_BACKWARD) {
				right = mid;
				if ((gint)mid - 1 >= 0) {
					left = mid - 1;
					goto index_set;
				} else {
					scidae_text_delete_from_neighbor_widget(self, SCIDAE_TEXT_MOVEMENT_BACKWARD, modifiers);
					return;
				}
			} else if (direction == SCIDAE_TEXT_MOVEMENT_FORWARD) {
				left = mid;
				if (mid + 1 < breaks->len) {
					right = mid + 1;
					goto index_set;
				} else {
					scidae_text_delete_from_neighbor_widget(self, SCIDAE_TEXT_MOVEMENT_FORWARD, modifiers);
					return;
				}
			} else {
				__builtin_unreachable();
			}
		} else if (cur < target)
			low = mid + 1;
		else
			high = mid - 1;
	}

	if (G_UNLIKELY(high < 0)) {
		scidae_text_delete_from_neighbor_widget(self, SCIDAE_TEXT_MOVEMENT_BACKWARD, modifiers);
		return;
	}
	if (G_UNLIKELY(low >= (gint)breaks->len)) {
		scidae_text_delete_from_neighbor_widget(self, SCIDAE_TEXT_MOVEMENT_FORWARD, modifiers);
		return;
	}
	left = high;
	right = low;
index_set:
	gssize pos = g_array_index(breaks, glong, left);
	gssize len = (gssize)g_array_index(breaks, glong, right) - pos;

	g_string_erase(priv->body, pos, len);
	priv->primary_cursor = pos;
	scidae_text_body_modified(self);
}

static void scidae_text_widget_delete_backward(ScidaeWidget* widget, ScidaeWidgetMovementModifier modifiers) {
	ScidaeText* self = SCIDAE_TEXT(widget);
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);
	scidae_text_delete_section_from_breaks(self, SCIDAE_TEXT_MOVEMENT_BACKWARD, modifiers, priv->primary_cursor, priv->char_breaks);
}

static void scidae_text_widget_delete_forward(ScidaeWidget* widget, ScidaeWidgetMovementModifier modifiers) {
	ScidaeText* self = SCIDAE_TEXT(widget);
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);
	scidae_text_delete_section_from_breaks(self, SCIDAE_TEXT_MOVEMENT_FORWARD, modifiers, priv->primary_cursor, priv->char_breaks);
}

static void scidae_text_class_init(ScidaeTextClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	ScidaeWidgetClass* widget_class = SCIDAE_WIDGET_CLASS(class);
	
	object_class->finalize = scidae_text_object_finalize;
	object_class->get_property = scidae_text_object_get_property;
	object_class->set_property = scidae_text_object_set_property;
	
	widget_class->get_markdown = scidae_text_widget_get_markdown;
	widget_class->merge_markdown_start = scidae_text_widget_merge_markdown_start;
	widget_class->merge_markdown_end = scidae_text_widget_merge_markdown_end;
	
	widget_class->set_cursor_start = scidae_text_widget_set_cursor_start;
	widget_class->set_cursor_end = scidae_text_widget_set_cursor_end;
	widget_class->drop_cursor = scidae_text_widget_drop_cursor;
	widget_class->move_cursor_backward = scidae_text_widget_move_cursor_backward;
	widget_class->move_cursor_forward = scidae_text_widget_move_cursor_forward;
	widget_class->insert_at_cursor = scidae_text_widget_insert_at_cursor;
	widget_class->delete_backward = scidae_text_widget_delete_backward;
	widget_class->delete_forward = scidae_text_widget_delete_forward;

	/**
	 * ScidaeText:body: (attributes org.gtk.Property.get=scidae_text_get_body org.gtk.Property.set=scidae_text_set_body)
	 *
	 * the contents of this widget
	 */
	obj_properties[PROP_BODY] = g_param_spec_string(
		"body",
		"Body",
		"The content of this widget",
		NULL,
		G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY
	);
	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void scidae_text_init(ScidaeText* self) {
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);
	priv->body = g_string_new(NULL);

	priv->char_breaks = g_array_new(FALSE, FALSE, sizeof(gsize));
	priv->word_breaks = g_array_new(FALSE, FALSE, sizeof(gsize));
	priv->sentence_breaks = g_array_new(FALSE, FALSE, sizeof(gsize));

	priv->primary_cursor = -1;
	priv->secondary_cursor = -1;
}

const gchar* scidae_text_get_body(ScidaeText* self) {
	g_return_val_if_fail(SCIDAE_IS_TEXT(self), NULL);
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);
	return priv->body->str;
}

void scidae_text_set_body(ScidaeText* self, const gchar* text) {
	g_return_if_fail(SCIDAE_IS_TEXT(self));
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);
	
	if (g_strcmp0(priv->body->str, text) == 0)
		return;
	g_string_assign(priv->body, text);

	scidae_text_body_modified(self);
}

void scidae_text_get_cursors(ScidaeText* self, glong* primary, glong* secondary) {
	g_return_if_fail(SCIDAE_IS_TEXT(self));
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);

	if (primary)
		*primary = priv->primary_cursor;
	if (secondary)
		*secondary = priv->secondary_cursor;
}

void scidae_text_set_cursor(ScidaeText* self, glong cursor) {
	g_return_if_fail(SCIDAE_IS_TEXT(self));
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);
	priv->primary_cursor = cursor;
	priv->secondary_cursor = cursor;

	ScidaeWidget* widget = (ScidaeWidget*)self;
	ScidaeContainer* parent = scidae_widget_get_parent(widget);
	scidae_container_mark_child_remeasure(parent, widget);
	scidae_container_update_cursor(parent, widget);
}
