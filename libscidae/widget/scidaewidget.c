#include "scidaewidget.h"
#include "scidaewidget-alt.h"

G_DEFINE_BOXED_TYPE (ScidaeMeasurementLine, scidae_measurement_line, scidae_measurement_line_ref, scidae_measurement_line_unref)

ScidaeMeasurementLine* scidae_measurement_line_ref(ScidaeMeasurementLine* self) {
	g_atomic_ref_count_inc(&self->ref);
	return self;
}

void scidae_measurement_line_unref(ScidaeMeasurementLine* self) {
	if (!self)
		return;
	if (g_atomic_ref_count_dec(&self->ref))
		self->del_fun(self);
}

typedef struct {
	ScidaeContext* context;
	ScidaeContainer* parent;
} ScidaeWidgetPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (ScidaeWidget, scidae_widget, G_TYPE_INITIALLY_UNOWNED)

enum {
	PROP_CONTEXT = 1,
	PROP_PARENT,
	N_PROPERTIES
};
static GParamSpec* obj_properties[N_PROPERTIES] = { NULL, };

static void scidae_widget_object_get_property(GObject* object, guint prop_id, GValue* val, GParamSpec* pspec) {
	ScidaeWidget* self = SCIDAE_WIDGET(object);
	
	switch (prop_id) {
		case PROP_CONTEXT:
			g_value_set_object(val, scidae_widget_get_context(self));
			break;
		case PROP_PARENT:
			g_value_set_object(val, scidae_widget_get_parent(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void scidae_widget_object_set_property(GObject* object, guint prop_id, const GValue* val, GParamSpec* pspec) {
	ScidaeWidget* self = SCIDAE_WIDGET(object);
	
	switch (prop_id) {
		case PROP_CONTEXT:
			scidae_widget_set_context(self, g_value_get_object(val));
			break;
		case PROP_PARENT:
			scidae_widget_set_parent(self, g_value_get_object(val));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void scidae_widget_class_init(ScidaeWidgetClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	
	class->get_markdown = NULL;
	class->merge_markdown_start = NULL;
	class->merge_markdown_end = NULL;

	class->measure = NULL;
	class->render = NULL;

	class->drop_cursor = NULL;
	class->move_cursor_backward = NULL;
	class->move_cursor_forward = NULL;
	class->move_cursor_to_pos = NULL;

	class->insert_at_cursor = NULL;

	class->delete_backward = NULL;
	class->delete_forward = NULL;
	
	object_class->get_property = scidae_widget_object_get_property;
	object_class->set_property = scidae_widget_object_set_property;
	
	/**
	 * ScidaeWidget:context: (attributes org.gtk.Property.get=scidae_widget_get_context org.gtk.Property.set=scidae_widget_set_context)
	 * 
	 * the Context this widget uses
	 */
	obj_properties[PROP_CONTEXT] = g_param_spec_object(
		"context",
		"Context",
		"The context this widget uses",
		SCIDAE_TYPE_CONTEXT,
		G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY
	);

	/**
	 * ScidaeWidget:parent: (attributes org.gtk.Property.get=scidae_widget_get_parent org.gtk.Property.set=scidae_widget_set_parent)
	 *
	 * the parent of this widget
	 */
	obj_properties[PROP_PARENT] = g_param_spec_object(
		"parent",
		"Parent",
		"The parent container that holds this widget",
		SCIDAE_TYPE_CONTAINER,
		G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY
	);
	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void scidae_widget_init(ScidaeWidget* self) {
	ScidaeWidgetPrivate* priv = scidae_widget_get_instance_private(self);
	priv->parent = NULL;
}

ScidaeContext* scidae_widget_get_context(ScidaeWidget* self) {
	g_return_val_if_fail(SCIDAE_IS_WIDGET(self), NULL);
	ScidaeWidgetPrivate* priv = scidae_widget_get_instance_private(self);
	return priv->context;
}

void scidae_widget_set_context(ScidaeWidget* self, ScidaeContext* context) {
	g_return_if_fail(SCIDAE_IS_WIDGET(self));
	g_return_if_fail(SCIDAE_IS_CONTEXT(context));
	ScidaeWidgetPrivate* priv = scidae_widget_get_instance_private(self);
	
	if (priv->parent)
		g_object_unref(priv->context);
	priv->context = g_object_ref(context);
	
	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_CONTEXT]);
}

ScidaeContainer* scidae_widget_get_parent(ScidaeWidget* self) {
	g_return_val_if_fail(SCIDAE_IS_WIDGET(self), NULL);
	ScidaeWidgetPrivate* priv = scidae_widget_get_instance_private(self);
	
	return priv->parent;
}

void scidae_widget_set_parent(ScidaeWidget* self, ScidaeContainer* parent) {
	g_return_if_fail(SCIDAE_IS_WIDGET(self));
	g_return_if_fail(parent == NULL || SCIDAE_IS_CONTAINER(parent));
	ScidaeWidgetPrivate* priv = scidae_widget_get_instance_private(self);
	
	ScidaeWidget* self_ref = g_object_ref(self);
	if (priv->parent)
		scidae_container_unparent(priv->parent, self);
	priv->parent = parent;

	g_object_notify_by_pspec(G_OBJECT(self_ref), obj_properties[PROP_PARENT]);
	g_object_unref(self_ref);
}

gchar* scidae_widget_get_markdown(ScidaeWidget* self) {
	g_return_val_if_fail(SCIDAE_IS_WIDGET(self), NULL);
	ScidaeWidgetClass* widget_class = SCIDAE_WIDGET_GET_CLASS(self);
	g_return_val_if_fail(widget_class->get_markdown, NULL);
	return widget_class->get_markdown(self);
}

void scidae_widget_merge_markdown_start(ScidaeWidget* self, const gchar* text) {
	g_return_if_fail(SCIDAE_IS_WIDGET(self));
	ScidaeWidgetClass* widget_class = SCIDAE_WIDGET_GET_CLASS(self);
	g_return_if_fail(widget_class->merge_markdown_start);
	widget_class->merge_markdown_start(self, text);
}

void scidae_widget_merge_markdown_end(ScidaeWidget* self, const gchar* text) {
	g_return_if_fail(SCIDAE_IS_WIDGET(self));
	ScidaeWidgetClass* widget_class = SCIDAE_WIDGET_GET_CLASS(self);
	g_return_if_fail(widget_class->merge_markdown_start);
	widget_class->merge_markdown_start(self, text);
}

ScidaeMeasurementResult scidae_widget_measure(ScidaeWidget* self, gint width, gint start_x, gboolean force, ScidaeWidgetMeasurementAttrs attrs, gpointer* previous) {
	g_return_val_if_fail(SCIDAE_IS_WIDGET(self), scidae_measurement_result_failure);
	ScidaeWidgetClass* widget_class = SCIDAE_WIDGET_GET_CLASS(self);
	g_return_val_if_fail(widget_class->measure, scidae_measurement_result_failure);
	return widget_class->measure(self, width, start_x, force, attrs, previous);
}

GskRenderNode* scidae_widget_render(ScidaeWidget* self, ScidaeMeasurementLine* measurement, const ScidaeRectangle* area) {
	g_return_val_if_fail(SCIDAE_IS_WIDGET(self), NULL);
	ScidaeWidgetClass* widget_class = SCIDAE_WIDGET_GET_CLASS(self);
	g_return_val_if_fail(widget_class->render, NULL);
	return widget_class->render(
		self,
		measurement,
		area == NULL ? &SCIDAE_RECTANGLE_INIT(0, 0, measurement->width, measurement->height) : area
	);
}

/*private api*/ void scidae_widget_set_cursor_start(ScidaeWidget* self) {
	g_return_if_fail(SCIDAE_IS_WIDGET(self));
	ScidaeWidgetClass* widget_class = SCIDAE_WIDGET_GET_CLASS(self);
	g_return_if_fail(widget_class->set_cursor_start);
	widget_class->set_cursor_start(self);
}
/*private api*/ void scidae_widget_set_cursor_end(ScidaeWidget* self) {
	g_return_if_fail(SCIDAE_IS_WIDGET(self));
	ScidaeWidgetClass* widget_class = SCIDAE_WIDGET_GET_CLASS(self);
	g_return_if_fail(widget_class->set_cursor_end);
	widget_class->set_cursor_end(self);
}
/*private api*/ void scidae_widget_drop_cursor(ScidaeWidget* self) {
	g_return_if_fail(SCIDAE_IS_WIDGET(self));
	ScidaeWidgetClass* widget_class = SCIDAE_WIDGET_GET_CLASS(self);
	g_return_if_fail(widget_class->drop_cursor);
	widget_class->drop_cursor(self);
}
void scidae_widget_move_cursor_backward(ScidaeWidget* self, ScidaeWidgetMovementModifier modifiers, ScidaeWidgetCursorType cursor) {
	g_return_if_fail(SCIDAE_IS_WIDGET(self));
	ScidaeWidgetClass* widget_class = SCIDAE_WIDGET_GET_CLASS(self);
	g_return_if_fail(widget_class->move_cursor_backward);
	widget_class->move_cursor_backward(self, modifiers, cursor);
}
void scidae_widget_move_cursor_forward(ScidaeWidget* self, ScidaeWidgetMovementModifier modifiers, ScidaeWidgetCursorType cursor) {
	g_return_if_fail(SCIDAE_IS_WIDGET(self));
	ScidaeWidgetClass* widget_class = SCIDAE_WIDGET_GET_CLASS(self);
	g_return_if_fail(widget_class->move_cursor_forward);
	widget_class->move_cursor_forward(self, modifiers, cursor);
}

void scidae_widget_move_cursor_to_pos(ScidaeWidget* self, ScidaeMeasurementLine* measurement, gint x, gint y, ScidaeWidgetCursorType cursor) {
	g_return_if_fail(SCIDAE_IS_WIDGET(self));
	ScidaeWidgetClass* widget_class = SCIDAE_WIDGET_GET_CLASS(self);
	g_return_if_fail(widget_class->move_cursor_to_pos);
	widget_class->move_cursor_to_pos(self, measurement, x, y, cursor);
}

void scidae_widget_insert_at_cursor(ScidaeWidget* self, const gchar* text, gssize len) {
	g_return_if_fail(SCIDAE_IS_WIDGET(self));
	ScidaeWidgetClass* widget_class = SCIDAE_WIDGET_GET_CLASS(self);
	g_return_if_fail(widget_class->insert_at_cursor);
	widget_class->insert_at_cursor(self, text, len);
}

void scidae_widget_delete_backward(ScidaeWidget* self, ScidaeWidgetMovementModifier modifiers) {
	g_return_if_fail(SCIDAE_IS_WIDGET(self));
	ScidaeWidgetClass* widget_class = SCIDAE_WIDGET_GET_CLASS(self);
	g_return_if_fail(widget_class->delete_backward);
	widget_class->delete_backward(self, modifiers);
}

void scidae_widget_delete_forward(ScidaeWidget* self, ScidaeWidgetMovementModifier modifiers) {
	g_return_if_fail(SCIDAE_IS_WIDGET(self));
	ScidaeWidgetClass* widget_class = SCIDAE_WIDGET_GET_CLASS(self);
	g_return_if_fail(widget_class->delete_forward);
	widget_class->delete_forward(self, modifiers);
}
