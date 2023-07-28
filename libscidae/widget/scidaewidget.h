#ifndef __SCIDAEWIDGET_H__
#define __SCIDAEWIDGET_H__

#include <glib-object.h>
#include <gsk/gsk.h>

#include <scidaecontext.h>

G_BEGIN_DECLS

#define SCIDAE_TYPE_MEASUREMENT_LINE (scidae_measurement_line_get_type())
/**
 * ScidaeMeasurementLine:
 * 
 * This structure should be returned by [vfunc@ScidaeWidget.measure].
 * It contains the measurment of one line, and widget specific behaviour. For
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

struct _ScidaeWidgetClass {
	GInitiallyUnownedClass parent_class;
	
	gchar*(*get_markdown)(ScidaeWidget* self);
	void(*merge_markdown_start)(ScidaeWidget* self, const gchar* text);
	void(*merge_markdown_end)(ScidaeWidget* self, const gchar* text);
	
	ScidaeMeasurementResult(*measure)(ScidaeWidget* self, gint width, gint start_x, gboolean force, gpointer* previous);
	GskRenderNode*(*render)(ScidaeWidget* self, ScidaeMeasurementLine* measurement);
};

/**
 * scidae_widget_get_markdown:
 * @self: the widget
 *
 * Get the content of any widget as markdown. The intended use is that the
 * previous widget may merge this one if the need arrises.
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
 * @previous: A marker that the widget can use to keep track the previous line (initially set to a reference pointing to `NULL`).
 *
 * Measures a single line of the widget.
 * Returns: (transfer full): The measurement result
 */
ScidaeMeasurementResult scidae_widget_measure(ScidaeWidget* self, gint width, gint start_x, gboolean force, gpointer* previous);

/**
 * scidae_widget_render:
 * @self: The widget to render
 * @measurement: The measurement of the widget
 *
 * Get a render node from the respecitve widget.
 * Returns: (transfer full): A render node
 */
GskRenderNode* scidae_widget_render(ScidaeWidget* self, ScidaeMeasurementLine* measurement);

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

G_END_DECLS

#endif // __SCIDAEWIDGET_H__