#ifndef __SCIDAEREVISIONINSERT_H__
#define __SCIDAEREVISIONINSERT_H__

#include <glib-object.h>
#include <scidaerevision.h>

G_BEGIN_DECLS

#define SCIDAE_TYPE_REVISION_INSERT (scidae_revision_insert_get_type())
G_DECLARE_FINAL_TYPE (ScidaeRevisionInsert, scidae_revision_insert, SCIDAE, REVISION_INSERT, ScidaeRevision)

ScidaeRevision* scidae_revision_insert_new(const gchar* content, gsize pos);

const gchar* scidae_revision_insert_get_content(ScidaeRevisionInsert* self);
void scidae_revision_insert_set_content(ScidaeRevisionInsert* self, const gchar* content);

gsize scidae_revision_insert_get_position(ScidaeRevisionInsert* self);
void scidae_revision_insert_set_position(ScidaeRevisionInsert* self, gsize position);

G_END_DECLS

#endif // __SCIDAEREVISIONINSERT_H__
