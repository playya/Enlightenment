cdef extern from "Ecore_X.h":
    int ecore_x_screensaver_event_available_get()
    void ecore_x_screensaver_idle_time_prefetch()
    void ecore_x_screensaver_idle_time_fetch()
    int ecore_x_screensaver_idle_time_get()
    void ecore_x_get_screensaver_prefetch()
    void ecore_x_get_screensaver_fetch()
    void ecore_x_screensaver_set(int timeout, int interval, int blank, int expose)
    void ecore_x_screensaver_timeout_set(int timeout)
    int ecore_x_screensaver_timeout_get()
    void ecore_x_screensaver_blank_set(int timeout)
    int ecore_x_screensaver_blank_get()
    void ecore_x_screensaver_expose_set(int timeout)
    int ecore_x_screensaver_expose_get()
    void ecore_x_screensaver_interval_set(int timeout)
    int ecore_x_screensaver_interval_get()
    void ecore_x_screensaver_event_listen_set(int on)
