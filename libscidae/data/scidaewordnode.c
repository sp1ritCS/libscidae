#include "scidaewordnode.h"

struct _ScidaeWordNode {
	GObject parent_instance;

	GString* current;
	GPtrArray* revisions;

	ScidaeWordNode* prev;
	ScidaeWordNode* next;
};

G_DEFINE_TYPE (ScidaeWordNode, scidae_word_node, G_TYPE_OBJECT)

enum {
	PROP_REVISIONS = 1,
	N_PROPERTIES
};
static GParamSpec* obj_properties[N_PROPERTIES] = { 0, };

static void scidae_word_node_object_finalize(GObject* object) {
	ScidaeWordNode* self = SCIDAE_WORD_NODE(object);

	g_ptr_array_unref(self->revisions);
	if (self->current)
		g_free(self->current);

	G_OBJECT_CLASS(scidae_word_node_parent_class)->finalize(object);
}

static void scidae_word_node_set_revisions(ScidaeWordNode* self, GPtrArray* revisions) {
	g_return_if_fail(SCIDAE_IS_WORD_NODE(self));

	if (self->revisions)
		g_ptr_array_unref(self->revisions);
	self->revisions = g_ptr_array_ref(revisions);
	g_ptr_array_set_free_func(self->revisions, g_object_unref);

	g_string_set_size(self->current, 0);
	GError* err = NULL;
	for (guint i = 0; i < self->revisions->len; i++) {
		scidae_word_node_apply_revision(self, g_ptr_array_index(self->revisions, i), &err);
		if (err) {
			g_critical("Failed to apply revision %p: %s", g_ptr_array_index(self->revisions, i), err->message);
			g_error_free(err);
			err = NULL;
		}
	}

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_REVISIONS]);
}

static void scidae_word_node_object_get_property(GObject* object, guint prop_id, GValue* val, GParamSpec* pspec) {
	ScidaeWordNode* self = SCIDAE_WORD_NODE(object);
	switch (prop_id) {
		case PROP_REVISIONS:
			g_value_set_boxed(val, scidae_word_node_get_revisions(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void scidae_word_node_object_set_property(GObject* object, guint prop_id, const GValue* val, GParamSpec* pspec) {
	ScidaeWordNode* self = SCIDAE_WORD_NODE(object);
	switch (prop_id) {
		case PROP_REVISIONS:
			scidae_word_node_set_revisions(self, g_value_get_boxed(val));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void scidae_word_node_class_init(ScidaeWordNodeClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);

	object_class->finalize = scidae_word_node_object_finalize;
	object_class->get_property = scidae_word_node_object_get_property;
	object_class->set_property = scidae_word_node_object_set_property;

	obj_properties[PROP_REVISIONS] = g_param_spec_boxed(
		"revisions",
		"Revisions",
		"Array of all Scidae.Revisions applied to this node",
		G_TYPE_PTR_ARRAY,
		G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
	);

	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void scidae_word_node_init(ScidaeWordNode* self) {
	self->current = g_string_new(NULL);
	self->revisions = g_ptr_array_new_with_free_func(g_object_unref);
	self->prev = NULL;
	self->next = NULL;
}

ScidaeWordNode* scidae_word_node_new(GPtrArray* revisions) {
	return g_object_new(SCIDAE_TYPE_WORD_NODE, "revisions", revisions, NULL);
}

const gchar* scidae_word_node_get_string(ScidaeWordNode* self) {
	g_return_val_if_fail(SCIDAE_IS_WORD_NODE(self), NULL);
	return self->current->str;
}

const GPtrArray* scidae_word_node_get_revisions(ScidaeWordNode* self) {
	g_return_val_if_fail(SCIDAE_IS_WORD_NODE(self), NULL);
	return self->revisions;
}

void scidae_word_node_apply_revision(ScidaeWordNode* self, ScidaeRevision* rev, GError** err) {
	g_return_if_fail(SCIDAE_IS_WORD_NODE(self));

	if (!scidae_revision_apply(rev, self->current, err))
		return;

	g_ptr_array_add(self->revisions, rev);
}