# This file is included verbatim by c_ecore.pyx

import traceback

cdef int animator_cb(void *_td) with GIL:
    cdef Animator obj
    cdef int r

    obj = <Animator>_td

    try:
        r = bool(obj._exec())
    except Exception, e:
        traceback.print_exc()
        r = 0

    if not r:
        obj.delete()
    return r


cdef class Animator:
    """Creates an animator to tick off at every animaton tick during main loop
       execution.

       This class represents an animator that will call the given B{func}
       every N seconds where N is the frametime interval set by
       L{animator_frametime_set()}. The function will be passed any extra
       parameters given to constructor.

       When the animator B{func} is called, it must return a value of either
       True or False (remember that Python returns None if no value is
       explicitly returned and None evaluates to False). If it returns
       B{True}, it will be called again at the next tick, or if it returns
       B{False} it will be deleted automatically making any references/handles
       for it invalid.

       Animators should be stopped/deleted by means of L{delete()} or
       returning False from B{func}, otherwise they'll continue alive, even
       if the current python context delete it's reference to it.
    """
    def __init__(self, func, *args, **kargs):
        if not callable(func):
            raise TypeError("Parameter 'func' must be callable")
        self.func = func
        self.args = args
        self.kargs = kargs
        if self.obj == NULL:
            self.obj = ecore_animator_add(animator_cb, <void *>self)
            if self.obj != NULL:
                python.Py_INCREF(self)

    def __str__(self):
        return "%s(func=%s, args=%s, kargs=%s)" % \
               (self.__class__.__name__, self.func, self.args, self.kargs)

    def __repr__(self):
        return ("%s(0x%x, func=%s, args=%s, kargs=%s, Ecore_Animator=0x%x, "
                "refcount=%d)") % \
               (self.__class__.__name__, <unsigned long>self,
                self.func, self.args, self.kargs,
                <unsigned long>self.obj, PY_REFCOUNT(self))

    def __dealloc__(self):
        if self.obj != NULL:
            ecore_animator_del(self.obj)
            self.obj = NULL
        self.func = None
        self.args = None
        self.kargs = None

    cdef _exec(self):
        return self.func(*self.args, **self.kargs)

    def delete(self):
        "Stop callback emission and free internal resources."
        if self.obj != NULL:
            ecore_animator_del(self.obj)
            self.obj = NULL
            python.Py_DECREF(self)

    def stop(self):
        "Alias for L{delete()}."
        self.delete()


def animator_add(func, *args, **kargs):
    """L{Animator} factory, for C-api compatibility.

       @rtype: L{Animator}
    """
    return Animator(func, *args, **kargs)
