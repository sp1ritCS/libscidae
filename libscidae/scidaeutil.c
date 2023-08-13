#include "scidaeutil.h"

gboolean scidae_rectangle_intersects(const ScidaeRectangle* a, const ScidaeRectangle* b) {
	g_return_val_if_fail(a != NULL, FALSE);
	g_return_val_if_fail(b != NULL, FALSE);

	return !(
		((a->origin.x + a->size.width) < b->origin.x || (b->origin.x + b->size.width) < a->origin.x) // Check for no overlap along x-axis
		|| ((a->origin.y + a->size.height) < b->origin.y || (b->origin.y + b->size.height) < a->origin.y) // Check for no overlap along y-axis
	);
}
