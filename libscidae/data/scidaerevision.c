#include "scidaerevision.h"

G_DEFINE_ABSTRACT_TYPE (ScidaeRevision, scidae_revision, G_TYPE_OBJECT)

static void scidae_revision_class_init(ScidaeRevisionClass* class) {
	class->apply = NULL;
}

static void scidae_revision_init(G_GNUC_UNUSED ScidaeRevision* self) {}

gboolean scidae_revision_apply(ScidaeRevision* self, GString* target, GError** err) {
	g_return_val_if_fail(SCIDAE_IS_REVISION(self), FALSE);
	ScidaeRevisionClass* class = SCIDAE_REVISION_GET_CLASS(self);
	g_return_val_if_fail(class->apply, FALSE);
	return class->apply(self, target, err);
}
