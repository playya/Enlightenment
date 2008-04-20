#ifndef __EPDF_PAGE_TRANSITION_H__
#define __EPDF_PAGE_TRANSITION_H__


#include "epdf_enum.h"
#include "epdf_forward.h"


/**
 * @file epdf_page_transition.h
 * @defgroup Epdf_Page_Transition Epdf Page Transitions
 * @brief Functions that manage PDF page transitions
 * @ingroup Epdf
 *
 * Functions that manage PDF page transitions
 */

/**
 * Delete an Epdf_Page_Transition object
 * @param transition The page transition to delete
 *
 * Delete an Epdf_Page_Transition object @p transition
 *
 * @ingroup Epdf_Page_Transition
 */
void epdf_page_transition_delete (Epdf_Page_Transition *transition);

/**
 * Return the type of a transition
 *
 * @param transition The page transition
 * @return The type of the transition
 *
 * Return the type of the transition @p transition, as an
 * Epdf_Page_Transition_Type
 *
 * @ingroup Epdf_Page_Transition
 */
Epdf_Page_Transition_Type epdf_page_transition_type_get (Epdf_Page_Transition *transition);

/**
 * Return the duration of a transition
 *
 * @param transition The page transition
 * @return The duration of the transition
 *
 * Return the duration of the transition @p transition
 *
 * @ingroup Epdf_Page_Transition
 */
int epdf_page_transition_duration_get (Epdf_Page_Transition *transition);

/**
 * Return the alignment of a transition
 *
 * @param transition The page transition
 * @return The alignment of the transition
 *
 * Return the alignment of the transition @p transition
 *
 * @ingroup Epdf_Page_Transition
 */
Epdf_Page_Transition_Alignment epdf_page_transition_alignment_get (Epdf_Page_Transition *transition);

/**
 * Return the direction of a transition
 *
 * @param transition The page transition
 * @return The direction of a transition
 *
 * Return the direction of the transition @p transition
 *
 * @ingroup Epdf_Page_Transition
 */
Epdf_Page_Transition_Direction epdf_page_transition_direction_get (Epdf_Page_Transition *transition);

/**
 * Return the angle of a transition
 *
 * @param transition The page transition
 * @return The angle of a transition
 *
 * Return the angle of the transition @p transition
 *
 * @ingroup Epdf_Page_Transition
 */
int epdf_page_transition_angle_get (Epdf_Page_Transition *transition);

/**
 * Return the scale of a transition
 *
 * @param transition The page transition
 * @return The scale of a transition
 *
 * Return the scale of the transition @p transition
 *
 * @ingroup Epdf_Page_Transition
 */
double epdf_page_transition_scale_get (Epdf_Page_Transition *transition);

/**
 * Whether the transition is rectangular or not
 *
 * @param transition The page transition
 * @return 1 if the transition is rectangular, 0 otherwise
 *
 * Whether the transition is rectangular or not
 *
 * @ingroup Epdf_Page_Transition
 */
unsigned char epdf_page_transition_is_rectangular_get (Epdf_Page_Transition *transition);


#endif /* __EPDF_PAGE_TRANSITION_H__ */
