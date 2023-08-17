#include "scidaewordnode.h"

struct _ScidaeWordNode {
	GObject parent_instance;

	ScidaeDataId* identifier;

	GString* current;
	GPtrArray* revisions;

	ScidaeWordNode* prev;
	ScidaeWordNode* next;
};

G_DEFINE_TYPE (ScidaeWordNode, scidae_word_node, G_TYPE_OBJECT)

enum {
	PROP_IDENTIFIER = 1,
	PROP_REVISIONS,
	N_PROPERTIES
};
static GParamSpec* obj_properties[N_PROPERTIES] = { 0, };

static void scidae_word_node_object_finalize(GObject* object) {
	ScidaeWordNode* self = SCIDAE_WORD_NODE(object);

	g_ptr_array_unref(self->revisions);
	if (self->current)
		g_free(self->current);

	if (self->identifier)
		scidae_data_id_free(self->identifier);

	G_OBJECT_CLASS(scidae_word_node_parent_class)->finalize(object);
}

static void scidae_word_node_object_constructed(GObject* object) {
	ScidaeWordNode* self = SCIDAE_WORD_NODE(object);
	if (!self->identifier) {
		self->identifier = scidae_data_id_new();
		g_info("Generated identifier \"%lu-%016lx\" for id-less word node %p.", self->identifier->timestamp, self->identifier->randomness, (void*)self);
	}
	if (!self->revisions)
		self->revisions = g_ptr_array_new_with_free_func(g_object_unref);

	G_OBJECT_CLASS(scidae_word_node_parent_class)->constructed(object);
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
		ScidaeRevision* rev = g_ptr_array_index(self->revisions, i);
		if (!scidae_revision_apply(rev, self->current, &err)) {
			g_critical("Failed to apply revision %p: %s", (void*)rev, err->message);
			g_error_free(err);
			err = NULL;
		}
	}

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_REVISIONS]);
}

static void scidae_word_node_object_get_property(GObject* object, guint prop_id, GValue* val, GParamSpec* pspec) {
	ScidaeWordNode* self = SCIDAE_WORD_NODE(object);
	switch (prop_id) {
		case PROP_IDENTIFIER:
			g_value_set_boxed(val, scidae_word_node_get_identifier(self));
			break;
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
		case PROP_IDENTIFIER:
			scidae_word_node_set_identifier(self, g_value_get_boxed(val));
			break;
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
	object_class->constructed = scidae_word_node_object_constructed;
	object_class->get_property = scidae_word_node_object_get_property;
	object_class->set_property = scidae_word_node_object_set_property;

	obj_properties[PROP_IDENTIFIER] = g_param_spec_boxed(
		"identifier",
		"Unique Identifier",
		"The unique id associated with this word node",
		SCIDAE_TYPE_DATA_ID,
		G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_CONSTRUCT_ONLY
	);

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
	self->identifier = NULL;
	self->current = g_string_new(NULL);
	self->revisions = NULL;
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

ScidaeDataId* scidae_word_node_get_identifier(ScidaeWordNode* self) {
	g_return_val_if_fail(SCIDAE_IS_WORD_NODE(self), NULL);
	return self->identifier;
}

void scidae_word_node_set_identifier(ScidaeWordNode* self, ScidaeDataId* identifier) {
	g_return_if_fail(SCIDAE_IS_WORD_NODE(self));

	if (self->identifier == identifier)
		return;

	if (self->identifier)
		scidae_data_id_free(self->identifier);
	self->identifier = scidae_data_id_copy(identifier);

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_IDENTIFIER]);
}

const GPtrArray* scidae_word_node_get_revisions(ScidaeWordNode* self) {
	g_return_val_if_fail(SCIDAE_IS_WORD_NODE(self), NULL);
	return self->revisions;
}

ScidaeWordNode* scidae_word_node_get_prev(ScidaeWordNode* self) {
	g_return_val_if_fail(SCIDAE_IS_WORD_NODE(self), NULL);
	return self->prev;
}

ScidaeWordNode* scidae_word_node_set_prev(ScidaeWordNode* self, ScidaeWordNode* new) {
	g_return_val_if_fail(SCIDAE_IS_WORD_NODE(self), NULL);

	ScidaeWordNode* old = self->prev;
	self->prev = new;
	return old;
}

ScidaeWordNode* scidae_word_node_get_next(ScidaeWordNode* self) {
	g_return_val_if_fail(SCIDAE_IS_WORD_NODE(self), NULL);
	return self->prev;
}

ScidaeWordNode* scidae_word_node_set_next(ScidaeWordNode* self, ScidaeWordNode* new) {
	g_return_val_if_fail(SCIDAE_IS_WORD_NODE(self), NULL);

	ScidaeWordNode* old = self->next;
	self->next = new;
	return old;
}

void scidae_word_node_apply_revision(ScidaeWordNode* self, ScidaeRevision* rev, GError** err) {
	g_return_if_fail(SCIDAE_IS_WORD_NODE(self));

	if (!scidae_revision_apply(rev, self->current, err))
		return;

	g_ptr_array_add(self->revisions, rev);
}
