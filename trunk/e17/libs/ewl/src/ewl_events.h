#ifndef __EWL_EVENTS_H__
#define __EWL_EVENTS_H__

/**
 * @defgroup Ewl_Events Events: Lower Level Event Handlers
 * @brief Defines the routines that dispatch the lower level events to EWL.
 *
 * @{
 */

int             ewl_ev_init(void);

int ewl_ev_window_expose(void *data, int type, void *_ev);
int ewl_ev_window_configure(void *data, int type, void *_ev);
int ewl_ev_window_delete(void *data, int type, void *_ev);

int ewl_ev_key_down(void *data, int type, void *_ev);
int ewl_ev_key_up(void *data, int type, void *_ev);
int ewl_ev_mouse_down(void *data, int type, void *_ev);
int ewl_ev_mouse_up(void *data, int type, void *_ev);
int ewl_ev_mouse_move(void *data, int type, void *_ev);
int ewl_ev_mouse_out(void *data, int type, void *_ev);
int ewl_ev_paste(void *data, int type, void *_ev);

/**
 * @}
 */

#endif				/* __EWL_EVENTS_H__ */
