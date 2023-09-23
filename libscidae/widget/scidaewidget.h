#ifndef __SCIDAEWIDGET_H__
#define __SCIDAEWIDGET_H__

#include <glib-object.h>
#include <gsk/gsk.h>

#include <scidaecontext.h>
#include <scidaeutil.h>

G_BEGIN_DECLS

typedef enum {
	SCIDAE_MEASUREMENT_HAS_MASTER_CURSOR = 1 << 0,
	SCIDAE_MEASUREMENT_HAS_SLAVE_CURSOR = 1 << 1,
//	SCIDAE_MEASUREMENT_ENDS_WITH_SELECTION = 1 << 2,
} ScidaeMeasurementProps;

#define SCIDAE_TYPE_MEASUREMENT_LINE (scidae_measurement_line_get_type())
/**
 * ScidaeMeasurementLine:
 * 
 * This structure should be returned by [vfunc@ScidaeWidget.measure].
 * It contains the measurement of one line, and widget specific behavior. For
 * this reason, the [iface@ScidaeContainer] that requested the measurement will
 * invoke the `del_fun` once the measurement is unneeded.
 */
struct _ScidaeMeasurementLine {
	GType creator;
	gatomicrefcount ref;
	GDestroyNotify del_fun;
	
	gint width;
	gint height;
	gint baseline;

	ScidaeMeasurementProps props;
};
typedef struct _ScidaeMeasurementLine ScidaeMeasurementLine;

GType scidae_measurement_line_get_type(void);

/**
 * scidae_measurement_line_alloc:
 * @size: the size (in bytes) of the measurement
 * @creator: the GType of the widget that created this measurement
 * @del_fun: a function pointer to a function freeing the measurement
 *
 * Allocate a new measurement line of a widget. Don't use this, use
 * [func@MEASUREMENT_LINE_NEW] instead.
 * Returns: (transfer full): the allocated measurement line
 */
static inline ScidaeMeasurementLine* scidae_measurement_line_alloc(gsize size, GType creator, GDestroyNotify del_fun) {
	ScidaeMeasurementLine* line = g_malloc(size);
	line->creator = creator;
	line->props = 0;
	g_atomic_ref_count_init(&line->ref);
	line->del_fun = del_fun;
	return line;
}

/**
 * SCIDAE_MEASUREMENT_LINE_NEW:
 * @creator: the GType of the widget that created this measurement
 * @type: the type of the measurement
 * @del_fun: a function pointer to a function freeing the measurement
 *
 * Allocate a new measurement line of a widget.
 * Returns: (transfer full): the allocated measurement line
 */
#define SCIDAE_MEASUREMENT_LINE_NEW(creator, type, del_fun) \
	(type*)scidae_measurement_line_alloc(sizeof(type), creator, del_fun)

/**
 * scidae_measurement_line_ref:
 * @self: (transfer none): the measurement line to upref
 *
 * Increments the refcount of a measurement line by one.
 * Returns: (transfer full): the measurement line
 */
ScidaeMeasurementLine* scidae_measurement_line_ref(ScidaeMeasurementLine* self);

/**
 * scidae_measurement_line_unref:
 * @self: (transfer full): the measurement to free
 * 
 * Decrements the refcount of a measurement line.
 */
void scidae_measurement_line_unref(ScidaeMeasurementLine* self);

enum _ScidaeMeasurementResultType {
	SCIDAE_MEASUREMENT_FAILURE = 0,
	SCIDAE_MEASUREMENT_SKIP,
	SCIDAE_MEASUREMENT_PARTIAL,
	SCIDAE_MEASUREMENT_FINISH
};
typedef enum _ScidaeMeasurementResultType ScidaeMeasurementResultType;

struct _ScidaeMeasurementResult {
	ScidaeMeasurementResultType result;
	union {
		ScidaeMeasurementLine* line;
		// maybe consider adding GError
	};
};
typedef struct _ScidaeMeasurementResult ScidaeMeasurementResult;

static const ScidaeMeasurementResult scidae_measurement_result_failure = {
	.result = SCIDAE_MEASUREMENT_FAILURE,
	.line = NULL
};
static const ScidaeMeasurementResult scidae_measurement_result_skip = {
	.result = SCIDAE_MEASUREMENT_SKIP
};

/**
 * ScidaeWidget:
 * 
 * This is the base that all other buffer widgets inherit from.
 */
#define SCIDAE_TYPE_WIDGET (scidae_widget_get_type())
G_DECLARE_DERIVABLE_TYPE (ScidaeWidget, scidae_widget, SCIDAE, WIDGET, GInitiallyUnowned)

typedef enum {
	SCIDAE_WIDGET_MEASUREMENT_NO_ATTRS = 0,
	SCIDAE_WIDGET_MEASUREMENT_CONTINUES_SELECTION = 1 << 0,
} ScidaeWidgetMeasurementAttrs;

