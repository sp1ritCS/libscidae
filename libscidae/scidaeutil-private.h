#ifndef __SCIDAEUTIL_PRIVATE_H__
#define __SCIDAEUTIL_PRIVATE_H__

#include <scidaeutil.h>
#include <widget/scidaewidget.h>

G_BEGIN_DECLS

/**
 * scidae_rectangle_translate_copy: (skip):
 * @self: a rectangle
 * @x: the X coordinate for horizontal translation
 * @y: the Y coordinate for vertical translation
 *
 * Copy the rectangle and translate it by point (x|y).
 * Returns: the copied rectangle
 */
static inline ScidaeRectangle scidae_rectangle_translate_copy(const ScidaeRectangle* self, guint x, guint y) {
	return SCIDAE_RECTANGLE_INIT(self->origin.x + x, self->origin.y + y, self->size.width, self->size.height);
}

G_END_DECLS

#endif // __SCIDAEUTIL_PRIVATE_H__
