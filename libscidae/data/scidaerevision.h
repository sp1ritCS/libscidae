#ifndef __SCIDAEREVISION_H__
#define __SCIDAEREVISION_H__

#include <glib-object.h>
#include <scidaedataid.h>

G_BEGIN_DECLS

#define SCIDAE_TYPE_REVISION (scidae_revision_get_type())
G_DECLARE_DERIVABLE_TYPE (ScidaeRevision, scidae_revision, SCIDAE, REVISION, GObject)

struct _ScidaeRevisionClass {
	GObjectClass parent_class;

	const gchar*(*get_namespace)(ScidaeRevision* self);
	void(*serialize_inner)(ScidaeRevision* self, GByteArray* inner);

	gboolean(*apply)(ScidaeRevision* self, GString* target, GError** err);
};

ScidaeDataId* scidae_revision_get_identifier(ScidaeRevision* self);

void scidae_revision_set_identifier(ScidaeRevision* self, ScidaeDataId* identifier);

const gchar* scidae_revision_get_namespace(ScidaeRevision* self);

GBytes* scidae_revision_serialize(ScidaeRevision* self);

gboolean scidae_revision_apply(ScidaeRevision* self, GString* target, GError** err);

G_END_DECLS

#endif // __SCIDAEREVISION_H__
