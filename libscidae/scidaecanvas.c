#include "scidaecanvas.h"

#include "widget/scidaecontainer.h"

struct _ScidaeCanvas {
	GtkWidget parent_instance;

	ScidaeToplevel* child;
	ScidaeMeasurementLine* latest;

	gdouble zoom;
	GtkAdjustment* hadjustment;
	GtkAdjustment* vadjustment;
};

static void scidae_canvas_scrollable_iface_init(GtkScrollableInterface* iface);
G_DEFINE_TYPE_WITH_CODE (ScidaeCanvas, scidae_canvas, GTK_TYPE_WIDGET,
	G_IMPLEMENT_INTERFACE(GTK_TYPE_SCROLLABLE, scidae_canvas_scrollable_iface_init)
)

enum {
	PROP_CHILD = 1,
	PROP_ZOOM,
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
		case PROP_ZOOM:
			g_value_set_double(val, scidae_canvas_get_zoom(self));
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
		case PROP_ZOOM:
			scidae_canvas_set_zoom(self, g_value_get_double(val));
			break;
		case PROP_HADJUSTMENT:
			scidae_canvas_set_hadjustment(self, g_value_get_object(val));
			break;
		case PROP_VADJUSTMENT:
			scidae_canvas_set_vadjustment(self, g_value_get_object(val));
			break;
		case PROP_HSCROLL_POLICY: G_GNUC_FALLTHROUGH;
		case PROP_VSCROLL_POLICY:
			g_critical("You can't set h/vscroll-policy");
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static inline void scidae_canvas_queue_remeasure(ScidaeCanvas* self) {
	if (self->latest)
		scidae_measurement_line_unref(self->latest);
	self->latest = NULL;

	gtk_widget_queue_draw(GTK_WIDGET(self));
}

void scidae_canvas_widget_size_allocate(GtkWidget* widget, int width, int height, int) {
	ScidaeCanvas* self = SCIDAE_CANVAS(widget);
	scidae_canvas_queue_remeasure(self);

	gtk_adjustment_set_page_size(self->hadjustment, width);
	gtk_adjustment_set_page_size(self->vadjustment, height);
}

static void scidae_canvas_int_remeasure_if_needed(ScidaeCanvas* self) {
	ScidaeWidget* child_widget = SCIDAE_WIDGET(self->child);
	if (!self->latest || scidae_toplevel_should_remeasure(self->child)) {
		gpointer prev = NULL;
		ScidaeMeasurementResult measurement_r = scidae_widget_measure(child_widget, scidae_context_to_units(scidae_widget_get_context(child_widget), gtk_widget_get_width(GTK_WIDGET(self))), 0, FALSE, SCIDAE_WIDGET_MEASUREMENT_NO_ATTRS, &prev);
		if (measurement_r.result != SCIDAE_MEASUREMENT_FINISH)
			g_error("Unexpected measurement result!");
		
		if (self->latest)
			scidae_measurement_line_unref(self->latest);
		self->latest = measurement_r.line;

		ScidaeFromUnitsFun s_from_units = scidae_context_get_from_units(scidae_widget_get_context(child_widget));
		gtk_adjustment_set_upper(self->hadjustment, s_from_units(self->latest->width) * self->zoom);
		gtk_adjustment_set_upper(self->vadjustment, s_from_units(self->latest->height) * self->zoom);
	}
}

static void scidae_canvas_widget_snapshot(GtkWidget* widget, GtkSnapshot* snapshot) {
	ScidaeCanvas* self = SCIDAE_CANVAS(widget);
	ScidaeWidget* child_widget = SCIDAE_WIDGET(self->child);

	scidae_canvas_int_remeasure_if_needed(self);

	ScidaeToUnitsFun s_to_units = scidae_context_get_to_units(scidae_widget_get_context(child_widget));
	ScidaeRectangle rect = SCIDAE_RECTANGLE_INIT(
		s_to_units(gtk_adjustment_get_value(self->hadjustment)),
		s_to_units(gtk_adjustment_get_value(self->vadjustment)),
		s_to_units(gtk_widget_get_width(widget) / self->zoom),
		s_to_units(gtk_widget_get_height(widget) / self->zoom)
	);

	g_autoptr(GskRenderNode) render = scidae_widget_render(child_widget, self->latest, &rect);
	g_autoptr(GskRenderNode) trans = gsk_transform_node_new(render,
		gsk_transform_translate(
			gsk_transform_scale(gsk_transform_new(), self->zoom, self->zoom),
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
	
	/**
	 * ScidaeCanvas:child: (attributes org.gtk.Property.get=scidae_canvas_get_child org.gtk.Property.set=scidae_canvas_set_child)
	 *
	 * the toplevel scidae widget that will be rendered in this canvas
	 */
	obj_properties[PROP_CHILD] = g_param_spec_object(
		"child",
		"Child",
		"The widget to render",
		SCIDAE_TYPE_TOPLEVEL,
		G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY
	);

	/**
	 * ScidaeCanvas:zoom: (attributes org.gtk.Property.get=scidae_canvas_get_zoom org.gtk.Property.set=scidae_canvas_set_zoom)
	 *
	 * the zoom factor of the canvas
	 */
	obj_properties[PROP_ZOOM] = g_param_spec_double(
		"zoom",
		"Zoom",
		"The current zoom level in display units",
		0.1,
		10.0,
		1.0,
		G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY
	);
	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);

	g_object_class_override_property(object_class, PROP_HADJUSTMENT, "hadjustment");
	g_object_class_override_property(object_class, PROP_HSCROLL_POLICY, "hscroll-policy");
	g_object_class_override_property(object_class, PROP_VADJUSTMENT, "vadjustment");
	g_object_class_override_property(object_class, PROP_VSCROLL_POLICY, "vscroll-policy");
}

static void scidae_canvas_zoom_changed(GtkGestureZoom*, gdouble scale, ScidaeCanvas* self) {
	scidae_canvas_set_zoom(self, /*self->zoom * */scale); // I don't like this TODO: find a better solution
}

static void scidae_canvas_im_commit(GtkIMContext*, gchar* str, ScidaeCanvas* self) {
	scidae_widget_insert_at_cursor(SCIDAE_WIDGET(self->child), str, -1);
}

static inline void scidae_canvas_mods_from_gdk(GdkModifierType state, ScidaeWidgetMovementModifier* mod, ScidaeWidgetCursorAction* cursor_action) {
	if (mod) {
		*mod = 0;
		if (state & GDK_CONTROL_MASK)
			*mod |= SCIDAE_MOVEMENT_MODIFIER_CONTROL;
		if (state & GDK_ALT_MASK)
			*mod |= SCIDAE_MOVEMENT_MODIFIER_ALT;
	}

	if (cursor_action) {
		if (state & GDK_SHIFT_MASK)
			*cursor_action = SCIDAE_CURSOR_ACTION_MOVE_MASTER;
		else
			*cursor_action = SCIDAE_CURSOR_ACTION_MOVE;
	}
}

static void scidae_canvas_handle_input(GtkEventControllerKey*, guint keyval, G_GNUC_UNUSED guint keycode, GdkModifierType state, ScidaeCanvas* self) {
	ScidaeWidgetMovementModifier mods;
	ScidaeWidgetCursorAction action;
	scidae_canvas_mods_from_gdk(state, &mods, &action);
	switch (keyval) {
		case GDK_KEY_Left:
			scidae_widget_move_cursor(SCIDAE_WIDGET(self->child), SCIDAE_DIRECTION_BACKWARD, mods, action);
			break;
		case GDK_KEY_Right:
			scidae_widget_move_cursor(SCIDAE_WIDGET(self->child), SCIDAE_DIRECTION_FORWARD, mods, action);
			break;
		case GDK_KEY_Up:
			scidae_canvas_int_remeasure_if_needed(self);
			if (SCIDAE_IS_CONTAINER(self->child))
				scidae_container_move_cursor_vert((ScidaeContainer*)self->child, self->latest, action, SCIDAE_VERTICAL_DIRECTION_UPWARD);
			break;
		case GDK_KEY_Down:
			scidae_canvas_int_remeasure_if_needed(self);
			if (SCIDAE_IS_CONTAINER(self->child))
				scidae_container_move_cursor_vert((ScidaeContainer*)self->child, self->latest, action, SCIDAE_VERTICAL_DIRECTION_DOWNWARD);
			break;
		case GDK_KEY_Home:
			scidae_canvas_int_remeasure_if_needed(self);
			if (SCIDAE_IS_CONTAINER(self->child))
				scidae_container_move_cursor_to_line_term((ScidaeContainer*)self->child, self->latest, action, SCIDAE_DIRECTION_BACKWARD);
			break;
		case GDK_KEY_End:
			scidae_canvas_int_remeasure_if_needed(self);
			if (SCIDAE_IS_CONTAINER(self->child))
				scidae_container_move_cursor_to_line_term((ScidaeContainer*)self->child, self->latest, action, SCIDAE_DIRECTION_FORWARD);
			break;
		case GDK_KEY_Return:
			scidae_widget_insert_at_cursor(SCIDAE_WIDGET(self->child), "\n", -1);
			break;
		case GDK_KEY_BackSpace:
			scidae_widget_delete(SCIDAE_WIDGET(self->child), SCIDAE_DIRECTION_BACKWARD, mods);
			break;
		case GDK_KEY_Delete:
			scidae_widget_delete(SCIDAE_WIDGET(self->child), SCIDAE_DIRECTION_FORWARD, mods);
			break;
		default:
			(void)0;
	}
}

static void scidae_canvas_handle_mouse(GtkGestureClick* gest, gint, gdouble x, gdouble y, ScidaeCanvas* self) {
	GdkEvent* ev = gtk_event_controller_get_current_event(GTK_EVENT_CONTROLLER(gest));
	GdkModifierType modifier = gdk_event_get_modifier_state(ev);
	ScidaeWidgetCursorAction action;
	scidae_canvas_mods_from_gdk(modifier, NULL, &action);

	ScidaeToUnitsFun to_units = scidae_context_get_to_units(scidae_widget_get_context(SCIDAE_WIDGET(self->child)));
	scidae_widget_move_cursor_to_pos(
		SCIDAE_WIDGET(self->child),
		self->latest,
		to_units(gtk_adjustment_get_value(self->hadjustment) + x / self->zoom),
		to_units(gtk_adjustment_get_value(self->vadjustment) + y / self->zoom),
		action
	);
}

static void scidae_canvas_init(ScidaeCanvas* self) {
	gtk_widget_set_focusable(GTK_WIDGET(self), TRUE);

	self->child = NULL;
	self->latest = NULL;

	self->zoom = 1.0;
	self->hadjustment = NULL;
	self->vadjustment = NULL;

	GtkGesture* zoom_gest = gtk_gesture_zoom_new();
	g_signal_connect(zoom_gest, "scale-changed", G_CALLBACK(scidae_canvas_zoom_changed), self);
	gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(zoom_gest));

	g_autoptr(GtkIMContext) ime = gtk_im_multicontext_new();
	gtk_im_context_set_use_preedit(ime, TRUE);
	gtk_im_context_set_client_widget(ime, GTK_WIDGET(self));
	g_signal_connect(ime, "commit", G_CALLBACK(scidae_canvas_im_commit), self);

	GtkEventController* ev = gtk_event_controller_key_new();
	gtk_event_controller_key_set_im_context(GTK_EVENT_CONTROLLER_KEY(ev), ime);
	g_signal_connect(ev, "key-pressed", G_CALLBACK(scidae_canvas_handle_input), self);
	gtk_widget_add_controller(GTK_WIDGET(self), ev);

	GtkGesture* click = gtk_gesture_click_new();
	gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), 1);
	g_signal_connect(click, "pressed", G_CALLBACK(scidae_canvas_handle_mouse), self);
	gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(click));
}

