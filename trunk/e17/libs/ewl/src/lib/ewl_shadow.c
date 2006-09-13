#include <Ewl.h>
#include "ewl_debug.h"
#include "ewl_macros.h"
#include "ewl_private.h"

/**
 * @return Returns a new shadow container on success, NULL on failure.
 * @brief Allocate and initialize a new shadow container
 */

Ewl_Widget *
ewl_shadow_new(void)
{
	Ewl_Shadow *s;

	DENTER_FUNCTION(DLEVEL_STABLE);

	s = NEW(Ewl_Shadow, 1);
	if (!s) {
		DRETURN_PTR(NULL, DLEVEL_STABLE);
	}

	ewl_shadow_init(s);

	DRETURN_PTR(EWL_WIDGET(s), DLEVEL_STABLE);
}

/**
 * @param s: the shadow container to initialize
 * @return Returns TRUE on success, FALSE on failure.
 * @brief Initialize a shadow container to default values
 */
int
ewl_shadow_init(Ewl_Shadow * s)
{
	Ewl_Widget *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("s", s, FALSE);

	w = EWL_WIDGET(s);

	if (!ewl_box_init(EWL_BOX(w))) {
		DRETURN_INT(FALSE, DLEVEL_STABLE);
	}
	ewl_box_orientation_set(EWL_BOX(w), EWL_ORIENTATION_VERTICAL);
	ewl_object_fill_policy_set(EWL_OBJECT(w), EWL_FLAG_FILL_SHRINK);

	ewl_widget_appearance_set(EWL_WIDGET(s), EWL_SHADOW_TYPE);
	ewl_widget_inherit(EWL_WIDGET(s), EWL_SHADOW_TYPE);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}
