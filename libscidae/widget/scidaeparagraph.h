#ifndef __SCIDAEPARAGRAPH_H__
#define __SCIDAEPARAGRAPH_H__

#include <glib-object.h>
#include <widget/scidaewidget.h>

G_BEGIN_DECLS

typedef struct _ScidaeParagraphMeasurementLine ScidaeParagraphMeasurementLine;

/**
 * ScidaeParagraph:
 *
 * The paragraph widget is supposed to be the commonly used "Box" that contains
 * other widgets. Due to it's simplistic nature ([method@ScidaeWidget.measure]
 * will not return partial results) it is also intended to be the child of
 * [class@ScidaeCanvas].
 */

#define SCIDAE_TYPE_PARAGRAPH (scidae_paragraph_get_type())
G_DECLARE_FINAL_TYPE (ScidaeParagraph, scidae_paragraph, SCIDAE, PARAGRAPH, ScidaeWidget)

/**
 * scidae_paragraph_new:
 * @context: a context
 *
 * Allocate a new paragraph widget.
 * Returns: (transfer full): the widget
 */
ScidaeWidget* scidae_paragraph_new(ScidaeContext* context);

/**
 * scidae_paragraph_get_line_spacing: (attributes org.gtk.Method.get_property=line-spacing)
 * @self: the paragraph widget
 *
 * Get the current distance between lines.
 * Returns: the line spacing
 */
gint scidae_paragraph_get_line_spacing(ScidaeParagraph* self);

/**
 * scidae_paragraph_set_line_spacing: (attributes org.gtk.Method.set_property=line-spacing)
 * @self: the paragraph widget
 * @spacing: the new line spacing
 *
 * Update the distance between lines.
 */
void scidae_paragraph_set_line_spacing(ScidaeParagraph* self, gint spacing);

/*
 * scidae_paragraph_add_child:
 * @self: this paragraph
 * @child: the child to add
 *
 * Appends a child as the last widget of the paragraph.
 */
void scidae_paragraph_add_child(ScidaeParagraph* self, ScidaeWidget* child);

/**
 * scidae_paragraph_insert_child_at:
 * @self: this paragraph
 * @child: the child to add
 * @index: the position of the new child
 *
 * Appends a child as the nth (index) widget of the paragraph.
 */
void scidae_paragraph_insert_child_at(ScidaeParagraph* self, ScidaeWidget* child, gint index);

G_END_DECLS

#endif // __SCIDAEPARAGRAPH_H__
