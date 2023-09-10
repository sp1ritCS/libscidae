#include "scidaepangotext.h"

#include "scidaepangocontext.h"

#include <pango2/pangocairo.h>

struct _ScidaePangoTextMeasurementLine {
	ScidaeMeasurementLine parent;
	Pango2Line* line;
	GArray* rects;
};
typedef struct _ScidaePangoTextMeasurementLine ScidaePangoTextMeasurementLine;
static void scidae_pango_text_measurement_line_free(ScidaePangoTextMeasurementLine* self) {
	g_return_if_fail(self->parent.creator == SCIDAE_TYPE_PANGO_TEXT);
	pango2_line_free(self->line);
	g_array_unref(self->rects);
	g_free(self);
}

struct _ScidaePangoText {
	ScidaeText parent_instance;
};
G_DEFINE_TYPE (ScidaePangoText, scidae_pango_text, SCIDAE_TYPE_TEXT)

static ScidaeMeasurementResult scidae_pango_text_widget_measure(ScidaeWidget* widget, gint width, gint start_x, G_GNUC_UNUSED gboolean force, ScidaeWidgetMeasurementAttrs attrs, gpointer* previous) {
	ScidaePangoText* self = SCIDAE_PANGO_TEXT(widget);
	
	Pango2LineBreaker* breaker = *previous;
	if (!breaker) {
		ScidaeContext* context = scidae_widget_get_context(widget);
		breaker = pango2_line_breaker_new(scidae_pango_context_get_context(SCIDAE_PANGO_CONTEXT(context)));
		*previous = breaker;
		
		const gchar* body = scidae_text_get_body(SCIDAE_TEXT(self));
		
		g_autoptr(Pango2AttrList) attrs = pango2_attr_list_new();
		guint base_font_size = scidae_context_get_base_font_size(context);
		pango2_attr_list_change(attrs, pango2_attr_size_new(base_font_size));
		if (g_strcmp0(body, "Hello") == 0)
			pango2_attr_list_change(attrs, pango2_attr_size_new(base_font_size*1.5));
		pango2_line_breaker_add_text(breaker, body, -1, attrs);
	}

	/*ScidaePangoTextMeasurementLine* measurement = g_new(ScidaePangoTextMeasurementLine, 1);
	measurement->parent.creator = SCIDAE_TYPE_PANGO_TEXT;
	measurement->parent.del_fun = (GDestroyNotify)scidae_pango_text_measurement_line_free;*/
	ScidaePangoTextMeasurementLine* measurement = SCIDAE_MEASUREMENT_LINE_NEW(
		SCIDAE_TYPE_PANGO_TEXT,
		ScidaePangoTextMeasurementLine,
		(GDestroyNotify)scidae_pango_text_measurement_line_free
	);

	measurement->line = pango2_line_breaker_next_line(breaker, start_x, width - start_x, PANGO2_WRAP_WORD_CHAR, PANGO2_ELLIPSIZE_NONE);
	measurement->parent.props = 0;
	
	Pango2Rectangle ext;
	pango2_line_get_extents(measurement->line, NULL, &ext);
	
	measurement->parent.width = ext.width;
	measurement->parent.height = ext.height;
	measurement->parent.baseline = -ext.y;

	measurement->rects = g_array_new(FALSE, FALSE, sizeof(Pango2Rectangle));
	glong primary,secondary;
	scidae_text_get_cursors(SCIDAE_TEXT(self), &primary, &secondary);
	gint start = pango2_line_get_start_index(measurement->line);
	gint end = start + pango2_line_get_length(measurement->line);
	if (primary >= 0 && start <= primary && end > primary) {
		Pango2Rectangle rect;
		pango2_line_get_cursor_pos(measurement->line, primary, &rect, NULL);
		g_array_append_val(measurement->rects, rect);

		measurement->parent.props |= SCIDAE_MEASUREMENT_HAS_CURSOR;
	}
	if (secondary >= 0 && primary != secondary && start <= secondary && end > secondary) {
		Pango2Rectangle rect;
		pango2_line_get_cursor_pos(measurement->line, primary, NULL, &rect);
		g_array_append_val(measurement->rects, rect);
	}

	ScidaeMeasurementResult res;
	res.line = (ScidaeMeasurementLine*)measurement;
	if (pango2_line_breaker_has_line(breaker)) {
		res.result = SCIDAE_MEASUREMENT_PARTIAL;
	} else {
		res.result = SCIDAE_MEASUREMENT_FINISH;
		g_object_unref(breaker);
	}
	return res;
}

static GskRenderNode* scidae_pango_text_widget_render(G_GNUC_UNUSED ScidaeWidget* widget, ScidaeMeasurementLine* w_measurement, G_GNUC_UNUSED const ScidaeRectangle* area) {
	g_return_val_if_fail(w_measurement->creator == SCIDAE_TYPE_PANGO_TEXT, NULL);
	ScidaePangoTextMeasurementLine* measurement = (ScidaePangoTextMeasurementLine*)w_measurement;
	
	GskRenderNode* node = gsk_cairo_node_new(&GRAPHENE_RECT_INIT(
		pango2_units_to_double(0),
		pango2_units_to_double(0),
		pango2_units_to_double(measurement->parent.width),
		pango2_units_to_double(measurement->parent.height)
	));
	cairo_t* ctx = gsk_cairo_node_get_draw_context(node);
	
	/*cairo_rectangle(ctx, pango2_units_to_double(0),
		pango2_units_to_double(0),
		pango2_units_to_double(measurement->parent.width),
		pango2_units_to_double(measurement->parent.height));*/

	cairo_set_source_rgba(ctx, 0.0, 0.0, 0.0, 1.0);
	cairo_move_to(ctx, 0, pango2_units_to_double(measurement->parent.baseline));
	pango2_cairo_show_line(ctx, measurement->line);
	cairo_stroke(ctx);

	for (guint i = 0; i < measurement->rects->len; i++) {
		Pango2Rectangle rect = g_array_index(measurement->rects, Pango2Rectangle, i);
		cairo_rectangle(ctx,
			pango2_units_to_double(rect.x),
			pango2_units_to_double(rect.y + measurement->parent.baseline),
			pango2_units_to_double(rect.width),
			pango2_units_to_double(rect.height)
		);
		cairo_stroke(ctx);
	}

	cairo_destroy(ctx);
	
	return node;
}

void scidae_pango_widget_move_cursor_to_pos(ScidaeWidget* widget, ScidaeMeasurementLine* w_measurement, gint x, G_GNUC_UNUSED gint y, ScidaeWidgetCursorType cursor) {
	g_return_if_fail(w_measurement->creator == SCIDAE_TYPE_PANGO_TEXT);
	ScidaePangoTextMeasurementLine* measurement = (ScidaePangoTextMeasurementLine*)w_measurement;

	gint idx,it;
	pango2_line_x_to_index(measurement->line, x, &idx, &it);

	scidae_text_set_cursor(SCIDAE_TEXT(widget), idx + it);
}

static void scidae_pango_text_class_init(ScidaePangoTextClass* class) {
	ScidaeWidgetClass* widget_class = SCIDAE_WIDGET_CLASS(class);

	widget_class->measure = scidae_pango_text_widget_measure;
	widget_class->render = scidae_pango_text_widget_render;

	widget_class->move_cursor_to_pos = scidae_pango_widget_move_cursor_to_pos;
}

static void scidae_pango_text_init(ScidaePangoText*) {}

ScidaeText* scidae_pango_text_new(ScidaePangoContext* context, const gchar* body) {
	return g_object_new(SCIDAE_TYPE_PANGO_TEXT, "context", context, "body", body, NULL);
}
