#include "scidaecontext.h"

typedef struct {
	ScidaeFromUnitsFun from_units;
	ScidaeToUnitsFun to_units;
} ScidaeContextPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (ScidaeContext, scidae_context, G_TYPE_OBJECT)

static void scidae_context_object_constructed(GObject* object) {
	ScidaeContext* self = SCIDAE_CONTEXT(object);
	ScidaeContextPrivate* priv = scidae_context_get_instance_private(self);
	
	priv->from_units = scidae_context_get_from_units(self);
	priv->to_units = scidae_context_get_to_units(self);
}

static void scidae_context_class_init(ScidaeContextClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	
	class->get_from_units = NULL;
	class->get_to_units = NULL;

	class->create_text_widget = NULL;
	
	object_class->constructed = scidae_context_object_constructed;
}

static void scidae_context_init(ScidaeContext* self) {
	ScidaeContextPrivate* priv = scidae_context_get_instance_private(self);
	priv->from_units = NULL;
	priv->to_units = NULL;
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
