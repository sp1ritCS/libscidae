#include "scidaetext.h"

#include "scidaewidget-alt.h"
#include "scidaewidget-private.h"
#include <grapheme.h>

typedef struct {
	GString* body;

	GArray* char_breaks;
	GArray* word_breaks;
	GArray* sentence_breaks;

	glong master_cursor;
	glong slave_cursor;
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
		g_info("Text widget %p is empty, dropping widget.", (void*)self);
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

static inline void scidae_text_apply_action_to_cursor(ScidaeText* self, glong* cursor, ScidaeWidgetModifyCursorAction action) {
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);
	switch (action) {
		case SCIDAE_MODIFY_CURSOR_DROP:
			*cursor = -1;
			break;
		case SCIDAE_MODIFY_CURSOR_MOVE_START:
			*cursor = 0;
			break;
		case SCIDAE_MODIFY_CURSOR_MOVE_END:
			*cursor = priv->body->len;
			break;
		case SCIDAE_MODIFY_CURSOR_RESET:
			g_return_if_reached(); // should not be reached as already handled.
	}
}
static void scidae_text_widget_modify_cursor(ScidaeWidget* widget, ScidaeWidgetCursorType cursor, ScidaeWidgetModifyCursorAction action) {
	ScidaeText* self = SCIDAE_TEXT(widget);
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);

	if (action == SCIDAE_MODIFY_CURSOR_RESET) {
		g_return_if_fail((cursor & SCIDAE_CURSOR_TYPE_BOTH) != SCIDAE_CURSOR_TYPE_BOTH);
			if (cursor & SCIDAE_CURSOR_TYPE_SLAVE)
				priv->slave_cursor = priv->master_cursor;
			else if (cursor & SCIDAE_CURSOR_TYPE_MASTER)
				priv->master_cursor = priv->slave_cursor;
		return;
	}

	if (cursor & SCIDAE_CURSOR_TYPE_MASTER)
		scidae_text_apply_action_to_cursor(self, &priv->master_cursor, action);
	if (cursor & SCIDAE_CURSOR_TYPE_SLAVE)
		scidae_text_apply_action_to_cursor(self, &priv->slave_cursor, action);
}

static inline GArray* scidae_text_get_breaks_from_mods(ScidaeText* self, ScidaeWidgetMovementModifier mods) {
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);

	if (mods & SCIDAE_MOVEMENT_MODIFIER_ALT)
		return priv->sentence_breaks;
	if (mods & SCIDAE_MOVEMENT_MODIFIER_CONTROL)
		return priv->word_breaks;
	return priv->char_breaks;
}

static ScidaeTextMovementDirection scidae_text_cursor_move_fun(ScidaeText* self, ScidaeDirection direction, GArray* breaks) {
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);

	gint low = 0;
	gint high = breaks->len;

	while (low <= high) {
		gint mid = low + (high - low) / 2;
		glong cur = g_array_index(breaks, glong, mid);

		if (cur == priv->master_cursor) {
			if (direction == SCIDAE_DIRECTION_FORWARD) {
				if (mid + 1 < (gint)breaks->len) {
					priv->master_cursor = g_array_index(breaks, glong, mid + 1);
					return SCIDAE_TEXT_MOVEMENT_NONE;
				} else {
					return SCIDAE_TEXT_MOVEMENT_FORWARD;
				}
			} else if (direction == SCIDAE_DIRECTION_BACKWARD) {
				if (mid - 1 >= 0) {
					priv->master_cursor = g_array_index(breaks, glong, mid - 1);
					return SCIDAE_TEXT_MOVEMENT_NONE;
				} else {
					return SCIDAE_TEXT_MOVEMENT_BACKWARD;
				}
			} else {
				g_return_val_if_reached(SCIDAE_TEXT_MOVEMENT_NONE);
			}
		} else if (cur < priv->master_cursor) {
			low = mid + 1;
		} else {
			high = mid - 1;
		}
	}

	if (direction == SCIDAE_DIRECTION_FORWARD) {
		if (low < (gint)breaks->len) {
			priv->master_cursor = g_array_index(breaks, glong, low);
			return SCIDAE_TEXT_MOVEMENT_NONE;
		} else {
			return SCIDAE_TEXT_MOVEMENT_FORWARD;
		}
	} else if (direction == SCIDAE_DIRECTION_BACKWARD) {
		if (high >= 0) {
			priv->master_cursor = g_array_index(breaks, glong, high);
			return SCIDAE_TEXT_MOVEMENT_NONE;
		} else {
			return SCIDAE_TEXT_MOVEMENT_BACKWARD;
		}
	} else {
		g_return_val_if_reached(SCIDAE_TEXT_MOVEMENT_NONE);
	}
}

