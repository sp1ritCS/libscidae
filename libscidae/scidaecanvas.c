#include "scidaecanvas.h"

#include "widget/scidaeparagraph.h"

struct _ScidaeCanvas {
	GtkWidget parent_instance;
	
	ScidaeToplevel* child;
	ScidaeMeasurementLine* latest;
};

G_DEFINE_TYPE (ScidaeCanvas, scidae_canvas, GTK_TYPE_WIDGET)

enum {
	PROP_CHILD = 1,
	N_PROPERTIES
};
static GParamSpec* obj_properties[N_PROPERTIES] = { 0, };

static void scidae_canvas_object_dispose(GObject* object) {
	ScidaeCanvas* self = SCIDAE_CANVAS(object);
	g_clear_pointer(&self->latest, scidae_measurement_line_unref);
	g_clear_object(&self->child);

	G_OBJECT_CLASS(scidae_canvas_parent_class)->dispose(object);
}

static void scidae_canvas_object_get_property(GObject* object, guint prop_id, GValue* val, GParamSpec* pspec) {
	ScidaeCanvas* self = SCIDAE_CANVAS(object);
	
	switch (prop_id) {
		case PROP_CHILD:
			g_value_set_object(val, scidae_canvas_get_child(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void scidae_canvas_object_set_property(GObject* object, guint prop_id, const GValue* val, GParamSpec* pspec) {
	ScidaeCanvas* self = SCIDAE_CANVAS(object);
	
	switch (prop_id) {
		case PROP_CHILD:
			scidae_canvas_set_child(self, g_value_get_object(val));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

void scidae_canvas_widget_size_allocate(GtkWidget* widget, int, int, int) {
	ScidaeCanvas* self = SCIDAE_CANVAS(widget);
	if (self->latest)
		scidae_measurement_line_unref(self->latest);
	self->latest = NULL;
}

static void scidae_canvas_widget_snapshot(GtkWidget* widget, GtkSnapshot* snapshot) {
	ScidaeCanvas* self = SCIDAE_CANVAS(widget);
	ScidaeWidget* child_widget = SCIDAE_WIDGET(self->child);

	if (!self->latest || scidae_toplevel_should_remeasure(self->child)) {
		gpointer prev = NULL;
		ScidaeMeasurementResult measurement_r = scidae_widget_measure(child_widget, scidae_context_to_units(scidae_widget_get_context(child_widget), gtk_widget_get_width(widget)), 0, FALSE, &prev);
		if (measurement_r.result != SCIDAE_MEASUREMENT_FINISH)
			g_error("Unexpected measurement result!");
		
		if (self->latest)
			scidae_measurement_line_unref(self->latest);
		self->latest = measurement_r.line;
	}

	g_autoptr(GskRenderNode) render = scidae_widget_render(child_widget, self->latest);
	gtk_snapshot_append_node(snapshot, render);
}

static void scidae_canvas_class_init(ScidaeCanvasClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(class);
	
	object_class->dispose = scidae_canvas_object_dispose;
	object_class->get_property = scidae_canvas_object_get_property;
	object_class->set_property = scidae_canvas_object_set_property;
	
	widget_class->size_allocate = scidae_canvas_widget_size_allocate;
	widget_class->snapshot = scidae_canvas_widget_snapshot;
	
	obj_properties[PROP_CHILD] = g_param_spec_object(
		"child",
		"Child",
		"The widget to render",
		SCIDAE_TYPE_TOPLEVEL,
		G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY
	);
	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void scidae_canvas_init(ScidaeCanvas* self) {
	self->child = NULL;
	self->latest = NULL;
}

GtkWidget* scidae_canvas_new(ScidaeToplevel* child) {
	return g_object_new(SCIDAE_TYPE_CANVAS, "child", child, NULL);
}

ScidaeToplevel* scidae_canvas_get_child(ScidaeCanvas* self) {
	g_return_val_if_fail(SCIDAE_IS_CANVAS(self), NULL);
	return self->child;
}

void scidae_canvas_set_child(ScidaeCanvas* self, ScidaeToplevel* child) {
	g_return_if_fail(SCIDAE_IS_CANVAS(self));
	g_return_if_fail(SCIDAE_IS_TOPLEVEL(child));
	
	if (self->child == child)
		return;
	
	if (self->child)
		g_object_unref(self->child);
	self->child = g_object_ref(child);
	
	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_CHILD]);
}
