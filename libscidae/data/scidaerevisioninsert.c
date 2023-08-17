#include "scidaerevisioninsert.h"

struct _ScidaeRevisionInsert {
	ScidaeRevision parent_instance;
	gchar* str;
	gsize pos;
};

G_DEFINE_TYPE (ScidaeRevisionInsert, scidae_revision_insert, SCIDAE_TYPE_REVISION)

enum {
	PROP_CONTENT = 1,
	PROP_POSITION,
	N_PROPERTIES
};
static GParamSpec* obj_properties[N_PROPERTIES] = { 0, };

static void scidae_revision_insert_object_finalize(GObject* object) {
	ScidaeRevisionInsert* self = SCIDAE_REVISION_INSERT(object);
	if (self->str)
		g_free(self->str);

	G_OBJECT_CLASS(scidae_revision_insert_parent_class)->finalize(object);
}

static void scidae_revision_insert_object_get_property(GObject* object, guint prop_id, GValue* val, GParamSpec* pspec) {
	ScidaeRevisionInsert* self = SCIDAE_REVISION_INSERT(object);
	switch (prop_id) {
		case PROP_CONTENT:
			g_value_set_string(val, scidae_revision_insert_get_content(self));
			break;
		case PROP_POSITION:
			g_value_set_uint64(val, scidae_revision_insert_get_position(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void scidae_revision_insert_object_set_property(GObject* object, guint prop_id, const GValue* val, GParamSpec* pspec) {
	ScidaeRevisionInsert* self = SCIDAE_REVISION_INSERT(object);
	switch (prop_id) {
		case PROP_CONTENT:
			scidae_revision_insert_set_content(self, g_value_get_string(val));
			break;
		case PROP_POSITION:
			scidae_revision_insert_set_position(self, g_value_get_uint64(val));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

const gchar* scidae_revision_insert_revision_get_namespace(G_GNUC_UNUSED ScidaeRevision* self) {
	return NULL; // Scidae.RevisionInsert is part of core
}

static const guchar scidae_revision_insert_serialization_id = 1;
void scidae_revision_insert_revision_serialize_inner(ScidaeRevision* revision, GByteArray* inner) {
	ScidaeRevisionInsert* self = SCIDAE_REVISION_INSERT(revision);

	g_byte_array_append(inner, &scidae_revision_insert_serialization_id, 1);

	g_byte_array_append(inner, (guchar*)self->str, strlen(self->str) + 1);

	guint64 pos = self->pos;
	g_byte_array_append(inner, (guchar*)&pos, sizeof(pos));
}

static gboolean scidae_revision_insert_revision_apply(ScidaeRevision* revision, GString* target, G_GNUC_UNUSED GError** err) {
	ScidaeRevisionInsert* self = SCIDAE_REVISION_INSERT(revision);
	g_string_insert(target, self->pos, self->str);
	return TRUE;
}

static void scidae_revision_insert_class_init(ScidaeRevisionInsertClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	ScidaeRevisionClass* revision_class = SCIDAE_REVISION_CLASS(class);

	object_class->finalize = scidae_revision_insert_object_finalize;
	object_class->get_property = scidae_revision_insert_object_get_property;
	object_class->set_property = scidae_revision_insert_object_set_property;

	revision_class->get_namespace = scidae_revision_insert_revision_get_namespace;
	revision_class->serialize_inner = scidae_revision_insert_revision_serialize_inner;
	revision_class->apply = scidae_revision_insert_revision_apply;

	obj_properties[PROP_CONTENT] = g_param_spec_string(
		"content",
		"Content",
		"The piece of text that gets inserted",
		NULL,
		G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY
	);

	obj_properties[PROP_POSITION] = g_param_spec_uint64(
		"position",
		"Position",
		"The position where the text gets inserted",
		0,
		G_MAXSIZE,
		0,
		G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY
	);

	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void scidae_revision_insert_init(ScidaeRevisionInsert* self) {
	self->str = NULL;
	self->pos = -1;
}

ScidaeRevision* scidae_revision_insert_new(ScidaeDataId* identifier, const gchar* content, gsize pos) {
	return g_object_new(SCIDAE_TYPE_REVISION_INSERT, "identifier", identifier, "content", content, "position", pos, NULL);
}

const gchar* scidae_revision_insert_get_content(ScidaeRevisionInsert* self) {
	g_return_val_if_fail(SCIDAE_IS_REVISION_INSERT(self), NULL);
	return self->str;
}

void scidae_revision_insert_set_content(ScidaeRevisionInsert* self, const gchar* content) {
	g_return_if_fail(SCIDAE_IS_REVISION_INSERT(self));

	if (g_strcmp0(self->str, content) == 0)
		return;

	if (self->str)
		g_free(self->str);
	self->str = g_strdup(content);

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_CONTENT]);
}

gsize scidae_revision_insert_get_position(ScidaeRevisionInsert* self) {
	g_return_val_if_fail(SCIDAE_IS_REVISION_INSERT(self), G_MAXSIZE);
	return self->pos;
}

void scidae_revision_insert_set_position(ScidaeRevisionInsert* self, gsize position) {
	g_return_if_fail(SCIDAE_IS_REVISION_INSERT(self));

	if (self->pos == position)
		return;
	self->pos = position;

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_POSITION]);
}
