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
 * scidae_word_node_get_prev:
 * @self: the word node
 *
 * Get the node in front of this word node.
 * Returns: (transfer none): the previous node
 */
ScidaeWordNode* scidae_word_node_get_prev(ScidaeWordNode* self);

/**
 * scidae_word_node_set_prev:
 * @self: the word node
 * @new_prev: (transfer full): the new previous node
 *
 * Replace the previous word node with a new one.
 * Returns: (transfer full): the old previous node
 */
ScidaeWordNode* scidae_word_node_set_prev(ScidaeWordNode* self, ScidaeWordNode* new_prev);

/**
 * scidae_word_node_get_next:
 * @self: the word node
 *
 * Get the node after of this word node.
 * Returns: (transfer none): the next node
 */
ScidaeWordNode* scidae_word_node_get_next(ScidaeWordNode* self);

/**
 * scidae_word_node_set_next:
 * @self: the word node
 * @new_next: (transfer full): the new next node
 *
 * Replace the next word node with a new one.
 * Returns: (transfer full): the old next node
 */
ScidaeWordNode* scidae_word_node_set_next(ScidaeWordNode* self, ScidaeWordNode* new_next);

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
