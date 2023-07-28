#ifndef __SCIDAEWIDGET_ALT_H__
#define __SCIDAEWIDGET_ALT_H__

#include <widget/scidaewidget.h>
#include <widget/scidaecontainer.h>

G_BEGIN_DECLS

/**
 * scidae_widget_get_parent: (attributes org.gtk.Method.get_property=parent)
 * @self: a `ScidaeWidget`
 *
 * Returns the parent of this widget (if the widget has a parent)
 *
 * Returns: (nullable) (transfer none): the parent of this widget
 */
ScidaeContainer* scidae_widget_get_parent(ScidaeWidget* self);

/**
 * scidae_widget_set_parent: (attributes org.gtk.Method.set_property=parent)
 * @self: a `ScidaeWidget`
 * @parent: the new parent
 * 
 * Sets a new parent for this widget.
 */
void scidae_widget_set_parent(ScidaeWidget* self, ScidaeContainer* parent);

G_END_DECLS

#endif // __SCIDAEWIDGET_ALT_H__
