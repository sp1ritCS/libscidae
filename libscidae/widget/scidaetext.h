#ifndef __SCIDAETEXT_H__
#define __SCIDAETEXT_H__

#include <glib-object.h>
#include <widget/scidaewidget.h>

G_BEGIN_DECLS

#define SCIDAE_TYPE_TEXT (scidae_text_get_type())
G_DECLARE_DERIVABLE_TYPE (ScidaeText, scidae_text, SCIDAE, TEXT, ScidaeWidget)

struct _ScidaeTextClass {
	ScidaeWidgetClass parent_class;
};

/**
 * scidae_text_get_body: (attributes org.gtk.Method.get_property=body)
 * @self: the text widget
 *
 * Get the current text of the widget.
 * Returns: (transfer none): the body
 */
const gchar* scidae_text_get_body(ScidaeText* self);

/**
 * scidae_text_set_body: (attributes org.gtk.Method.set_property=body)
 * @self: the text widget
 * @text: the new body
 *
 * Update the body of the text widget
 */
void scidae_text_set_body(ScidaeText* self, const gchar* text);

/**
 * scidae_text_get_cursors:
 * @self: the text widget
 * @master: (nullable): the location to store the master cursor
 * @slave: (nullable): the location to store the slave cursor
 *
 * Get the positions of the cursors of this widget.
 */
void scidae_text_get_cursors(ScidaeText* self, glong* master, glong* slave);

/**
 * scidae_text_set_cursor:
 * @self: the text widget
 * @action: the cursor action
 * @cursor: the cursor index
 *
 * Set both master & slave cursors to `cursor`.
 */
void scidae_text_set_cursor(ScidaeText* self, ScidaeWidgetCursorAction action, glong cursor);

G_END_DECLS

#endif // __SCIDAETEXT_H__
