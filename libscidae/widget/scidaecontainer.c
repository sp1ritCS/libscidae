#include "scidaecontainer.h"

G_DEFINE_INTERFACE (ScidaeContainer, scidae_container, SCIDAE_TYPE_WIDGET)

static void scidae_container_default_init(ScidaeContainerInterface* iface) {
	iface->unparent = NULL;
	iface->get_children = NULL;
	iface->get_prev = NULL;
	iface->get_next = NULL;
	iface->update_cursor = NULL;
	iface->move_cursor_to_line_term = NULL;
	iface->move_cursor_vert = NULL;
	iface->mark_child_remeasure = NULL;
}

ScidaeWidget* scidae_container_get_prev(ScidaeContainer* self, ScidaeWidget* widget) {
	g_return_val_if_fail(SCIDAE_IS_CONTAINER(self), NULL);
	ScidaeContainerInterface* iface = SCIDAE_CONTAINER_GET_IFACE(self);
	g_return_val_if_fail(iface->get_prev != NULL, NULL);
	return iface->get_prev(self, widget);
}

ScidaeWidget* scidae_container_get_next(ScidaeContainer* self, ScidaeWidget* widget) {
	g_return_val_if_fail(SCIDAE_IS_CONTAINER(self), NULL);
	ScidaeContainerInterface* iface = SCIDAE_CONTAINER_GET_IFACE(self);
	g_return_val_if_fail(iface->get_next != NULL, NULL);
	return iface->get_next(self, widget);
}

void scidae_container_unparent(ScidaeContainer* self, ScidaeWidget* child) {
	g_return_if_fail(SCIDAE_IS_CONTAINER(self));
	ScidaeContainerInterface* iface = SCIDAE_CONTAINER_GET_IFACE(self);
	g_return_if_fail(iface->unparent != NULL);
	iface->unparent(self, child);
}

GList* scidae_container_get_children(ScidaeContainer* self) {
	g_return_val_if_fail(SCIDAE_IS_CONTAINER(self), NULL);
	ScidaeContainerInterface* iface = SCIDAE_CONTAINER_GET_IFACE(self);
	g_return_val_if_fail(iface->get_children != NULL, NULL);
	return iface->get_children(self);
}

void scidae_container_update_cursor(ScidaeContainer* self, ScidaeWidgetCursorType type, ScidaeWidget* cursor_holder) {
	g_return_if_fail(SCIDAE_IS_CONTAINER(self));
	ScidaeContainerInterface* iface = SCIDAE_CONTAINER_GET_IFACE(self);
	g_return_if_fail(iface->update_cursor != NULL);
	iface->update_cursor(self, type, cursor_holder);
}

void scidae_container_move_cursor_to_line_term(ScidaeContainer* self, ScidaeMeasurementLine* measurement, ScidaeWidgetCursorAction action, ScidaeDirection direction) {
	g_return_if_fail(SCIDAE_IS_CONTAINER(self));
	ScidaeContainerInterface* iface = SCIDAE_CONTAINER_GET_IFACE(self);
	g_return_if_fail(iface->move_cursor_to_line_term != NULL);
	iface->move_cursor_to_line_term(self, measurement, action, direction);
}

gboolean scidae_container_move_cursor_vert(ScidaeContainer* self, ScidaeMeasurementLine* measurement, ScidaeWidgetCursorAction action, ScidaeContainerVerticalDirection direction) {
	g_return_val_if_fail(SCIDAE_IS_CONTAINER(self), FALSE);
	ScidaeContainerInterface* iface = SCIDAE_CONTAINER_GET_IFACE(self);
	g_return_val_if_fail(iface->move_cursor_vert != NULL, FALSE);
	return iface->move_cursor_vert(self, measurement, action, direction);
}

void scidae_container_mark_child_remeasure(ScidaeContainer* self, ScidaeWidget* child) {
	g_return_if_fail(SCIDAE_IS_CONTAINER(self));
	ScidaeContainerInterface* iface = SCIDAE_CONTAINER_GET_IFACE(self);
	g_return_if_fail(iface->mark_child_remeasure != NULL);
	iface->mark_child_remeasure(self, child);
}
