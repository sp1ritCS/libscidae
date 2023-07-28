#include "scidaetext.h"

#include "scidaewidget-alt.h"

typedef struct {
	GString* body;
} ScidaeTextPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (ScidaeText, scidae_text, SCIDAE_TYPE_WIDGET)

enum {
	PROP_BODY = 1,
	N_PROPERTIES
};
static GParamSpec* obj_properties[N_PROPERTIES] = { NULL, };

static void scidae_text_object_finalize(GObject* object) {
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(SCIDAE_TEXT(object));

	g_string_free(priv->body, TRUE);

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
	
	scidae_container_mark_child_remeasure(scidae_widget_get_parent(widget), widget);
}

static void scidae_text_widget_merge_markdown_end(ScidaeWidget* widget, const gchar* text) {
	ScidaeTextPrivate* priv = scidae_text_get_instance_private(SCIDAE_TEXT(widget));
	g_string_append(priv->body, text);
	
	scidae_container_mark_child_remeasure(scidae_widget_get_parent(widget), widget);
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

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_BODY]);
}
