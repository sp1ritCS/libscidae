#include "scidaeparagraph.h"
#include "scidaecontainer.h"
#include "scidaewidget-alt.h"
#include "scidaetoplevel.h"

#include "scidaewidget-private.h"
#include "scidaeutil-private.h"

typedef struct {
	ScidaeMeasurementLine* line;
	gint xpos;
	gint yoffset;
	// the ScidaeWidget in the paragraph this represents
	GList* node;
} ScidaeParagraphMeasurementWrapper;
static void scidae_paragraph_measurement_wrapper_free_inner(ScidaeParagraphMeasurementWrapper* self) {
	scidae_measurement_line_unref(self->line);
}

static inline GArray* scidae_paragraph_create_line_measurement_buf(void) {
	GArray* measurements = g_array_new(FALSE, FALSE, sizeof(ScidaeParagraphMeasurementWrapper));
	g_array_set_clear_func(measurements, (GDestroyNotify)scidae_paragraph_measurement_wrapper_free_inner);
	return measurements;
}

typedef struct {
	// Contains ScidaeParagraphMeasurementWrapper
	GArray* measurements;
	gint ypos;
} ScidaeParagraphInternalMeasurementLineWrapper;
static void scidae_paragraph_internal_measurement_line_wrapper_free_inner(ScidaeParagraphInternalMeasurementLineWrapper* self) {
	g_array_unref(self->measurements);
}

struct _ScidaeParagraphMeasurementLine {
	ScidaeMeasurementLine parent;

	GArray* measurements;
	ScidaeParagraphMeasurementWrapper* m_cursor;
};

struct _ScidaeParagraph {
	ScidaeWidget parent_instance;

	GList* nodes;
	GHashTable* node_table;
	gint line_spacing;

	gboolean force_remeasure;
	GHashTable* remeasure_requests;

	ScidaeWidget* cursor_holder;
};

static void scidae_paragraph_container_iface_init(ScidaeContainerInterface* iface);
static void scidae_paragraph_toplevel_iface_init(ScidaeToplevelInterface* iface);
G_DEFINE_TYPE_WITH_CODE (ScidaeParagraph, scidae_paragraph, SCIDAE_TYPE_WIDGET,
	G_IMPLEMENT_INTERFACE(SCIDAE_TYPE_CONTAINER, scidae_paragraph_container_iface_init)
	G_IMPLEMENT_INTERFACE(SCIDAE_TYPE_TOPLEVEL, scidae_paragraph_toplevel_iface_init)
)

enum {
	PROP_LINE_SPACING = 1,
	N_PROPERTIES
};

GParamSpec* obj_properties[N_PROPERTIES] = { 0, };

static void scidae_paragraph_object_dispose(GObject* object) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(object);
	g_clear_pointer(&self->remeasure_requests, g_hash_table_unref);
	g_clear_pointer(&self->node_table, g_hash_table_unref);
	g_list_free_full(self->nodes, g_object_unref);
	
	G_OBJECT_CLASS(scidae_paragraph_parent_class)->dispose(object);
}

static void scidae_paragraph_object_constructed(GObject* object) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(object);
	self->line_spacing = scidae_context_to_units(scidae_widget_get_context(SCIDAE_WIDGET(self)), 0.f);
}