static void scidae_canvas_scrollable_iface_init(G_GNUC_UNUSED GtkScrollableInterface* iface) {}

GtkWidget* scidae_canvas_new(ScidaeToplevel* child) {
	return g_object_new(SCIDAE_TYPE_CANVAS, "child", child, NULL);
}

ScidaeToplevel* scidae_canvas_get_child(ScidaeCanvas* self) {
	g_return_val_if_fail(SCIDAE_IS_CANVAS(self), NULL);
	return self->child;
}

static void scidae_canvas_context_fontsize_changed(ScidaeContext*, GParamSpec*, ScidaeCanvas* self) {
	scidae_canvas_queue_remeasure(self);
}

static void scidae_canvas_toplevel_redraw(ScidaeToplevel*, ScidaeCanvas* self) {
	scidae_canvas_queue_remeasure(self);
}

void scidae_canvas_set_child(ScidaeCanvas* self, ScidaeToplevel* child) {
	g_return_if_fail(SCIDAE_IS_CANVAS(self));
	g_return_if_fail(SCIDAE_IS_TOPLEVEL(child));

	if (self->child == child)
		return;

	if (self->child) {
		g_signal_handlers_disconnect_by_func(scidae_widget_get_context(SCIDAE_WIDGET(self->child)), scidae_canvas_context_fontsize_changed, self);
		g_signal_handlers_disconnect_by_func(self->child, scidae_canvas_toplevel_redraw, self);
		g_object_unref(self->child);
	}
	self->child = g_object_ref(child);
	g_signal_connect(scidae_widget_get_context(SCIDAE_WIDGET(self->child)), "notify::base-font-size", G_CALLBACK(scidae_canvas_context_fontsize_changed), self);
	g_signal_connect(self->child, "redraw", G_CALLBACK(scidae_canvas_toplevel_redraw), self);

	scidae_canvas_queue_remeasure(self);

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_CHILD]);
}

gdouble scidae_canvas_get_zoom(ScidaeCanvas* self) {
	g_return_val_if_fail(SCIDAE_IS_CANVAS(self), NAN);
	return self->zoom;
}

void scidae_canvas_set_zoom(ScidaeCanvas* self, gdouble zoom) {
	g_return_if_fail(SCIDAE_IS_CANVAS(self));

	if (zoom < 0.1)
		zoom = 0.1;
	if (zoom > 10.0)
		zoom = 10.0;
	if (self->zoom == zoom) // maybe do float cmp with epsilon instead?
		return;

	self->zoom = zoom;

	if (self->latest) {
		ScidaeFromUnitsFun s_from_units = scidae_context_get_from_units(scidae_widget_get_context(SCIDAE_WIDGET(self->child)));
		gtk_adjustment_set_upper(self->hadjustment, s_from_units(self->latest->width) * self->zoom);
		gtk_adjustment_set_upper(self->vadjustment, s_from_units(self->latest->height) * self->zoom);
	}

	gtk_widget_queue_draw(GTK_WIDGET(self));
	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_ZOOM]);
}