typedef enum {
	SCIDAE_CURSOR_TYPE_MASTER = 1 << 0,
	SCIDAE_CURSOR_TYPE_SLAVE = 1 << 1,
	SCIDAE_CURSOR_TYPE_BOTH = SCIDAE_CURSOR_TYPE_MASTER | SCIDAE_CURSOR_TYPE_SLAVE
} ScidaeWidgetCursorType;
typedef enum {
	SCIDAE_MODIFY_CURSOR_DROP = 0,
	SCIDAE_MODIFY_CURSOR_MOVE_START,
	SCIDAE_MODIFY_CURSOR_MOVE_END,
	SCIDAE_MODIFY_CURSOR_RESET,
} ScidaeWidgetModifyCursorAction;

typedef enum {
	SCIDAE_DIRECTION_BACKWARD = -1,
	SCIDAE_DIRECTION_FORWARD = 1
} ScidaeDirection;

typedef enum {
	SCIDAE_MOVEMENT_MODIFIER_CONTROL = 1 << 0,
	SCIDAE_MOVEMENT_MODIFIER_ALT = 1 << 1
} ScidaeWidgetMovementModifier;

typedef enum {
	SCIDAE_CURSOR_ACTION_MOVE,
	SCIDAE_CURSOR_ACTION_MOVE_MASTER
} ScidaeWidgetCursorAction;

typedef enum {
	SCIDAE_DELETE_REGION_CURSOR_FROM_START,
	SCIDAE_DELETE_REGION_CURSOR_TO_END
} ScidaeWidgetDeleteRegion;

struct _ScidaeWidgetClass {
	GInitiallyUnownedClass parent_class;
	
	gchar*(*get_markdown)(ScidaeWidget* self);
	void(*merge_markdown_start)(ScidaeWidget* self, const gchar* text);
	void(*merge_markdown_end)(ScidaeWidget* self, const gchar* text);
	
	ScidaeMeasurementResult(*measure)(ScidaeWidget* self, gint width, gint start_x, gboolean force, ScidaeWidgetMeasurementAttrs attrs, gpointer* previous);
	GskRenderNode*(*render)(ScidaeWidget* self, ScidaeMeasurementLine* measurement, const ScidaeRectangle* area);

	/* BEGIN EDITING FACILITES */
	void(*modify_cursor)(ScidaeWidget* self, ScidaeWidgetCursorType cursor, ScidaeWidgetModifyCursorAction action);
	void(*move_cursor)(ScidaeWidget* self, ScidaeDirection direction, ScidaeWidgetMovementModifier modifiers, ScidaeWidgetCursorAction action);
	void(*move_cursor_to_pos)(ScidaeWidget* self, ScidaeMeasurementLine* measurement, gint x, gint y, ScidaeWidgetCursorAction action);
	gint(*get_cursor_x)(ScidaeWidget* self, ScidaeMeasurementLine* measurement);
	void(*modify_cursor_on_measurement)(ScidaeWidget* self, ScidaeMeasurementLine* measurement, ScidaeDirection direction, ScidaeWidgetCursorAction action);
	void(*insert_at_cursor)(ScidaeWidget* self, const gchar* text, gssize len);
	void(*delete)(ScidaeWidget* self, ScidaeDirection direction, ScidaeWidgetMovementModifier modifiers);
	void(*delete_region)(ScidaeWidget* self, ScidaeWidgetDeleteRegion region);
};

/**
 * scidae_widget_get_markdown:
 * @self: the widget
 *
 * Get the content of any widget as markdown. The intended use is that the
 * previous widget may merge this one if the need arises.
 * Returns: (transfer full): the content as markdown
 */
gchar* scidae_widget_get_markdown(ScidaeWidget* self);

/**
 * scidae_widget_merge_markdown_start:
 * @self: the widget
 * @text: the markdown text to merge
 *
 * Merge a string of text onto the start of this widget.
 */
void scidae_widget_merge_markdown_start(ScidaeWidget* self, const gchar* text);

/**
 * scidae_widget_merge_markdown_end:
 * @self: the widget
 * @text: the markdown text to merge
 *
 * Merge a string of text onto the end of this widget.
 */
void scidae_widget_merge_markdown_end(ScidaeWidget* self, const gchar* text);

// TODO: either figure out a decent way to box this, or provide alternative bindable API
/**
 * scidae_widget_measure: (skip)
 * @self: The widget to measure
 * @width: The total width of the surface that this will be rendered to
 * @start_x: The position that the first line will start at (should be less than `width`)
 * @force: Force rendering, even if `width-start_x` is less than the necessary space (set if the previous result was of type `SCIDAE_MEASUREMENT_SKIP`)
 * @attrs: Attributes about the current state of the measurement pipeline
 * @previous: A marker that the widget can use to keep track the previous line (initially set to a reference pointing to `NULL`).
 *
 * Measures a single line of the widget.
 * Returns: (transfer full): The measurement result
 */
