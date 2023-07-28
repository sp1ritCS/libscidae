#ifndef __SCIDAEPANGOCONTEXT_H__
#define __SCIDAEPANGOCONTEXT_H__

#include <glib-object.h>
#include <scidaecontext.h>

#include <pango2/pango.h>

G_BEGIN_DECLS

#define SCIDAE_TYPE_PANGO_CONTEXT (scidae_pango_context_get_type())
G_DECLARE_FINAL_TYPE (ScidaePangoContext, scidae_pango_context, SCIDAE, PANGO_CONTEXT, ScidaeContext)

/**
 * scidae_pango_context_new:
 * @context: the pango context to wrap
 *
 * Create a ScidaeContext for Pango2
 * Returns: (transfer full): the new context
 */
ScidaeContext* scidae_pango_context_new(Pango2Context* context);

/**
 * scidae_pango_context_get_context: (attributes org.gtk.Method.get_property=pango-context)
 * @self: the scidae context
 *
 * Get the [class@Pango2.Context] of the scidae pango context wrapper.
 * Returns: (transfer none): the pango context
 */
Pango2Context* scidae_pango_context_get_context(ScidaePangoContext* self);

/**
 * scidae_pango_context_set_context: (attributes org.gtk.Method.set_property=pango-context)
 * @self: the scidae context
 * @context: the new Pango2 context
 *
 * Set a new Pango2 context in the scidae pango context wrapper.
 */
void scidae_pango_context_set_context(ScidaePangoContext* self, Pango2Context* context);

G_END_DECLS

#endif // __SCIDAEPANGOCONTEXT_H__
