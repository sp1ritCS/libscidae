#include "scidaetoplevel.h"

G_DEFINE_INTERFACE (ScidaeToplevel, scidae_toplevel, SCIDAE_TYPE_WIDGET)

static void scidae_toplevel_default_init(ScidaeToplevelInterface* iface) {
	iface->should_remeasure = NULL;
}

gboolean scidae_toplevel_should_remeasure(ScidaeToplevel* self) {
	g_return_val_if_fail(SCIDAE_IS_TOPLEVEL(self), FALSE);
	ScidaeToplevelInterface* iface = SCIDAE_TOPLEVEL_GET_IFACE(self);
	g_return_val_if_fail(iface->should_remeasure, FALSE);
	return iface->should_remeasure(self);
}
