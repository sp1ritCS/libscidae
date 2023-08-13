#include "scidaecanvas.h"

#include "widget/scidaeparagraph.h"

struct _ScidaeCanvas {
	GtkWidget parent_instance;
	
	ScidaeToplevel* child;
	ScidaeMeasurementLine* latest;

	GtkAdjustment* hadjustment;
	GtkAdjustment* vadjustment;
};

static void scidae_canvas_scrollable_iface_init(GtkScrollableInterface* iface);
G_DEFINE_TYPE_WITH_CODE (ScidaeCanvas, scidae_canvas, GTK_TYPE_WIDGET,
	G_IMPLEMENT_INTERFACE(GTK_TYPE_SCROLLABLE, scidae_canvas_scrollable_iface_init)
)

enum {
	PROP_CHILD = 1,
	N_PROPERTIES,
	PROP_HADJUSTMENT = N_PROPERTIES,
	PROP_HSCROLL_POLICY,
	PROP_VADJUSTMENT,
	PROP_VSCROLL_POLICY
};
static GParamSpec* obj_properties[N_PROPERTIES] = { 0, };

static void scidae_canvas_object_dispose(GObject* object) {
	ScidaeCanvas* self = SCIDAE_CANVAS(object);
	g_clear_pointer(&self->latest, scidae_measurement_line_unref);
	g_clear_object(&self->child);

	g_clear_object(&self->hadjustment);
	g_clear_object(&self->vadjustment);

	G_OBJECT_CLASS(scidae_canvas_parent_class)->dispose(object);
}

static void scidae_canvas_adjustment_changed(GtkAdjustment*, ScidaeCanvas* self) {
	gtk_widget_queue_draw(GTK_WIDGET(self));
}

static void scidae_canvas_set_hadjustment(ScidaeCanvas* self, GtkAdjustment* hadjustment) {
	g_return_if_fail(SCIDAE_IS_CANVAS(self));
	g_return_if_fail(!hadjustment || GTK_IS_ADJUSTMENT(hadjustment));

	if (self->hadjustment == hadjustment && self->hadjustment)
		return;

	if (self->hadjustment) {
		g_signal_handlers_disconnect_by_func(self->hadjustment, scidae_canvas_adjustment_changed, self);
		g_object_unref(self->hadjustment);
	}

	if (!hadjustment)
		hadjustment = gtk_adjustment_new(0.0, 0.0, 0.0, 1.0, 5.0, 0.0);

	g_signal_connect(hadjustment, "value-changed", G_CALLBACK(scidae_canvas_adjustment_changed), self);
	self->hadjustment = g_object_ref(hadjustment);

	g_object_notify(G_OBJECT(self), "hadjustment");
}

static void scidae_canvas_set_vadjustment(ScidaeCanvas* self, GtkAdjustment* vadjustment) {
	g_return_if_fail(SCIDAE_IS_CANVAS(self));
	g_return_if_fail(!vadjustment || GTK_IS_ADJUSTMENT(vadjustment));

	if (self->vadjustment == vadjustment && self->vadjustment)
		return;

	if (self->vadjustment) {
		g_signal_handlers_disconnect_by_func(self->vadjustment, scidae_canvas_adjustment_changed, self);
		g_object_unref(self->vadjustment);
	}

	if (!vadjustment)
		vadjustment = gtk_adjustment_new(0.0, 0.0, 0.0, 1.0, 5.0, 0.0);

	g_signal_connect(vadjustment, "value-changed", G_CALLBACK(scidae_canvas_adjustment_changed), self);
	self->vadjustment = g_object_ref(vadjustment);

	g_object_notify(G_OBJECT(self), "vadjustment");
}

static void scidae_canvas_object_get_property(GObject* object, guint prop_id, GValue* val, GParamSpec* pspec) {
	ScidaeCanvas* self = SCIDAE_CANVAS(object);
	
	switch (prop_id) {
		case PROP_CHILD:
			g_value_set_object(val, scidae_canvas_get_child(self));
			break;
		case PROP_HADJUSTMENT:
			g_value_set_object(val, self->hadjustment);
			break;
		case PROP_VADJUSTMENT:
			g_value_set_object(val, self->vadjustment);
			break;
		case PROP_HSCROLL_POLICY:
			g_value_set_enum(val, GTK_SCROLL_NATURAL);
			break;
		case PROP_VSCROLL_POLICY:
			g_value_set_enum(val, GTK_SCROLL_NATURAL);
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
		case PROP_HADJUSTMENT:
			scidae_canvas_set_hadjustment(self, g_value_get_object(val));
			break;
		case PROP_HSCROLL_POLICY: G_GNUC_FALLTHROUGH;
		case PROP_VSCROLL_POLICY:
			g_critical("You can't set h/vscroll-policy");
			break;
		case PROP_VADJUSTMENT:
			scidae_canvas_set_vadjustment(self, g_value_get_object(val));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

void scidae_canvas_widget_size_allocate(GtkWidget* widget, int width, int height, int) {
	ScidaeCanvas* self = SCIDAE_CANVAS(widget);
	if (self->latest)
		scidae_measurement_line_unref(self->latest);
	self->latest = NULL;

	gtk_adjustment_set_page_size(self->hadjustment, width);
	gtk_adjustment_set_page_size(self->vadjustment, height);
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

		ScidaeFromUnitsFun s_from_units = scidae_context_get_from_units(scidae_widget_get_context(child_widget));
		gtk_adjustment_set_upper(self->hadjustment, s_from_units(self->latest->width));
		gtk_adjustment_set_upper(self->vadjustment, s_from_units(self->latest->height));
	}

	GtkAllocation alloc;
	gtk_widget_get_allocation(widget, &alloc);

	ScidaeToUnitsFun s_to_units = scidae_context_get_to_units(scidae_widget_get_context(child_widget));
	ScidaeRectangle rect = SCIDAE_RECTANGLE_INIT(
		s_to_units(gtk_adjustment_get_value(self->hadjustment)),
		s_to_units(gtk_adjustment_get_value(self->vadjustment)),
		s_to_units(alloc.width),
		s_to_units(alloc.height)
	);

	g_autoptr(GskRenderNode) render = scidae_widget_render(child_widget, self->latest, &rect);
	g_autoptr(GskRenderNode) trans = gsk_transform_node_new(render,
		gsk_transform_translate(
			gsk_transform_new(),
			&GRAPHENE_POINT_INIT(
				-gtk_adjustment_get_value(self->hadjustment),
				-gtk_adjustment_get_value(self->vadjustment)
			)
		)
	);

	gtk_snapshot_append_node(snapshot, trans);
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

	g_object_class_override_property(object_class, PROP_HADJUSTMENT, "hadjustment");
	g_object_class_override_property(object_class, PROP_HSCROLL_POLICY, "hscroll-policy");
	g_object_class_override_property(object_class, PROP_VADJUSTMENT, "vadjustment");
	g_object_class_override_property(object_class, PROP_VSCROLL_POLICY, "vscroll-policy");
}

static void scidae_canvas_init(ScidaeCanvas* self) {
	self->child = NULL;
	self->latest = NULL;

	self->hadjustment = NULL;
	self->vadjustment = NULL;
}

static void scidae_canvas_scrollable_iface_init(G_GNUC_UNUSED GtkScrollableInterface* iface) {}

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