static void scidae_paragraph_object_get_property(GObject* object, guint prop_id, GValue* val, GParamSpec* pspec) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(object);
	switch (prop_id) {
		case PROP_LINE_SPACING:
			g_value_set_int(val, scidae_paragraph_get_line_spacing(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void scidae_paragraph_object_set_property(GObject* object, guint prop_id, const GValue* val, GParamSpec* pspec) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(object);
	switch (prop_id) {
		case PROP_LINE_SPACING:
			scidae_paragraph_set_line_spacing(self, g_value_get_int(val));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static gchar* scidae_paragraph_widget_get_markdown(ScidaeWidget* widget) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(widget);
	
	GString* ret = g_string_new(NULL);
	for (GList* i = self->nodes; i != NULL; i = i->next) {
		gchar* r = scidae_widget_get_markdown(i->data);
		g_string_append(ret, r);
		g_free(r);
	}

	return g_string_free(ret, FALSE);
}

static void scidae_paragraph_widget_merge_markdown_start(ScidaeWidget* widget, const gchar* text) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(widget);
	
	GList* first_node = g_list_first(self->nodes);
	ScidaeWidget* first = (first_node == NULL) ? NULL : first_node->data;
	if (!first) {
		ScidaeText* new = scidae_context_create_text_widget(scidae_widget_get_context(widget), text);
		scidae_paragraph_insert_child_at(self, SCIDAE_WIDGET(new), 0);
	}
	
	scidae_widget_merge_markdown_start(first, text);
}

static void scidae_paragraph_widget_merge_markdown_end(ScidaeWidget* widget, const gchar* text) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(widget);
	
	GList* last_node = g_list_last(self->nodes);
	ScidaeWidget* last = (last_node == NULL) ? NULL : last_node->data;
	if (!last) {
		ScidaeText* new = scidae_context_create_text_widget(scidae_widget_get_context(widget), text);
		scidae_paragraph_add_child(self, SCIDAE_WIDGET(new));
	}
	
	scidae_widget_merge_markdown_start(last, text);
}

static void scidae_paragraph_measurement_line_free(ScidaeParagraphMeasurementLine* self) {
	g_return_if_fail(self->parent.creator == SCIDAE_TYPE_PARAGRAPH);
	g_array_unref(self->measurements);
	g_free(self);
}

static void scidae_paragraph_widget_measure_make_line(ScidaeParagraph* self, ScidaeParagraphMeasurementLine* paragraph_measurement, gint* cur_y, GArray* current_line) {
	gint max_baseline_positive_offset = 0;
	gint max_baseline_negative_offset = 0;
	
	ScidaeParagraphInternalMeasurementLineWrapper line_wrapper = {
		.measurements = current_line,
		.ypos = *cur_y
	};
	
	for (guint i = 0; i < current_line->len; i++) {
		ScidaeParagraphMeasurementWrapper wrapper = g_array_index(current_line, ScidaeParagraphMeasurementWrapper, i);
		ScidaeMeasurementLine* measurement = wrapper.line;
		if (measurement->baseline > max_baseline_positive_offset)
			max_baseline_positive_offset = measurement->baseline;
		if (measurement->height - measurement->baseline > max_baseline_negative_offset)
			max_baseline_negative_offset = measurement->height - measurement->baseline;
	}
	
	for (guint i = 0; i < current_line->len; i++) {
		ScidaeParagraphMeasurementWrapper* wrapper = &g_array_index(current_line, ScidaeParagraphMeasurementWrapper, i);
		wrapper->yoffset = max_baseline_positive_offset - wrapper->line->baseline;
	}

	*cur_y = line_wrapper.ypos + max_baseline_positive_offset + max_baseline_negative_offset + self->line_spacing;
	
	g_array_append_val(paragraph_measurement->measurements, line_wrapper);
}

static ScidaeMeasurementResult scidae_paragraph_widget_measure_from(ScidaeWidget* widget, GList* start, gint width, gint start_x, gboolean force, G_GNUC_UNUSED gpointer* previous) {
	if (start_x != 0 && !force)
		return scidae_measurement_result_skip;
	
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(widget);
	/*ScidaeParagraphMeasurementLine* ret = g_new(ScidaeParagraphMeasurementLine, 1);
	ret->parent.creator = SCIDAE_TYPE_PARAGRAPH;
	ret->parent.del_fun = (GDestroyNotify)scidae_paragraph_measurement_line_free;*/
	ScidaeParagraphMeasurementLine* ret = SCIDAE_MEASUREMENT_LINE_NEW(
		SCIDAE_TYPE_PARAGRAPH,
		ScidaeParagraphMeasurementLine,
		(GDestroyNotify)scidae_paragraph_measurement_line_free
	);
	ret->m_cursor = NULL;
	
	ret->parent.width = width;
	ret->parent.baseline = 0;
	
	ret->measurements = g_array_new(FALSE, FALSE, sizeof(ScidaeParagraphInternalMeasurementLineWrapper));
	g_array_set_clear_func(ret->measurements, (GDestroyNotify)scidae_paragraph_internal_measurement_line_wrapper_free_inner);
	
	gint curX = 0;
	gint cur_y = 0;
	
	// TODO: implement support for caching and only measure at a certain node
	
	GArray* current_line = scidae_paragraph_create_line_measurement_buf();

	for (GList* i = start; i != NULL; i = i->next) {
		gpointer store = NULL;
		ScidaeMeasurementResult res;
		do {
			gboolean has_skipped = FALSE;
			res = scidae_widget_measure(SCIDAE_WIDGET(i->data), width, curX, FALSE, SCIDAE_WIDGET_MEASUREMENT_NO_ATTRS, &store);
redo:
			switch (res.result) {
				case SCIDAE_MEASUREMENT_FAILURE:
					g_critical("Received measurement failure. Something bad is probably happening.");
					goto next_node;
				case SCIDAE_MEASUREMENT_SKIP:
					if (has_skipped) {
						g_critical("%s has attempted to skip measuring twice. This is not allowed!", G_OBJECT_TYPE_NAME(i->data));
						goto next_node;
					}
					// makeLine
					scidae_paragraph_widget_measure_make_line(self, ret, &cur_y, current_line);
					current_line = scidae_paragraph_create_line_measurement_buf();
					curX = 0;
					res = scidae_widget_measure(SCIDAE_WIDGET(i->data), width, curX, TRUE, SCIDAE_WIDGET_MEASUREMENT_NO_ATTRS, &store);
					has_skipped = TRUE;
					goto redo;
					break;
				case SCIDAE_MEASUREMENT_PARTIAL: {
					ScidaeParagraphMeasurementWrapper wrapper = {
						.line = res.line,
						.xpos = curX,
						.yoffset = 0,
						.node = i
					};
					g_array_append_val(current_line, wrapper);
					if (res.line->props & SCIDAE_MEASUREMENT_HAS_CURSOR) {
						ret->m_cursor = &g_array_index(current_line, ScidaeParagraphMeasurementWrapper, current_line->len - 1);
						self->cursor_holder = i->data;
					}
					// makeLine
					scidae_paragraph_widget_measure_make_line(self, ret, &cur_y, current_line);
					current_line = scidae_paragraph_create_line_measurement_buf();
					curX = 0;
				} break;
				case SCIDAE_MEASUREMENT_FINISH: {
					ScidaeParagraphMeasurementWrapper wrapper = {
						.line = res.line,
						.xpos = curX,
						.yoffset = 0,
						.node = i
					};
					curX += res.line->width;
					g_array_append_val(current_line, wrapper);
					if (res.line->props & SCIDAE_MEASUREMENT_HAS_CURSOR) {
						ret->m_cursor = &g_array_index(current_line, ScidaeParagraphMeasurementWrapper, current_line->len - 1);
						self->cursor_holder = i->data;
					}
				} break;
				default:
					g_critical("Received unexpected result type (%d). Something bad is probably happening.", res.result);
					goto next_node;
			}
		} while (res.result != SCIDAE_MEASUREMENT_FINISH);
next_node:
	}
	scidae_paragraph_widget_measure_make_line(self, ret, &cur_y, current_line);
	
	ret->parent.height = cur_y;

	ScidaeMeasurementResult result = {
		.result = SCIDAE_MEASUREMENT_FINISH,
		.line = (ScidaeMeasurementLine*)ret
	};
	return result;
}

static ScidaeMeasurementResult scidae_paragraph_widget_measure(ScidaeWidget* widget, gint width, gint start_x, gboolean force, ScidaeWidgetMeasurementAttrs attrs, G_GNUC_UNUSED gpointer* previous) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(widget);
	GList* start = self->nodes;
	/*if (!self->force_remeasure) {
		if (g_hash_table_size(self->remeasure_requests) == 0) {
			// copy and dump measurement
		} else {
			for (; start != NULL; start = start->next) {
				if (g_hash_table_contains(self->remeasure_requests, start->data))
					break;
			}
		}
	}*/
		
	self->force_remeasure = FALSE;
	g_hash_table_remove_all(self->remeasure_requests);	
	return scidae_paragraph_widget_measure_from(widget, start, width, start_x, force, previous);
}

