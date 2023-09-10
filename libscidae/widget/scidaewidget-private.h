#ifndef __SCIDAEWIDGET_PRIVATE_H__
#define __SCIDAEWIDGET_PRIVATE_H__

#include <widget/scidaewidget.h>

G_BEGIN_DECLS

/*private api*/
/**
 * scidae_widget_set_cursor_start:
 * @self: the widget
 *
 * Set the cursor to the start of this widget.
 */
void scidae_widget_set_cursor_start(ScidaeWidget* self);

/**
 * scidae_widget_set_cursor_end:
 * @self: the widget
 *
 * Set the cursor to the end of this widget.
 */
void scidae_widget_set_cursor_end(ScidaeWidget* self);

/**
 * scidae_widget_drop_cursor:
 * @self: the widget
 *
 * Remove the cursor from the widget
 */
void scidae_widget_drop_cursor(ScidaeWidget* self);

G_END_DECLS

#endif // __SCIDAEWIDGET_PRIVATE_H__
