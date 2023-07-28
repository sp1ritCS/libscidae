#include "scidaecontainer.h"

G_DEFINE_INTERFACE (ScidaeContainer, scidae_container, SCIDAE_TYPE_WIDGET)

static void scidae_container_default_init(ScidaeContainerInterface* iface) {
	iface->unparent = NULL;
	iface->get_children = NULL;
	iface->mark_child_remeasure = NULL;
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
	g_return_val_if_fail(iface->unparent != NULL, NULL);
	return iface->get_children(self);
}

void scidae_container_mark_child_remeasure(ScidaeContainer* self, ScidaeWidget* child) {
	g_return_if_fail(SCIDAE_IS_CONTAINER(self));
	ScidaeContainerInterface* iface = SCIDAE_CONTAINER_GET_IFACE(self);
	g_return_if_fail(iface->mark_child_remeasure != NULL);
	iface->mark_child_remeasure(self, child);
}