// note: takes ownership of node, caller gets ownership of return value
static GskRenderNode* scidae_paragraph_translate_render_node_by(GskRenderNode* node, ScidaeFromUnitsFun from_units, gint x, gint y) {
	if (x == 0 && y == 0)
		return node;
	g_autoptr(GskTransform) translation = gsk_transform_translate(NULL, &GRAPHENE_POINT_INIT(from_units(x), from_units(y)));
	GskRenderNode* translated = gsk_transform_node_new(node, translation);
	gsk_render_node_unref(node);
	return translated;
}

static GskRenderNode* scidae_paragraph_widget_render(G_GNUC_UNUSED ScidaeWidget* widget, ScidaeMeasurementLine* w_measurement, const ScidaeRectangle* area) {
	g_return_val_if_fail(w_measurement->creator == SCIDAE_TYPE_PARAGRAPH, NULL);
	ScidaeParagraphMeasurementLine* measurement = (ScidaeParagraphMeasurementLine*)w_measurement;
	
	g_autoptr(GPtrArray) children = g_ptr_array_new_with_free_func((GDestroyNotify)gsk_render_node_unref);
	for (guint i = 0; i < measurement->measurements->len; i++) {
		ScidaeParagraphInternalMeasurementLineWrapper wrapper = g_array_index(measurement->measurements, ScidaeParagraphInternalMeasurementLineWrapper, i);
		for (guint j = 0; j < wrapper.measurements->len; j++) {
			ScidaeParagraphMeasurementWrapper m_wrapper = g_array_index(wrapper.measurements, ScidaeParagraphMeasurementWrapper, j);

			if (!scidae_rectangle_intersects(area,
				&SCIDAE_RECTANGLE_INIT(
					m_wrapper.xpos,
					wrapper.ypos + m_wrapper.yoffset,
					m_wrapper.line->width,
					m_wrapper.line->height
				)
			))
				continue;

			ScidaeRectangle translated_area = scidae_rectangle_translate_copy(area, m_wrapper.xpos, wrapper.ypos + m_wrapper.yoffset);
			GskRenderNode* render = scidae_widget_render(m_wrapper.node->data, m_wrapper.line, &translated_area);
			GskRenderNode* translated = scidae_paragraph_translate_render_node_by(
				render,
				scidae_context_get_from_units(scidae_widget_get_context(widget)),
				m_wrapper.xpos,
				wrapper.ypos + m_wrapper.yoffset
			);
			g_ptr_array_add(children, translated);
		}
	}

	return gsk_container_node_new((GskRenderNode**)children->pdata, children->len);
}

