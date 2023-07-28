#include "scidaepangocontext.h"

#include "scidaepangotext.h"

struct _ScidaePangoContext {
	ScidaeContext parent_instance;
	
	Pango2Context* context;
};

G_DEFINE_TYPE (ScidaePangoContext, scidae_pango_context, SCIDAE_TYPE_CONTEXT)

enum {
	PROP_CONTEXT = 1,
	N_PROPERTIES
};
static GParamSpec* obj_properties[N_PROPERTIES] = { 0, };

static void scidae_pango_context_object_dispose(GObject* object) {
	ScidaePangoContext* self = SCIDAE_PANGO_CONTEXT(object);
	g_clear_object(&self->context);

	G_OBJECT_CLASS(scidae_pango_context_parent_class)->dispose(object);
}

static void scidae_pango_context_object_get_property(GObject* object, guint prop_id, GValue* val, GParamSpec* pspec) {
	ScidaePangoContext* self = SCIDAE_PANGO_CONTEXT(object);
	switch (prop_id) {
		case PROP_CONTEXT:
			g_value_set_object(val, scidae_pango_context_get_context(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void scidae_pango_context_object_set_property(GObject* object, guint prop_id, const GValue* val, GParamSpec* pspec) {
	ScidaePangoContext* self = SCIDAE_PANGO_CONTEXT(object);
	switch (prop_id) {
		case PROP_CONTEXT:
			scidae_pango_context_set_context(self, g_value_get_object(val));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static ScidaeFromUnitsFun scidae_pango_context_context_units_from_double(ScidaeContext*) {
	return pango2_units_to_double;
}

static ScidaeToUnitsFun scidae_pango_context_context_units_to_double(ScidaeContext*) {
	return pango2_units_from_double;
}

static ScidaeText* scidae_pango_context_context_create_text_widget(ScidaeContext* self, const gchar* text) {
	return scidae_pango_text_new(SCIDAE_PANGO_CONTEXT(self), text);
}

static void scidae_pango_context_class_init(ScidaePangoContextClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	ScidaeContextClass* context_class = SCIDAE_CONTEXT_CLASS(class);
	
	object_class->dispose = scidae_pango_context_object_dispose;
	object_class->get_property = scidae_pango_context_object_get_property;
	object_class->set_property = scidae_pango_context_object_set_property;

	context_class->get_from_units = scidae_pango_context_context_units_from_double;
	context_class->get_to_units = scidae_pango_context_context_units_to_double;
	
	context_class->create_text_widget = scidae_pango_context_context_create_text_widget;
	
	/**
	 * ScidaePangoContext:pango-context: (attributes org.gtk.Property.get=scidae_pango_context_get_context org.gtk.Property.set=scidae_pango_context_set_context)
	 * 
	 * the Pango2 context that is used for anything created by this ScidaeContext
	 */
	obj_properties[PROP_CONTEXT] = g_param_spec_object(
		"pango-context",
		"Pango2 Context",
		"The Pango context this ScidaeContext wraps",
		PANGO2_TYPE_CONTEXT,
		G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY
	);
	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void scidae_pango_context_init(ScidaePangoContext* self) {
	self->context = NULL;
}

ScidaeContext* scidae_pango_context_new(Pango2Context* context) {
	return g_object_new(SCIDAE_TYPE_PANGO_CONTEXT, "pango-context", context, NULL);
}

Pango2Context* scidae_pango_context_get_context(ScidaePangoContext* self) {
	g_return_val_if_fail(SCIDAE_IS_PANGO_CONTEXT(self), NULL);
	return self->context;
}

void scidae_pango_context_set_context(ScidaePangoContext* self, Pango2Context* context) {
	g_return_if_fail(SCIDAE_IS_PANGO_CONTEXT(self));
	g_return_if_fail(PANGO2_IS_CONTEXT(context));
	
	if (self->context)
		g_object_unref(self->context);
	self->context = g_object_ref(context);

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_CONTEXT]);
}