ScidaeMeasurementResult scidae_widget_measure(ScidaeWidget* self, gint width, gint start_x, gboolean force, ScidaeWidgetMeasurementAttrs attrs, gpointer* previous);

/**
 * scidae_widget_render:
 * @self: The widget to render
 * @measurement: The measurement of the widget
 * @area: (nullable): The area that will be rendered
 *
 * Get a render node from the respective widget. If area is %NULL, the
 * entire area as specified in %measurement will be rendered.
 * Returns: (transfer full): A render node
 */
GskRenderNode* scidae_widget_render(ScidaeWidget* self, ScidaeMeasurementLine* measurement, const ScidaeRectangle* area);

/**
 * scidae_widget_get_context: (attributes org.gtk.Method.get_property=context)
 * @self: the widget
 *
 * Get the context a widget uses
 * Returns: (transfer none): the context
 */
ScidaeContext* scidae_widget_get_context(ScidaeWidget* self);

/**
 * scidae_widget_set_context: (attributes org.gtk.Method.set_property=context)
 * @self: the widget
 * @context: the new context
 *
 * Set a new context of a widget.
 */
void scidae_widget_set_context(ScidaeWidget* self, ScidaeContext* context);

/* BEGIN EDITING FACILITIES */

/**
 * scidae_widget_modify_cursor:
 * @self: the widget to modify the state of
 * @cursor: what cursors should `action` apply to
 * @action: the action to apply (drop, move to start, move to end)
 *
 * Modify the cursor state of a widget. Since this doesn't cause bubbled
 * updating of parent widgets, this method should only be called internally or
 * if you are know what you are doing (you probably don't).
 */
void scidae_widget_modify_cursor(ScidaeWidget* self, ScidaeWidgetCursorType cursor, ScidaeWidgetModifyCursorAction action);

/**
 * scidae_widget_move_cursor:
 * @self: the widget to move the cursor in
 * @direction: the direction to move the cursor in
 * @modifiers: modifier keys depressed during the moving action
 * @action: the cursor action to execute
 *
 * Moves the cursor one unit (specified by `modifiers`) in the direction
 * specified by `direction`.
 */
void scidae_widget_move_cursor(ScidaeWidget* self, ScidaeDirection direction, ScidaeWidgetMovementModifier modifiers, ScidaeWidgetCursorAction action);

/**
 * scidae_widget_move_cursor_to_pos:
 * @self: the widget to move the cursor to
 * @measurement: the current measurement of `self`
 * @x: the approximate x-coordinate of the new cursor position in context units
 * @y: the approximate y-coordinate of the new cursor position in context units
 * @action: the cursor action to execute
 *
 * Moves the cursor to a position approximatly specified by the x and y
 * coordinates.
 */
void scidae_widget_move_cursor_to_pos(ScidaeWidget* self, ScidaeMeasurementLine* measurement, gint x, gint y, ScidaeWidgetCursorAction action);

/**
 * scidae_widget_get_cursor_x:
 * @self: the widget
 * @measurement: the current measurement of `self`
 *
 * Get the current x-coordinate of the cursor based on measurement.
 * Returns: the x-coordinate of the cursor
 */
gint scidae_widget_get_cursor_x(ScidaeWidget* self, ScidaeMeasurementLine* measurement);

/**
 * scidae_widget_modify_cursor_on_measurement:
 * @self: the widget
 * @measurement: the current measurement of `self`
 * @direction: the direction to move the cursor in
 * @action: the cursor action to execute
 *
 * Moves the cursor to the start or end of a measurement based on
 * `direction`,
 */
void scidae_widget_modify_cursor_on_measurement(ScidaeWidget* self, ScidaeMeasurementLine* measurement, ScidaeDirection direction, ScidaeWidgetCursorAction action);

/**
 * scidae_widget_insert_at_cursor:
 * @self: the widget to insert text into
 * @text: the text to insert
 * @len: the length of the text to insert (might be -1 if the text is NULL terminated)
 *
 * Inserts a string of text at the current position of the cursor. If a
 * selection exists, it should delete the selection first.
 */
void scidae_widget_insert_at_cursor(ScidaeWidget* self, const gchar* text, gssize len);

/**
 * scidae_widget_delete:
 * @self: the widget to delete text from
 * @direction: the direction to delete text from
 * @modifiers: modifier keys depressed during the moving action
 *
 * Deletes a unit of text (based on `modifiers`) in the direction
 * specified by `direction`. If a selection exists it deletes the
 * selection instead.
 */
void scidae_widget_delete(ScidaeWidget* self, ScidaeDirection direction, ScidaeWidgetMovementModifier modifiers);

G_END_DECLS

#endif // __SCIDAEWIDGET_H__