// Editing
static void scidae_paragraph_widget_set_cursor_start(ScidaeWidget* widget) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(widget);
	if (self->cursor_holder)
		scidae_widget_drop_cursor(self->cursor_holder);
	if (self->nodes) {
		self->cursor_holder = self->nodes->data;
		scidae_widget_set_cursor_start(self->cursor_holder);
	} else
		g_critical("set_cursor_start called on paragraph without nodes");
}
static void scidae_paragraph_widget_set_cursor_end(ScidaeWidget* widget) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(widget);
	if (self->cursor_holder)
		scidae_widget_drop_cursor(self->cursor_holder);
	if (self->nodes) {
		self->cursor_holder = g_list_last(self->nodes)->data;
		scidae_widget_set_cursor_start(self->cursor_holder);
	} else
		g_critical("set_cursor_end called on paragraph without nodes");
}
static void scidae_paragraph_widget_drop_cursor(ScidaeWidget* widget) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(widget);
	scidae_widget_drop_cursor(self->cursor_holder);
	self->cursor_holder = NULL;
}
static void scidae_paragraph_widget_move_cursor_backward(ScidaeWidget* widget, ScidaeWidgetMovementModifier modifiers, ScidaeWidgetCursorType cursor) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(widget);
	scidae_widget_move_cursor_backward(self->cursor_holder, modifiers, cursor);
}

