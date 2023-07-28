#ifndef __SCIDAECONTEXT_H__
#define __SCIDAECONTEXT_H__

#include <glib-object.h>

G_BEGIN_DECLS

typedef gdouble(*ScidaeFromUnitsFun)(gint units);
typedef gint(*ScidaeToUnitsFun)(gdouble display);

#define SCIDAE_TYPE_CONTEXT (scidae_context_get_type())
G_DECLARE_DERIVABLE_TYPE (ScidaeContext, scidae_context, SCIDAE, CONTEXT, GObject)

typedef struct _ScidaeText ScidaeText;

struct _ScidaeContextClass {
	GObjectClass parent_class;
	
	ScidaeFromUnitsFun(*get_from_units)(ScidaeContext* self);
	ScidaeToUnitsFun(*get_to_units)(ScidaeContext* self);
	
	ScidaeText*(*create_text_widget)(ScidaeContext* self, const gchar* text);
};

/**
 * scidae_context_get_from_units: (skip):
 * @self: the context
 *
 * Get the function pointer for the function that converts context units to
 * display-like format (where the pixels are within the double).
 * Returns: (transfer none): function pointer to a function that takes an integer and returns a double
 */
ScidaeFromUnitsFun scidae_context_get_from_units(ScidaeContext* self);

/**
 * scidae_context_get_to_units: (skip):
 * @self: the context
 *
 * Get the function pointer for the function that converts display units context-
 * units.
 * Returns: (transfer none): function pointer to a function that takes a double and returns an integer
 */
ScidaeToUnitsFun scidae_context_get_to_units(ScidaeContext* self);

/**
 * scidae_context_create_text_widget:
 * @self: the context
 * @text: The initial content of the widget
 *
 * Create a new text widget mapped to a specific context.
 * Returns: (transfer full): a new text widget
 */
ScidaeText* scidae_context_create_text_widget(ScidaeContext* self, const gchar* text);

/**
 * scidae_context_from_units:
 * @self: the context
 * @units: the context units
 *
 * Converts context units to display units. This is mostly for language bindings,
 * for better performance cache the output from [method@ScidaeContext.get_from_units]
 * and use that.
 * Returns: the display units
 */
gdouble scidae_context_from_units(ScidaeContext* self, gint units);

/**
 * scidae_context_to_units:
 * @self: the context
 * @display: the display units
 *
 * Converts display units to context units. This is mostly for language bindings,
 * for better performance cache the output from [method@ScidaeContext.get_to_units]
 * and use that.
 * Returns: the context units
 */
gint scidae_context_to_units(ScidaeContext* self, gdouble display);

G_END_DECLS

#endif // __SCIDAECONTEXT_H__
