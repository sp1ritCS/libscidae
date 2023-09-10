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
	ScidaeWidget*(*get_prev)(ScidaeContainer* self, ScidaeWidget* widget);
	ScidaeWidget*(*get_next)(ScidaeContainer* self, ScidaeWidget* widget);
	void(*unparent)(ScidaeContainer* self, ScidaeWidget* child);

	void(*update_cursor)(ScidaeContainer* self, ScidaeWidget* cursor_holder);
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
 * scidae_container_get_prev:
 * @self: a `ScidaeContainer`
 * @widget: the `ScidaeWidget`
 *
 * Get the widget before `widget`.
 * Returns: (transfer none): the widget behind
 */
ScidaeWidget* scidae_container_get_prev(ScidaeContainer* self, ScidaeWidget* widget);

/**
 * scidae_container_get_next:
 * @self: a `ScidaeContainer`
 * @widget: the `ScidaeWidget`
 *
 * Get the widget in front of `widget`.
 * Returns: (transfer none): the widget infront
 */
ScidaeWidget* scidae_container_get_next(ScidaeContainer* self, ScidaeWidget* widget);

/**
 * scidae_container_unparent:
 * @self: a `ScidaeContainer`
 * @child: the widget that removed this parent
 * 
 * Called when a child removes this parent.
 */
void scidae_container_unparent(ScidaeContainer* self, ScidaeWidget* child);

/**
 * scidae_container_update_cursor:
 * @self: the container
 * @cursor_holder: the widgets that now holds the cursor
 *
 * Update the cursor holder thats tracked by this container.
 */
void scidae_container_update_cursor(ScidaeContainer* self, ScidaeWidget* cursor_holder);

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
