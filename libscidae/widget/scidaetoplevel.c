#include "scidaetoplevel.h"

G_DEFINE_INTERFACE (ScidaeToplevel, scidae_toplevel, SCIDAE_TYPE_WIDGET)

enum {
	SIG_REDRAW,
	LAST_SIGNAL
};
static int signals[LAST_SIGNAL];

static void scidae_toplevel_default_init(ScidaeToplevelInterface* iface) {
	iface->should_remeasure = NULL;

	signals[SIG_REDRAW] = g_signal_new("redraw", SCIDAE_TYPE_TOPLEVEL,
		G_SIGNAL_RUN_LAST,
		0,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE, 0
	);
}

gboolean scidae_toplevel_should_remeasure(ScidaeToplevel* self) {
	g_return_val_if_fail(SCIDAE_IS_TOPLEVEL(self), FALSE);
	ScidaeToplevelInterface* iface = SCIDAE_TOPLEVEL_GET_IFACE(self);
	g_return_val_if_fail(iface->should_remeasure, FALSE);
	return iface->should_remeasure(self);
}

void scidae_toplevel_emit_redraw(ScidaeToplevel* self) {
	g_return_if_fail(SCIDAE_IS_TOPLEVEL(self));
	g_signal_emit(self, signals[SIG_REDRAW], 0);
}