static void scidae_text_widget_move_cursor(ScidaeWidget* widget, ScidaeDirection direction, ScidaeWidgetMovementModifier modifiers, ScidaeWidgetCursorAction action) {
	ScidaeText* self = SCIDAE_TEXT(widget);
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);
	GArray* breaks = scidae_text_get_breaks_from_mods(self, modifiers);

	ScidaeTextMovementDirection movdir = SCIDAE_TEXT_MOVEMENT_NONE;
	if (priv->master_cursor < 0) {
		if (direction == SCIDAE_DIRECTION_BACKWARD)
			priv->master_cursor = priv->body->len;
		else if (direction == SCIDAE_DIRECTION_FORWARD) {
			priv->master_cursor = 0;
			movdir = scidae_text_cursor_move_fun(self, direction, breaks);
		}
	} else if (direction == SCIDAE_DIRECTION_BACKWARD && priv->master_cursor == 0)
		movdir = SCIDAE_TEXT_MOVEMENT_BACKWARD;
	else if (direction == SCIDAE_DIRECTION_FORWARD && priv->master_cursor == (glong)priv->body->len)
		movdir = SCIDAE_TEXT_MOVEMENT_FORWARD;
	else
		movdir = scidae_text_cursor_move_fun(self, direction, breaks);

	switch (movdir) {
		case SCIDAE_TEXT_MOVEMENT_BACKWARD: {
			ScidaeContainer* parent = scidae_widget_get_parent(widget);
			ScidaeWidget* prev = scidae_container_get_prev(parent, widget);
			if (prev) {
				scidae_widget_move_cursor(prev, SCIDAE_DIRECTION_BACKWARD, modifiers, action);
				priv->master_cursor = -1;
				ScidaeWidgetCursorType type = SCIDAE_CURSOR_TYPE_MASTER;
				if (action == SCIDAE_CURSOR_ACTION_MOVE)
					type |= SCIDAE_CURSOR_TYPE_BOTH;
				scidae_container_update_cursor(parent, type, prev);
			} else {
				priv->master_cursor = 0;
			}
		} break;
		case SCIDAE_TEXT_MOVEMENT_FORWARD: {
			ScidaeContainer* parent = scidae_widget_get_parent(widget);
			ScidaeWidget* next = scidae_container_get_next(parent, widget);
			if (next) {
				scidae_widget_move_cursor(next, SCIDAE_DIRECTION_FORWARD, modifiers, action);
				priv->master_cursor = -1;
				ScidaeWidgetCursorType type = SCIDAE_CURSOR_TYPE_MASTER;
				if (action == SCIDAE_CURSOR_ACTION_MOVE)
					type |= SCIDAE_CURSOR_TYPE_BOTH;
				scidae_container_update_cursor(parent, type, next);
			} else {
				priv->master_cursor = priv->body->len;
			}
		} break;
		default:
			(void)0;
	}
	if (action == SCIDAE_CURSOR_ACTION_MOVE)
		priv->slave_cursor = priv->master_cursor;

	scidae_container_mark_child_remeasure(scidae_widget_get_parent(widget), widget);
}

static inline void scidae_text_delete_selection_int(ScidaeText* self) {
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);

	glong lesser = priv->master_cursor;
	glong greater = priv->slave_cursor;
	if (priv->master_cursor > priv->slave_cursor) {
		lesser = priv->slave_cursor;
		greater = priv->master_cursor;
	}
	g_string_erase(priv->body, lesser, greater - lesser);
	priv->master_cursor = lesser;
	priv->slave_cursor = lesser;
}

static void scidae_text_widget_insert_at_cursor(ScidaeWidget* widget, const gchar* text, gssize len) {
	ScidaeText* self = SCIDAE_TEXT(widget);
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);
	if (priv->master_cursor < 0) {
		g_critical("Attempt to insert into widget without cursor.");
		return;
	}

	if (priv->master_cursor != priv->slave_cursor)
		scidae_text_delete_selection_int(self);

	g_string_insert_len(priv->body, priv->master_cursor, text, len);
	priv->master_cursor += len >= 0 ? (guint)len : strlen(text);
	priv->slave_cursor = priv->master_cursor;

	scidae_text_body_modified(self);
}

static void scidae_text_delete_from_neighbor_widget(ScidaeText* self, ScidaeDirection direction, ScidaeWidgetMovementModifier modifiers) {
	ScidaeContainer* parent = scidae_widget_get_parent((ScidaeWidget*)self);
	switch (direction) {
		case SCIDAE_DIRECTION_BACKWARD: {
			ScidaeWidget* prev = scidae_container_get_prev(parent, (ScidaeWidget*)self);
			if (!prev)
				return;
			scidae_widget_modify_cursor(prev, SCIDAE_CURSOR_TYPE_BOTH, SCIDAE_MODIFY_CURSOR_MOVE_END);
			scidae_container_update_cursor(parent, SCIDAE_CURSOR_TYPE_BOTH, prev);
			scidae_widget_delete(prev, SCIDAE_DIRECTION_BACKWARD, modifiers);
		} break;
		case SCIDAE_DIRECTION_FORWARD: {
			ScidaeWidget* next = scidae_container_get_next(parent, (ScidaeWidget*)self);
			if (!next)
				return;
			scidae_widget_modify_cursor(next, SCIDAE_CURSOR_TYPE_BOTH, SCIDAE_MODIFY_CURSOR_MOVE_START);
			scidae_container_update_cursor(parent, SCIDAE_CURSOR_TYPE_BOTH, next);
			scidae_widget_delete(next, SCIDAE_DIRECTION_FORWARD, modifiers);
		} break;
		default:
			g_return_if_reached();
	}
}

