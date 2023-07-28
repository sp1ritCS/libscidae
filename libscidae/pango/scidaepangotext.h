#ifndef __SCIDAEPANGOTEXT_H__
#define __SCIDAEPANGOTEXT_H__

#include <glib-object.h>
#include <pango/scidaepangocontext.h>
#include <widget/scidaetext.h>

#include <pango2/pango.h>

G_BEGIN_DECLS

#define SCIDAE_TYPE_PANGO_TEXT (scidae_pango_text_get_type())
G_DECLARE_FINAL_TYPE (ScidaePangoText, scidae_pango_text, SCIDAE, PANGO_TEXT, ScidaeText)

ScidaeText* scidae_pango_text_new(ScidaePangoContext* context, const gchar* body);

G_END_DECLS

#endif // __SCIDAEPANGOTEXT_H__
