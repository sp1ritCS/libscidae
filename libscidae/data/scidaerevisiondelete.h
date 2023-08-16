#ifndef __SCIDAEREVISIONDELETE_H__
#define __SCIDAEREVISIONDELETE_H__

#include <glib-object.h>
#include <scidaerevision.h>

G_BEGIN_DECLS

#define SCIDAE_TYPE_REVISION_DELETE (scidae_revision_delete_get_type())
G_DECLARE_FINAL_TYPE (ScidaeRevisionDelete, scidae_revision_delete, SCIDAE, REVISION_DELETE, ScidaeRevision)

ScidaeRevision* scidae_revision_delete_new(gsize start, gsize length);

gsize scidae_revision_delete_get_start(ScidaeRevisionDelete* self);
void scidae_revision_delete_set_start(ScidaeRevisionDelete* self, gsize start);

gsize scidae_revision_delete_get_length(ScidaeRevisionDelete* self);
void scidae_revision_delete_set_length(ScidaeRevisionDelete* self, gsize length);

G_END_DECLS

#endif // __SCIDAEREVISIONDELETE_H__
