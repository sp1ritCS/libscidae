#ifndef __SCIDAETOPLEVEL_H__
#define __SCIDAETOPLEVEL_H__

#include <glib-object.h>
#include <widget/scidaewidget.h>

G_BEGIN_DECLS

#define SCIDAE_TYPE_TOPLEVEL (scidae_toplevel_get_type())
G_DECLARE_INTERFACE (ScidaeToplevel, scidae_toplevel, SCIDAE, TOPLEVEL, ScidaeWidget)

struct _ScidaeToplevelInterface {
	GTypeInterface parent_iface;
	
	gboolean(*should_remeasure)(ScidaeToplevel* self);
};

/**
 * scidae_toplevel_should_remeasure:
 * @self: the toplevel widget
 *
 * Tests if the widget has changes and wants to be remeasured.
 * Returns: %TRUE if the widget wants to be remeasured, %FALSE otherwise
 */
gboolean scidae_toplevel_should_remeasure(ScidaeToplevel* self);

/**
 * scidae_toplevel_emit_redraw:
 * @self: the toplevel widget
 *
 * Emit a redraw signal. Should only be used by implementors.
 */
void scidae_toplevel_emit_redraw(ScidaeToplevel* self);

G_END_DECLS

#endif // __SCIDAETOPLEVEL_H__
