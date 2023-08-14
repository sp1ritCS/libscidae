#include "scidaecontext.h"

typedef struct {
	ScidaeFromUnitsFun from_units;
	ScidaeToUnitsFun to_units;

	guint base_font_size;
} ScidaeContextPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (ScidaeContext, scidae_context, G_TYPE_OBJECT)

enum {
	PROP_BASE_FONT_SIZE = 1,
	N_PROPERTIES
};
static GParamSpec* obj_properties[N_PROPERTIES] = { 0, };

static void scidae_context_object_constructed(GObject* object) {
	ScidaeContext* self = SCIDAE_CONTEXT(object);
	ScidaeContextPrivate* priv = scidae_context_get_instance_private(self);
	
	priv->from_units = scidae_context_get_from_units(self);
	priv->to_units = scidae_context_get_to_units(self);
}

static void scidae_context_object_get_property(GObject* object, guint prop_id, GValue* val, GParamSpec* pspec) {
	ScidaeContext* self = SCIDAE_CONTEXT(object);
	switch (prop_id) {
		case PROP_BASE_FONT_SIZE:
			g_value_set_uint(val, scidae_context_get_base_font_size(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void scidae_context_object_set_property(GObject* object, guint prop_id, const GValue* val, GParamSpec* pspec) {
	ScidaeContext* self = SCIDAE_CONTEXT(object);
	switch (prop_id) {
		case PROP_BASE_FONT_SIZE:
			scidae_context_set_base_font_size(self, g_value_get_uint(val));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void scidae_context_class_init(ScidaeContextClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	
	class->get_from_units = NULL;
	class->get_to_units = NULL;

	class->create_text_widget = NULL;
	
	object_class->constructed = scidae_context_object_constructed;
	object_class->get_property = scidae_context_object_get_property;
	object_class->set_property = scidae_context_object_set_property;

	/**
	 * ScidaeContext:base-font-size: (attributes org.gtk.Property.get=scidae_context_get_base_font_size org.gtk.Property.set=scidae_context_set_base_font_size)
	 *
	 * The font size of "normal" text in context units, used to derive the
	 * fontsize of all other text widgets.
	 */
	obj_properties[PROP_BASE_FONT_SIZE] = g_param_spec_uint(
		"base-font-size",
		"Base Fontsize",
		"The font size of \"normal\" text",
		0,
		G_MAXUINT,
		0,
		G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY
	);

	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void scidae_context_init(ScidaeContext* self) {
	ScidaeContextPrivate* priv = scidae_context_get_instance_private(self);
	priv->from_units = NULL;
	priv->to_units = NULL;
	priv->base_font_size = 0;
}
																	
ScidaeFromUnitsFun scidae_context_get_from_units(ScidaeContext* self) {
	g_return_val_if_fail(SCIDAE_IS_CONTEXT(self), NULL);
	ScidaeContextClass* class = SCIDAE_CONTEXT_GET_CLASS(self);
	g_return_val_if_fail(class->get_from_units, NULL);
	return class->get_from_units(self);
}

ScidaeToUnitsFun scidae_context_get_to_units(ScidaeContext* self) {
	g_return_val_if_fail(SCIDAE_IS_CONTEXT(self), NULL);
	ScidaeContextClass* class = SCIDAE_CONTEXT_GET_CLASS(self);
	g_return_val_if_fail(class->get_to_units, NULL);
	return class->get_to_units(self);
}

ScidaeText* scidae_context_create_text_widget(ScidaeContext* self, const gchar* text) {
	g_return_val_if_fail(SCIDAE_IS_CONTEXT(self), NULL);
	ScidaeContextClass* class = SCIDAE_CONTEXT_GET_CLASS(self);
	g_return_val_if_fail(class->get_to_units, NULL);
	return class->create_text_widget(self, text);
}

gdouble scidae_context_from_units(ScidaeContext* self, gint units) {
	g_return_val_if_fail(SCIDAE_IS_CONTEXT(self), NAN);
	ScidaeContextPrivate* priv = scidae_context_get_instance_private(self);
	return priv->from_units(units);
}

gint scidae_context_to_units(ScidaeContext* self, gdouble display) {
	g_return_val_if_fail(SCIDAE_IS_CONTEXT(self), -1);
	ScidaeContextPrivate* priv = scidae_context_get_instance_private(self);
	return priv->to_units(display);
}

guint scidae_context_get_base_font_size(ScidaeContext* self) {
	g_return_val_if_fail(SCIDAE_IS_CONTEXT(self), 0);
	ScidaeContextPrivate* priv = scidae_context_get_instance_private(self);
	return priv->base_font_size;
}

void scidae_context_set_base_font_size(ScidaeContext* self, guint base_font_size) {
	g_return_if_fail(SCIDAE_IS_CONTEXT(self));
	ScidaeContextPrivate* priv = scidae_context_get_instance_private(self);

	if (priv->base_font_size == base_font_size)
		return;

	priv->base_font_size = base_font_size;

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_BASE_FONT_SIZE]);
}
