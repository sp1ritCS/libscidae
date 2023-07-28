#ifndef __SCIDAECONTAINER_H__
#define __SCIDAECONTAINER_H__

#include <glib-object.h>
#include <widget/scidaewidget.h>

G_BEGIN_DECLS

/**
 * ScidaeContainer:
 * 
 * This is the interface implemented by every [class@Sciade.Widget] capable of
 * becoming the parent to one or more other [class@Sciade.Widget]s. 
 */
#define SCIDAE_TYPE_CONTAINER (scidae_container_get_type())
G_DECLARE_INTERFACE (ScidaeContainer, scidae_container, SCIDAE, CONTAINER, ScidaeWidget)

struct _ScidaeContainerInterface {
	GTypeInterface parent_iface;
	
	GList*(*get_children)(ScidaeContainer* self);
	void(*unparent)(ScidaeContainer* self, ScidaeWidget* child);

	void(*mark_child_remeasure)(ScidaeContainer* self, ScidaeWidget* child);
};

/**
 * scidae_container_get_children:
 * @self: a `ScidaeContainer`
 *
 * Get all children of this container as a list.
 * Returns: (transfer container) (element-type ScidaeWidget): a GList containing [class@Scidae.Widget]s
 */
GList* scidae_container_get_children(ScidaeContainer* self);

/**
 * scidae_container_unparent:
 * @self: a `ScidaeContainer`
 * @child: the widget that removed this parent
 * 
 * Called when a child removes this parent.
 */
void scidae_container_unparent(ScidaeContainer* self, ScidaeWidget* child);

/**
 * scidae_container_mark_child_remeasure:
 * @self: the container
 * @child: the child requesting remeasuring
 *
 * Request from a child that it wants to be remeasured.
 */
void scidae_container_mark_child_remeasure(ScidaeContainer* self, ScidaeWidget* child);

G_END_DECLS

#endif // __SCIDAEICONTAINER_H__
