#ifndef __SCIDAEREVISION_H__
#define __SCIDAEREVISION_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define SCIDAE_TYPE_REVISION (scidae_revision_get_type())
G_DECLARE_DERIVABLE_TYPE (ScidaeRevision, scidae_revision, SCIDAE, REVISION, GObject)

struct _ScidaeRevisionClass {
	GObjectClass parent_class;

	gboolean(*apply)(ScidaeRevision* self, GString* target, GError** err);
};

gboolean scidae_revision_apply(ScidaeRevision* self, GString* target, GError** err);

G_END_DECLS

#endif // __SCIDAEREVISION_H__