static void scidae_paragraph_widget_move_cursor_forward(ScidaeWidget* widget, ScidaeWidgetMovementModifier modifiers, ScidaeWidgetCursorType cursor) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(widget);
	scidae_widget_move_cursor_forward(self->cursor_holder, modifiers, cursor);
}

static void scidae_paragraph_widget_move_cursor_to_pos(G_GNUC_UNUSED ScidaeWidget* widget, ScidaeMeasurementLine* w_measurement, gint x, gint y, ScidaeWidgetCursorType cursor) {
	g_return_if_fail(w_measurement->creator == SCIDAE_TYPE_PARAGRAPH);
	ScidaeParagraphMeasurementLine* measurement = (ScidaeParagraphMeasurementLine*)w_measurement;

	ScidaeParagraphInternalMeasurementLineWrapper* found_wrapper = NULL;
	ScidaeParagraphMeasurementWrapper* found_segment = NULL;

	for (guint i = 0; i < measurement->measurements->len - 1; i++) {
		ScidaeParagraphInternalMeasurementLineWrapper wrapper = g_array_index(measurement->measurements, ScidaeParagraphInternalMeasurementLineWrapper, i);
		ScidaeParagraphInternalMeasurementLineWrapper next_wrapper = g_array_index(measurement->measurements, ScidaeParagraphInternalMeasurementLineWrapper, i+1);

		if (wrapper.ypos <= y && next_wrapper.ypos > y) {
			found_wrapper = &wrapper;
			break;
		}
	}
	if (!found_wrapper && measurement->measurements->len > 0)
		found_wrapper = &g_array_index(measurement->measurements, ScidaeParagraphInternalMeasurementLineWrapper, measurement->measurements->len - 1);

	if (!found_wrapper)
		g_return_if_reached();

	for (guint j = 0; j < found_wrapper->measurements->len - 1; j++) {
		ScidaeParagraphMeasurementWrapper segment = g_array_index(found_wrapper->measurements, ScidaeParagraphMeasurementWrapper, j);
		ScidaeParagraphMeasurementWrapper next_segment = g_array_index(found_wrapper->measurements, ScidaeParagraphMeasurementWrapper, j+1);

		if (segment.xpos <= x && next_segment.xpos > x) {
			found_segment = &segment;
			break;
		}
	}
	if (!found_segment && found_wrapper->measurements->len > 0)
		found_segment = &g_array_index(found_wrapper->measurements, ScidaeParagraphMeasurementWrapper, found_wrapper->measurements->len - 1);

	if (!found_segment)
		g_return_if_reached();

	scidae_widget_move_cursor_to_pos(found_segment->node->data, found_segment->line, x - found_segment->xpos, y - found_segment->yoffset - found_wrapper->ypos, cursor);
}

void scidae_paragraph_widget_insert_at_cursor(ScidaeWidget* widget, const gchar* text, gssize len) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(widget);
	scidae_widget_insert_at_cursor(self->cursor_holder, text, len);
}

void scidae_paragraph_widget_delete_backward(ScidaeWidget* widget, ScidaeWidgetMovementModifier modifiers) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(widget);
	scidae_widget_delete_backward(self->cursor_holder, modifiers);
}

void scidae_paragraph_widget_delete_forward(ScidaeWidget* widget, ScidaeWidgetMovementModifier modifiers) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(widget);
	scidae_widget_delete_forward(self->cursor_holder, modifiers);
}

