#ifndef __SCIDAEWORDNODE_H__
#define __SCIDAEWORDNODE_H__

#include <glib-object.h>
#include <scidaerevision.h>

G_BEGIN_DECLS

/**
 * ScidaeWordNode:
 *
 * Word nodes make out the main part of the scidae data structure. Each word has
 * essentially a piece table, here named "revisions", that store the make-up of
 * this particular word. It can be altered by appending new revisions, that have
 * the ability to modify the current contents.
 */

#define SCIDAE_TYPE_WORD_NODE (scidae_word_node_get_type())
G_DECLARE_FINAL_TYPE (ScidaeWordNode, scidae_word_node, SCIDAE, WORD_NODE, GObject)

/**
 * scidae_word_node_new:
 * @revisions: (element-type ScidaeData.Revision): array of revisions for this node
 *
 * Create a new word node from revisions
 * Returns: (transfer full): new word node
 */
ScidaeWordNode* scidae_word_node_new(GPtrArray* revisions);

/**
 * scidae_word_node_get_string:
 * @self: the word node
 *
 * Get the current string this word node represents
 * Returns: (transfer none): the current string
 */
const gchar* scidae_word_node_get_string(ScidaeWordNode* self);

/**
 * scidae_word_node_get_revisions:
 * @self: the word node
 *
 * Get the array of all applied revisions.
 * Returns: (transfer none) (element-type ScidaeData.Revision): all applied revisions
 */
const GPtrArray* scidae_word_node_get_revisions(ScidaeWordNode* self);

/**
 * scidae_word_node_apply_revision:
 * @self: the word node
 * @rev: the revision to apply
 * @err: pointer to a GError* initialized as %NULL
 *
 * Apply a new revision to the word node.
 */
void scidae_word_node_apply_revision(ScidaeWordNode* self, ScidaeRevision* rev, GError** err);

G_END_DECLS

#endif // __SCIDAEWORDNODE_H__
