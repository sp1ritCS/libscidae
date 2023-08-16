#include "scidaerevisiondelete.h"

struct _ScidaeRevisionDelete {
	ScidaeRevision parent_instance;

	gsize start;
	gsize len;
};

G_DEFINE_TYPE (ScidaeRevisionDelete, scidae_revision_delete, SCIDAE_TYPE_REVISION)

enum {
	PROP_START = 1,
	PROP_LENGTH,
	N_PROPERTIES
};
static GParamSpec* obj_properties[N_PROPERTIES] = { 0, };

static void scidae_revision_delete_object_get_property(GObject* object, guint prop_id, GValue* val, GParamSpec* pspec) {
	ScidaeRevisionDelete* self = SCIDAE_REVISION_DELETE(object);
	switch (prop_id) {
		case PROP_START:
			g_value_set_uint64(val, scidae_revision_delete_get_start(self));
			break;
		case PROP_LENGTH:
			g_value_set_uint64(val, scidae_revision_delete_get_length(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void scidae_revision_delete_object_set_property(GObject* object, guint prop_id, const GValue* val, GParamSpec* pspec) {
	ScidaeRevisionDelete* self = SCIDAE_REVISION_DELETE(object);
	switch (prop_id) {
		case PROP_START:
			scidae_revision_delete_set_start(self, g_value_get_uint64(val));
			break;
		case PROP_LENGTH:
			scidae_revision_delete_set_length(self, g_value_get_uint64(val));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static gboolean scidae_revision_delete_revision_apply(ScidaeRevision* revision, GString* target, G_GNUC_UNUSED GError** err) {
	ScidaeRevisionDelete* self = SCIDAE_REVISION_DELETE(revision);
	g_string_erase(target, self->start, self->len);
	return TRUE;
}

static void scidae_revision_delete_class_init(ScidaeRevisionDeleteClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	ScidaeRevisionClass* revision_class = SCIDAE_REVISION_CLASS(class);

	object_class->get_property = scidae_revision_delete_object_get_property;
	object_class->set_property = scidae_revision_delete_object_set_property;

	revision_class->apply = scidae_revision_delete_revision_apply;

	obj_properties[PROP_START] = g_param_spec_uint64(
		"start",
		"Start",
		"The index where to start to delete",
		0,
		G_MAXSIZE,
		0,
		G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY
	);

	obj_properties[PROP_LENGTH] = g_param_spec_uint64(
		"length",
		"Length",
		"The the amount of bytes to delete",
		0,
		G_MAXSIZE,
		0,
		G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY
	);

	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void scidae_revision_delete_init(ScidaeRevisionDelete* self) {
	self->start = 0;
	self->len = 0;
}

ScidaeRevision* scidae_revision_delete_new(gsize start, gsize length) {
	return g_object_new(SCIDAE_TYPE_REVISION_DELETE, "start", start, "length", length, NULL);
}

gsize scidae_revision_delete_get_start(ScidaeRevisionDelete* self) {
	g_return_val_if_fail(SCIDAE_REVISION_DELETE(self), 0);
	return self->start;
}

void scidae_revision_delete_set_start(ScidaeRevisionDelete* self, gsize start) {
	g_return_if_fail(SCIDAE_IS_REVISION_DELETE(self));

	if (self->start == start)
		return;
	self->start = start;
	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_START]);
}

gsize scidae_revision_delete_get_length(ScidaeRevisionDelete* self) {
	g_return_val_if_fail(SCIDAE_REVISION_DELETE(self), 0);
	return self->len;
}

void scidae_revision_delete_set_length(ScidaeRevisionDelete* self, gsize length) {
	g_return_if_fail(SCIDAE_IS_REVISION_DELETE(self));

	if (self->len == length)
		return;
	self->len = length;
	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_LENGTH]);
}