static void scidae_paragraph_class_init(ScidaeParagraphClass* class) {
	GObjectClass* object_class= G_OBJECT_CLASS(class);
	ScidaeWidgetClass* widget_class = SCIDAE_WIDGET_CLASS(class);
	
	object_class->dispose = scidae_paragraph_object_dispose;
	object_class->constructed = scidae_paragraph_object_constructed;
	object_class->get_property = scidae_paragraph_object_get_property;
	object_class->set_property = scidae_paragraph_object_set_property;
	
	widget_class->get_markdown = scidae_paragraph_widget_get_markdown;
	widget_class->merge_markdown_start = scidae_paragraph_widget_merge_markdown_start;
	widget_class->merge_markdown_end = scidae_paragraph_widget_merge_markdown_end;

	widget_class->measure = scidae_paragraph_widget_measure;
	widget_class->render = scidae_paragraph_widget_render;
	
	widget_class->set_cursor_start = scidae_paragraph_widget_set_cursor_start;
	widget_class->set_cursor_end = scidae_paragraph_widget_set_cursor_end;
	widget_class->drop_cursor = scidae_paragraph_widget_drop_cursor;
	widget_class->move_cursor_backward = scidae_paragraph_widget_move_cursor_backward;
	widget_class->move_cursor_forward = scidae_paragraph_widget_move_cursor_forward;
	widget_class->move_cursor_to_pos = scidae_paragraph_widget_move_cursor_to_pos;
	widget_class->insert_at_cursor = scidae_paragraph_widget_insert_at_cursor;
	widget_class->delete_backward = scidae_paragraph_widget_delete_backward;
	widget_class->delete_forward = scidae_paragraph_widget_delete_forward;

	/**
	 * ScidaeParagraph:line-spacing: (attributes org.gtk.Property.get=scidae_paragraph_get_line_spacing org.gtk.Property.set=scidae_paragraph_set_line_spacing)
	 *
	 * The space betwen lines (in context units)
	 */
	obj_properties[PROP_LINE_SPACING] = g_param_spec_int(
		"line-spacing",
		"Line spacing",
		"Space between lines",
		0, G_MAXINT, 0,
		G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY
	);
	
	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void scidae_paragraph_init(ScidaeParagraph* self) {
	self->line_spacing = 0.f;
	self->nodes = NULL;
	self->node_table = g_hash_table_new(g_direct_hash, g_direct_equal);
	
	self->force_remeasure = FALSE;
	self->remeasure_requests = g_hash_table_new(g_direct_hash, g_direct_equal);

	self->cursor_holder = NULL;
}

static GList* scidae_paragraph_container_get_children(ScidaeContainer* container) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(container);
	return g_list_copy(self->nodes);
}

static ScidaeWidget* scidae_paragraph_container_get_prev(ScidaeContainer* container, ScidaeWidget* widget) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(container);

	GList* node = g_hash_table_lookup(self->node_table, widget);
	if (!node)
		return NULL;
	return node->prev == NULL ? NULL : node->prev->data;
}

static ScidaeWidget* scidae_paragraph_container_get_next(ScidaeContainer* container, ScidaeWidget* widget) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(container);

	GList* node = g_hash_table_lookup(self->node_table, widget);
	if (!node)
		return NULL;
	return node->next == NULL ? NULL : node->next->data;
}


static inline void scidae_paragraph_request_redraw(ScidaeParagraph* self) {
	scidae_toplevel_emit_redraw(SCIDAE_TOPLEVEL(self));

	ScidaeContainer* parent = scidae_widget_get_parent(SCIDAE_WIDGET(self));
	if (!parent)
		return;
	scidae_container_mark_child_remeasure(parent, SCIDAE_WIDGET(self));
}

static void scidae_paragraph_container_unparent(ScidaeContainer* container, ScidaeWidget* child) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(container);

	GList* node = g_hash_table_lookup(self->node_table, child);
	g_return_if_fail(node != NULL);
	g_hash_table_remove(self->node_table, child);

	if (node->data == self->cursor_holder) {
		if (node->prev) {
			self->cursor_holder = node->prev->data;
			scidae_widget_set_cursor_end(self->cursor_holder);
		} else if (node->next) {
			self->cursor_holder = node->next->data;
			scidae_widget_set_cursor_start(self->cursor_holder);
		} else {
			self->cursor_holder = NULL;
			g_debug("Paragraph %p is out of widgets :/\n", (void*)self);
		}
	}

	self->nodes = g_list_remove_link(self->nodes, node);
	g_object_unref(node->data);
	g_list_free(node);

	scidae_paragraph_request_redraw(self);
}

