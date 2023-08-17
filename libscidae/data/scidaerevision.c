#include "scidaerevision.h"

typedef struct {
	ScidaeDataId* identifier;
} ScidaeRevisionPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (ScidaeRevision, scidae_revision, G_TYPE_OBJECT)

enum {
	PROP_IDENTIFIER = 1,
	N_PROPERTIES
};
static GParamSpec* obj_properties[N_PROPERTIES] = { 0, };

static void scidae_revision_object_finalize(GObject* object) {
	ScidaeRevisionPrivate* priv = scidae_revision_get_instance_private(SCIDAE_REVISION(object));
	if (priv->identifier)
		scidae_data_id_free(priv->identifier);

	G_OBJECT_CLASS(scidae_revision_parent_class)->finalize(object);
}

static void scidae_revision_object_constructed(GObject* object) {
	ScidaeRevisionPrivate* priv = scidae_revision_get_instance_private(SCIDAE_REVISION(object));
	if (!priv->identifier) {
		priv->identifier = scidae_data_id_new();
		g_info("Generated identifier \"%lu-%016lx\" for id-less revision %p.", priv->identifier->timestamp, priv->identifier->randomness, (void*)object);
	}

	G_OBJECT_CLASS(scidae_revision_parent_class)->constructed(object);
}

static void scidae_revision_object_get_property(GObject* object, guint prop_id, GValue* val, GParamSpec* pspec) {
	ScidaeRevision* self = SCIDAE_REVISION(object);
	switch (prop_id) {
		case PROP_IDENTIFIER:
			g_value_set_boxed(val, scidae_revision_get_identifier(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void scidae_revision_object_set_property(GObject* object, guint prop_id, const GValue* val, GParamSpec* pspec) {
	ScidaeRevision* self = SCIDAE_REVISION(object);
	switch (prop_id) {
		case PROP_IDENTIFIER:
			scidae_revision_set_identifier(self, g_value_get_boxed(val));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void scidae_revision_class_init(ScidaeRevisionClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);

	class->apply = NULL;

	object_class->finalize = scidae_revision_object_finalize;
	object_class->constructed = scidae_revision_object_constructed;
	object_class->get_property = scidae_revision_object_get_property;
	object_class->set_property = scidae_revision_object_set_property;

	obj_properties[PROP_IDENTIFIER] = g_param_spec_boxed(
		"identifier",
		"Unique Identifier",
		"The unique id associated with this revision",
		SCIDAE_TYPE_DATA_ID,
		G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_CONSTRUCT_ONLY
	);

	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void scidae_revision_init(ScidaeRevision* self) {
	ScidaeRevisionPrivate* priv = scidae_revision_get_instance_private(self);
	priv->identifier = NULL;
}

ScidaeDataId* scidae_revision_get_identifier(ScidaeRevision* self) {
	g_return_val_if_fail(SCIDAE_IS_REVISION(self), NULL);
	ScidaeRevisionPrivate* priv = scidae_revision_get_instance_private(self);
	return priv->identifier;
}

void scidae_revision_set_identifier(ScidaeRevision* self, ScidaeDataId* identifier) {
	g_return_if_fail(SCIDAE_IS_REVISION(self));
	ScidaeRevisionPrivate* priv = scidae_revision_get_instance_private(self);

	if (priv->identifier == identifier)
		return;

	if (priv->identifier)
		scidae_data_id_free(priv->identifier);
	priv->identifier = scidae_data_id_copy(identifier);

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_IDENTIFIER]);
}

gboolean scidae_revision_apply(ScidaeRevision* self, GString* target, GError** err) {
	g_return_val_if_fail(SCIDAE_IS_REVISION(self), FALSE);
	ScidaeRevisionClass* class = SCIDAE_REVISION_GET_CLASS(self);
	g_return_val_if_fail(class->apply, FALSE);
	return class->apply(self, target, err);
}
