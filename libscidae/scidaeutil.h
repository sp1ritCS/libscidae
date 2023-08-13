#ifndef __SCIDAEUTIL_H__
#define __SCIDAEUTIL_H__

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * ScidaeRectangle:
 * @origin: the coordinates of the origin of the rectangle
 * @size: the size of the rectangle
 *
 * A rectangle in context space, heavily inspired by graphenes
 * rectangle.
 */
struct _ScidaeRectangle {
	struct _ScidaePoint {
		guint x;
		guint y;
	} origin;
	struct _ScidaeSize {
		guint width;
		guint height;
	} size;
};
typedef struct _ScidaeRectangle ScidaeRectangle;

/**
 * SCIDAE_RECTANGLE_INIT:
 * @_x: the X coordinate of the origin
 * @_y: the Y coordinate of the origin
 * @_w: the width
 * @_h: the height
 *
 * Initializes a #ScidaeRectangle when declaring it.
 */
#define SCIDAE_RECTANGLE_INIT(_x,_y,_w,_h) \
	(ScidaeRectangle) { .origin = { .x = (_x), .y = (_y) }, .size = { .width = (_w), .height = (_h) } }

/**
 * scidae_rectangle_intersects:
 * @a: one rectangle
 * @b: the other rectangle
 *
 * Tests if the two rectangles intersect with each other.
 * Returns: %TRUE if they intersect, %FALSE otherwise
 */
gboolean scidae_rectangle_intersects(const ScidaeRectangle* a, const ScidaeRectangle* b);

G_END_DECLS

#endif // __SCIDAEUTIL_H__