void scidae_paragraph_container_update_cursor(ScidaeContainer* container, ScidaeWidget* cursor_holder) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(container);
	if (self->cursor_holder == cursor_holder)
		return;
	g_return_if_fail(g_hash_table_contains(self->node_table, cursor_holder));
	if (self->cursor_holder)
		scidae_widget_drop_cursor(self->cursor_holder);
	self->cursor_holder = cursor_holder;
	scidae_paragraph_request_redraw(self);
}

void scidae_paragraph_container_mark_child_remeasure(ScidaeContainer* container, ScidaeWidget* child) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(container);
	g_hash_table_add(self->remeasure_requests, child);
	scidae_paragraph_request_redraw(self);
}

static void scidae_paragraph_container_iface_init(ScidaeContainerInterface* iface) {
	iface->get_children = scidae_paragraph_container_get_children;
	iface->get_prev = scidae_paragraph_container_get_prev;
	iface->get_next = scidae_paragraph_container_get_next;
	iface->unparent = scidae_paragraph_container_unparent;
	iface->update_cursor = scidae_paragraph_container_update_cursor;
	iface->mark_child_remeasure = scidae_paragraph_container_mark_child_remeasure;
}

gboolean scidae_paragraph_toplevel_should_be_remeasured(ScidaeToplevel* toplevel) {
	ScidaeParagraph* self = SCIDAE_PARAGRAPH(toplevel);
	return self->force_remeasure || g_hash_table_size(self->remeasure_requests) > 0;
}

static void scidae_paragraph_toplevel_iface_init(ScidaeToplevelInterface* iface) {
	iface->should_remeasure = scidae_paragraph_toplevel_should_be_remeasured;
}

ScidaeWidget* scidae_paragraph_new(ScidaeContext* context) {
	return g_object_new(SCIDAE_TYPE_PARAGRAPH, "context", context, NULL);
}

gint scidae_paragraph_get_line_spacing(ScidaeParagraph* self) {
	g_return_val_if_fail(SCIDAE_IS_PARAGRAPH(self), -1);
	return self->line_spacing;
}

void scidae_paragraph_set_line_spacing(ScidaeParagraph* self, gint spacing) {
	g_return_if_fail(SCIDAE_IS_PARAGRAPH(self));
	self->line_spacing = spacing;

	self->force_remeasure = TRUE;
	scidae_paragraph_request_redraw(self);
	
	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_LINE_SPACING]);
}

void scidae_paragraph_add_child(ScidaeParagraph* self, ScidaeWidget* child) {
	g_return_if_fail(SCIDAE_IS_PARAGRAPH(self));
	
	GList* link = g_list_alloc();
	link->data = g_object_ref_sink(child);
	//self->nodes = g_list_append(self->nodes, g_object_ref_sink(child));
	self->nodes = g_list_insert_before_link(self->nodes, NULL, link);
	g_hash_table_insert(self->node_table, child, link);

	scidae_widget_set_parent(child, SCIDAE_CONTAINER(self));

	if (self->cursor_holder == NULL) {
		self->cursor_holder = child;
		scidae_widget_set_cursor_start(self->cursor_holder);
	}
}

void scidae_paragraph_insert_child_at(ScidaeParagraph* self, ScidaeWidget* child, gint index) {
	g_return_if_fail(SCIDAE_IS_PARAGRAPH(self));
	
	//self->nodes = g_list_insert(self->nodes, g_object_ref_sink(child), index);
	GList* link = g_list_alloc();
	link->data = g_object_ref_sink(child);
	self->nodes = g_list_insert_before_link(self->nodes, g_list_nth(self->nodes, index), link);
	g_hash_table_insert(self->node_table, child, link);

	scidae_widget_set_parent(child, SCIDAE_CONTAINER(self));

	if (self->cursor_holder == NULL) {
		self->cursor_holder = child;
		scidae_widget_set_cursor_start(self->cursor_holder);
	}
}
