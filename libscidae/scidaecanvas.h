#ifndef __SCIDAECANVAS_H__
#define __SCIDAECANVAS_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include <widget/scidaewidget.h>
#include <widget/scidaetoplevel.h>

G_BEGIN_DECLS

/**
 * ScidaeCanvas:
 * 
 * This widget is used to display and edit a [class@Widget]. 
 */
#define SCIDAE_TYPE_CANVAS (scidae_canvas_get_type())
G_DECLARE_FINAL_TYPE (ScidaeCanvas, scidae_canvas, SCIDAE, CANVAS, GtkWidget)

/**
 * scidae_canvas_new:
 * @child: the child this canvas will render
 *
 * Create a new instance of ScidaeCanvas.
 * Returns: (transfer full): the canvas as widget
 */
GtkWidget* scidae_canvas_new(ScidaeToplevel* child);

/**
 * scidae_canvas_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: the canvas
 *
 * Get the widget that the canvas is currently drawing.
 * Returns: (transfer none): the current widget
 */
ScidaeToplevel* scidae_canvas_get_child(ScidaeCanvas* self);

/**
 * scidae_canvas_set_child: (attributes org.gtk.Method.set_property=child)
 * @self: the canvas
 * @child: a new widget
 *
 * Change the widget that the canvas renderes.
 */
void scidae_canvas_set_child(ScidaeCanvas* self, ScidaeToplevel* child);

/**
 * scidae_canvas_get_zoom: (attributes org.gtk.Method.get_property=zoom)
 * @self: the canvas
 *
 * Get the current zoom level in display units.
 * Returns: the current zoom level
 */
gdouble scidae_canvas_get_zoom(ScidaeCanvas* self);

/**
 * scidae_canvas_set_zoom: (attributes org.gtk.Method.set_property=zoom)
 * @self: the canvas
 * @zoom: the new zoom level in display units
 *
 * Update the current zoom level.
 */
void scidae_canvas_set_zoom(ScidaeCanvas* self, gdouble zoom);

G_END_DECLS

#endif // __SCIDAECANVAS_H__