static void scidae_text_widget_delete(ScidaeWidget* widget, ScidaeDirection direction, ScidaeWidgetMovementModifier modifiers) {
	ScidaeText* self = SCIDAE_TEXT(widget);
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);

	if (priv->master_cursor != priv->slave_cursor) {
		scidae_text_delete_selection_int(self);
		scidae_text_body_modified(self);
		return;
	}

	GArray* breaks = scidae_text_get_breaks_from_mods(self, modifiers);

	gsize left;
	gsize right;

	gint low = 0;
	gint high = breaks->len;

	while (low <= high) {
		guint mid = low + (high-low)/2;
		glong cur = g_array_index(breaks, glong, mid);

		if (cur == priv->master_cursor) {
			if (direction == SCIDAE_DIRECTION_BACKWARD) {
				right = mid;
				if ((gint)mid - 1 >= 0) {
					left = mid - 1;
					goto index_set;
				} else {
					scidae_text_delete_from_neighbor_widget(self, SCIDAE_DIRECTION_BACKWARD, modifiers);
					return;
				}
			} else if (direction == SCIDAE_DIRECTION_FORWARD) {
				left = mid;
				if (mid + 1 < breaks->len) {
					right = mid + 1;
					goto index_set;
				} else {
					scidae_text_delete_from_neighbor_widget(self, SCIDAE_DIRECTION_FORWARD, modifiers);
					return;
				}
			} else {
				g_return_if_reached();
			}
		} else if (cur < priv->master_cursor)
			low = mid + 1;
		else
			high = mid - 1;
	}

	if (G_UNLIKELY(high < 0)) {
		scidae_text_delete_from_neighbor_widget(self, SCIDAE_DIRECTION_BACKWARD, modifiers);
		return;
	}
	if (G_UNLIKELY(low >= (gint)breaks->len)) {
		scidae_text_delete_from_neighbor_widget(self, SCIDAE_DIRECTION_FORWARD, modifiers);
		return;
	}
	left = high;
	right = low;

index_set: (void)0; // workarround for wierd clang-analyze bug
	gssize pos = g_array_index(breaks, glong, left);
	gssize len = (gssize)g_array_index(breaks, glong, right) - pos;

	g_string_erase(priv->body, pos, len);
	priv->master_cursor = pos;
	priv->slave_cursor = pos;
	scidae_text_body_modified(self);
}

void scidae_text_widget_delete_region(ScidaeWidget* widget, ScidaeWidgetDeleteRegion region) {
	ScidaeText* self = SCIDAE_TEXT(widget);
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);

	glong* cursor = &priv->master_cursor;
	if (*cursor < 0)
		cursor = &priv->slave_cursor;
	g_return_if_fail(*cursor >= 0);

	switch (region) {
		case SCIDAE_DELETE_REGION_CURSOR_FROM_START:
			g_string_erase(priv->body, 0, *cursor - 0);
			*cursor = 0;
			scidae_text_body_modified(self);
			break;
		case SCIDAE_DELETE_REGION_CURSOR_TO_END:
			g_string_erase(priv->body, *cursor, priv->body->len - *cursor);
			*cursor = priv->body->len;
			scidae_text_body_modified(self);
			break;
	}
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
	
	widget_class->modify_cursor = scidae_text_widget_modify_cursor;
	widget_class->move_cursor = scidae_text_widget_move_cursor;
	widget_class->insert_at_cursor = scidae_text_widget_insert_at_cursor;
	widget_class->delete = scidae_text_widget_delete;
	widget_class->delete_region = scidae_text_widget_delete_region;

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

	priv->master_cursor = -1;
	priv->slave_cursor = -1;
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

void scidae_text_get_cursors(ScidaeText* self, glong* master, glong* slave) {
	g_return_if_fail(SCIDAE_IS_TEXT(self));
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);

	if (master)
		*master = priv->master_cursor;
	if (slave)
		*slave = priv->slave_cursor;
}

void scidae_text_set_cursor(ScidaeText* self, ScidaeWidgetCursorAction action, glong cursor) {
	g_return_if_fail(SCIDAE_IS_TEXT(self));
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(self);

	if (action == SCIDAE_CURSOR_ACTION_MOVE) {
		priv->master_cursor = cursor;
		priv->slave_cursor = cursor;

		ScidaeWidget* widget = (ScidaeWidget*)self;
		ScidaeContainer* parent = scidae_widget_get_parent(widget);
		scidae_container_update_cursor(parent, SCIDAE_CURSOR_TYPE_BOTH, widget);
		scidae_container_mark_child_remeasure(parent, widget);
	} else if (action == SCIDAE_CURSOR_ACTION_MOVE_MASTER) {
		priv->master_cursor = cursor;

		ScidaeWidget* widget = (ScidaeWidget*)self;
		ScidaeContainer* parent = scidae_widget_get_parent(widget);
		scidae_container_update_cursor(parent, SCIDAE_CURSOR_TYPE_MASTER, widget);
		scidae_container_mark_child_remeasure(parent, widget);
	}
}
