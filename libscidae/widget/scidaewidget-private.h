#ifndef __SCIDAEWIDGET_PRIVATE_H__
#define __SCIDAEWIDGET_PRIVATE_H__

#include <widget/scidaewidget.h>

G_BEGIN_DECLS

/* private api */
/**
 * scidae_widget_delete_region:
 * @self: the widget
 * @region: the region to delete
 *
 * Entirely removes a region of text from the widget. The region
 * separator is any cursor and `region` specifies if either the contents
 * from start to cursor or from cursor to end should be deleted.
 */
void scidae_widget_delete_region(ScidaeWidget* self, ScidaeWidgetDeleteRegion region);

G_END_DECLS

#endif // __SCIDAEWIDGET_PRIVATE_H__
