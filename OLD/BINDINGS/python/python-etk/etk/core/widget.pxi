# Copyright (C) 2007-2008 Caio Marcelo de Oliveira Filho, Gustavo Sverzut Barbieri
#
# This file is part of Python-Etk.
#
# Python-Etk is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# Python-Etk is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this Python-Etk.  If not, see <http://www.gnu.org/licenses/>.

import evas.c_evas

# Virtual functions

cdef void _virtual_size_request(Etk_Widget *widget, Etk_Size *size_requisition) with gil:
    self = Object_from_instance(<Etk_Object *>widget)
    (w, h) = self._size_request()
    size_requisition.w = w
    size_requisition.h = h

cdef void _virtual_size_allocate(Etk_Widget *widget, Etk_Geometry geometry) with gil:
    self = Object_from_instance(<Etk_Object *>widget)
    self._size_allocate(geometry.x, geometry.y, geometry.w, geometry.h)

cdef void _virtual_theme_signal_emit(Etk_Widget *widget, char *signal, Etk_Bool size_recalc) with gil:
    self = Object_from_instance(<Etk_Object *>widget)
    self._theme_signal_emit(signal, size_recalc)

cdef void _virtual_scroll_size_get(Etk_Widget *widget, Etk_Size scrollview_size, Etk_Size scrollbar_size, Etk_Size *scroll_size) with gil:
    self = Object_from_instance(<Etk_Object *>widget)
    (w, h) = self._scroll_size_get(scrollview_size.w, scrollview_size.h, scrollbar_size.w, scrollbar_size.h)
    scroll_size.w = w
    scroll_size.h = h

cdef void _virtual_scroll(Etk_Widget *widget, int x, int y) with gil:
    self = Object_from_instance(<Etk_Object *>widget)
    self._scroll(x, y)

cdef public class Widget(Object) [object PyEtk_Widget, type PyEtk_Widget_Type]:
    def __init__(self, **kargs):
        if self.obj == NULL:
            self._set_obj(<Etk_Object*>etk_widget_new(etk_widget_type_get(),
                                                      NULL))
        self._set_common_params(**kargs)

    cdef object _set_obj(self, Etk_Object *obj):
        cdef Etk_Widget *w
        Object._set_obj(self, obj)
        w = <Etk_Widget*>obj

        if getattr3(self.__class__, "_size_request", None) is not None:
            w.size_request = _virtual_size_request

        if getattr3(self.__class__, "_size_allocate", None) is not None:
            w.size_allocate = _virtual_size_allocate

        if getattr3(self.__class__, "_theme_signal_emit", None) is not None:
            w.theme_signal_emit = _virtual_theme_signal_emit

        if getattr3(self.__class__, "_scroll_size_get", None) is not None:
            w.scroll_size_get = _virtual_scroll_size_get

        if getattr3(self.__class__, "_scroll", None) is not None:
            w.scroll = _virtual_scroll

        return self

    def _set_common_params(self, size_request=None, **kargs):
        if size_request is not None:
            self.size_request_set(*size_request)

        if kargs:
            Object._set_common_params(self, **kargs)

    def clip_get(self):
        cdef Etk_Widget *wid
        cdef Evas_Object *eobj
        wid = <Etk_Widget*>self.obj
        eobj = etk_widget_clip_get(<Etk_Widget*>self.obj)
        return evas.c_evas._Object_from_instance(<long>eobj)

    def clip_set(self, clip):
        cdef evas.c_evas.Evas_Object *eobj
        cdef evas.c_evas.Object o
        if clip is None:
            eobj = NULL
        elif isinstance(clip, evas.c_evas.Object):
            o = clip
            eobj = o.obj
        else:
            raise ValueError("clip must be evas.c_evas.Object or None")
        etk_widget_clip_set(<Etk_Widget*>self.obj, eobj)

    def clip_unset(self):
        etk_widget_clip_unset(<Etk_Widget*>self.obj)

    property clip:
        def __get__(self):
            return self.clip_get()

        def __set__(self, value):
            self.clip_set(value)

        def __del__(self):
            self.clip_unset()

    def color_get(self):
        cdef int r, g, b, a
        etk_widget_color_get(<Etk_Widget*>self.obj, &r, &g, &b, &a)
        return (r, g, b, a)

    def color_set(self, int r, int g, int b, int a):
        etk_widget_color_set(<Etk_Widget*>self.obj, r, g, b, a)

    property color:
        def __get__(self):
            return self.color_get()

        def __set__(self, spec):
            self.color_set(*spec)

    def disabled_get(self):
        return bool(<int>etk_widget_disabled_get(<Etk_Widget*>self.obj))

    def disabled_set(self, int disabled):
        etk_widget_disabled_set(<Etk_Widget*>self.obj, <Etk_Bool>disabled)

    property disabled:
        def __get__(self):
            return bool(self.disabled_get())

        def __set__(self, value):
            self.disabled_set(value)


    def disabled_set_all(self, int disabled):
        etk_widget_disabled_set_all(<Etk_Widget*>self.obj, <Etk_Bool>disabled)

    def dnd_dest_get(self):
        return bool(<int>etk_widget_dnd_dest_get(<Etk_Widget*>self.obj))

    def dnd_dest_set(self, int on):
        etk_widget_dnd_dest_set(<Etk_Widget*>self.obj, <Etk_Bool>on)

    property dnd_dest:
        def __get__(self):
            return self.dnd_dest_get()

        def __set__(self, value):
            self.dnd_dest_set(value)

##     def dnd_drag_data_set(self, char** types, int num_types, void* data, int data_size):
##         # FIXME: unsupported method arguments
##         pass

    def dnd_drag_widget_get(self):
        cdef Etk_Object *o
        o = <Etk_Object*>etk_widget_dnd_drag_widget_get(<Etk_Widget*>self.obj)
        return Object_from_instance(o)

    def dnd_drag_widget_set(self, Widget drag_widget):
        etk_widget_dnd_drag_widget_set(<Etk_Widget*>self.obj,
                                       <Etk_Widget*>drag_widget.obj)

    property dnd_drag_widget:
        def __get__(self):
            return self.dnd_drag_widget_get()

        def __set__(self, value):
            self.dnd_drag_widget_set(value)

    def dnd_files_get(self):
        cdef int num_files, i
        cdef char **cfiles

## XXX: not present in libetk.so
        return tuple()
##         cfiles = etk_widget_dnd_files_get(<Etk_Widget*>self.obj, &num_files)
##         files = []
##         for i from 0 <= i < num_files:
##             files.append(cfiles[i])
##         return files

    property dnd_files:
        def __get__(self):
            return self.dnd_files_get()

    def dnd_internal_get(self):
        return bool(<int>etk_widget_dnd_internal_get(<Etk_Widget*>self.obj))

    def dnd_internal_set(self, int on):
        etk_widget_dnd_internal_set(<Etk_Widget*>self.obj, <Etk_Bool>on)

    property dnd_internal:
        def __get__(self):
            return self.dnd_internal_get()

        def __set__(self, value):
            self.dnd_internal_set(value)

    def dnd_source_get(self):
        return bool(<int>etk_widget_dnd_source_get(<Etk_Widget*>self.obj))

    def dnd_source_set(self, int on):
        etk_widget_dnd_source_set(<Etk_Widget*>self.obj, <Etk_Bool>on)

    property dnd_source:
        def __get__(self):
            return self.dnd_source_get()

        def __set__(self, value):
            self.dnd_source_set(value)

    def dnd_types_get(self):
        cdef int num_types, i
        cdef char **ctypes

        ctypes = etk_widget_dnd_types_get(<Etk_Widget*>self.obj, &num_types)
        types = []
        for i from 0 <= i < num_types:
            types.append(ctypes[i])
        return types

    def dnd_types_set(self, types):
        cdef char** ctypes
        cdef int num_types, i

        num_types = len(types)
        if num_types < 1:
            ctypes = NULL
        else:
            ctypes = <char**>python.PyMem_Malloc(num_types * sizeof(char*))
            for i in 0 <= i < num_types:
                ctypes[i] = types[i]
        etk_widget_dnd_types_set(<Etk_Widget*>self.obj, ctypes, num_types)
        if ctypes != NULL:
            python.PyMem_Free(ctypes)

    property dnd_types:
        def __get__(self):
            return self.dnd_types_get()

        def __set__(self, value):
            self.dnd_types_set(value)

    def enter(self):
        etk_widget_enter(<Etk_Widget*>self.obj)

    def focus(self):
        etk_widget_focus(<Etk_Widget*>self.obj)

    def focusable_get(self):
        return bool(<int>etk_widget_focusable_get(<Etk_Widget*>self.obj))

    def focusable_set(self, int focusable):
        etk_widget_focusable_set(<Etk_Widget*>self.obj, <Etk_Bool>focusable)

    property focusable:
        def __get__(self):
            return self.focusable_get()

        def __set__(self, value):
            self.focusable_set(value)

    def geometry_get(self):
        cdef int x, y, w, h
        etk_widget_geometry_get(<Etk_Widget*>self.obj, &x, &y, &w, &h)
        return (x, y, w, h)

    property geometry:
        def __get__(self):
            return self.geometry_get()

    def has_event_object_get(self):
        return bool(<int>etk_widget_has_event_object_get(<Etk_Widget*>self.obj))

    def has_event_object_set(self, int has_event_object):
        etk_widget_has_event_object_set(<Etk_Widget*>self.obj,
                                        <Etk_Bool>has_event_object)

    property has_event_object:
        def __get__(self):
            return self.has_event_object_get()

        def __set__(self, value):
            self.has_event_object_set(value)

    def hide(self):
        etk_widget_hide(<Etk_Widget*>self.obj)

    def hide_all(self):
        etk_widget_hide_all(<Etk_Widget*>self.obj)

    def inner_geometry_get(self):
        cdef int x, y, w, h
        etk_widget_inner_geometry_get(<Etk_Widget*>self.obj, &x, &y, &w, &h)
        return (x, y, w, h)

    property inner_geometry:
        def __get__(self):
            return self.inner_geometry_get()

    def internal_get(self):
        return bool(<int>etk_widget_internal_get(<Etk_Widget*>self.obj))

    def internal_set(self, int internal):
        etk_widget_internal_set(<Etk_Widget*>self.obj, <Etk_Bool>internal)

    property internal:
        def __get__(self):
            return self.internal_get()

        def __set__(self, value):
            self.internal_set(value)

    def is_focused(self):
        return bool(<int>etk_widget_is_focused(<Etk_Widget*>self.obj))

    def is_swallowed(self):
        return bool(<int>etk_widget_is_swallowed(<Etk_Widget*>self.obj))

    def is_visible(self):
        return bool(<int>etk_widget_is_visible(<Etk_Widget*>self.obj))

    def leave(self):
        etk_widget_leave(<Etk_Widget*>self.obj)

    def lower(self):
        etk_widget_lower(<Etk_Widget*>self.obj)

    def member_object_add(self, evas.c_evas.Object eobj):
        return etk_widget_member_object_add(<Etk_Widget*>self.obj, eobj.obj)

    def member_object_del(self, evas.c_evas.Object eobj):
        etk_widget_member_object_del(<Etk_Widget*>self.obj, eobj.obj)

    def member_object_lower(self, evas.c_evas.Object eobj):
        etk_widget_member_object_lower(<Etk_Widget*>self.obj, eobj.obj)

    def member_object_raise(self, evas.c_evas.Object eobj):
        etk_widget_member_object_raise(<Etk_Widget*>self.obj, eobj.obj)

    def member_object_stack_above(self, evas.c_evas.Object eobj,
                                  evas.c_evas.Object eobj_above):
        etk_widget_member_object_stack_above(<Etk_Widget*>self.obj, eobj.obj,
                                             eobj_above.obj)

    def member_object_stack_below(self, evas.c_evas.Object eobj,
                                  evas.c_evas.Object eobj_below):
        etk_widget_member_object_stack_below(<Etk_Widget*>self.obj, eobj.obj,
                                             eobj_below.obj)

    def padding_get(self):
        cdef int left, right, top, bottom
        etk_widget_padding_get(<Etk_Widget*>self.obj, &left, &right,
                               &top, &bottom)
        return (left, right, top, bottom)

    def padding_set(self, int left, int right, int top, int bottom):
        etk_widget_padding_set(<Etk_Widget*>self.obj, left, right,
                               top, bottom)

    property padding:
        def __get__(self):
            return self.padding_get()

        def __set__(self, spec):
            self.padding_set(*spec)

    def parent_get(self):
        cdef Etk_Widget *wid
        wid = etk_widget_parent_get(<Etk_Widget*>self.obj)
        return Object_from_instance(<Etk_Object*>wid)

    def parent_set(self, Widget parent):
        cdef Etk_Widget *p
        if parent is None:
            p = NULL
        else:
            p = <Etk_Widget*>parent.obj
        etk_widget_parent_set(<Etk_Widget*>self.obj, p)

    property parent:
        def __get__(self):
            return self.parent_get()

        def __set__(self, value):
            self.parent_set(value)

    def pass_mouse_events_get(self):
        return bool(<int>etk_widget_pass_mouse_events_get(<Etk_Widget*>self.obj))

    def pass_mouse_events_set(self, int pass_mouse_events):
        etk_widget_pass_mouse_events_set(<Etk_Widget*>self.obj,
                                         <Etk_Bool>pass_mouse_events)

    property pass_mouse_events:
        def __get__(self):
            return self.pass_mouse_events_get()

        def __set__(self, value):
            self.pass_mouse_events_set(value)

    def propagate_color_get(self):
        return bool(<int>etk_widget_propagate_color_get(<Etk_Widget*>self.obj))

    def propagate_color_set(self, int propagate_color):
        etk_widget_propagate_color_set(<Etk_Widget*>self.obj,
                                       <Etk_Bool>propagate_color)

    property propagate_color:
        def __get__(self):
            return self.propagate_color_get()

        def __set__(self, value):
            self.propagate_color_set(value)

    def raise_(self):
        etk_widget_raise(<Etk_Widget*>self.obj)

    def redraw_queue(self):
        etk_widget_redraw_queue(<Etk_Widget*>self.obj)

    def repeat_mouse_events_get(self):
        return bool(<int>etk_widget_repeat_mouse_events_get(<Etk_Widget*>self.obj))

    def repeat_mouse_events_set(self, int repeat_mouse_events):
        etk_widget_repeat_mouse_events_set(<Etk_Widget*>self.obj,
                                           <Etk_Bool>repeat_mouse_events)

    property repeat_mouse_events:
        def __get__(self):
            return self.repeat_mouse_events_get()

        def __set__(self, value):
            self.repeat_mouse_events_set(value)

    def show(self):
        etk_widget_show(<Etk_Widget*>self.obj)

    def show_all(self):
        etk_widget_show_all(<Etk_Widget*>self.obj)

    def size_allocate(self, x, y, w, h):
        cdef Etk_Geometry g
        g.x = x
        g.y = y
        g.w = w
        g.h = h
        etk_widget_size_allocate(<Etk_Widget*>self.obj, g)

    def size_recalc_queue(self):
        etk_widget_size_recalc_queue(<Etk_Widget*>self.obj)

    def size_request(self, int hidden_has_no_size=True):
        cdef Etk_Size s
        etk_widget_size_request_full(<Etk_Widget*>self.obj, &s,
                                     <int>hidden_has_no_size)
        return (s.w, s.h)

    def size_request_set(self, int w, int h):
        etk_widget_size_request_set(<Etk_Widget*>self.obj, w, h)

    def swallow_object(self, char* part, evas.c_evas.Object eobj):
        return bool(<int>etk_widget_swallow_object(<Etk_Widget*>self.obj,
                                                   part, eobj.obj))

    def swallow_widget(self, char* part, Widget to_swallow):
        return bool(<int>etk_widget_swallow_widget(
            <Etk_Widget*>self.obj, part, <Etk_Widget*>to_swallow.obj))

##     def theme_data_get(self, char* data_name, char* format):
##         __ret = etk_widget_theme_data_get(<Etk_Widget*>self.obj, data_name, format)
##         return (__ret)

    def theme_file_get(self):
        cdef char *__char_ret
        __ret = None
        __char_ret = etk_widget_theme_file_get(<Etk_Widget*>self.obj)
        if __char_ret != NULL:
            __ret = __char_ret
        return (__ret)

    def theme_file_set(self, char* theme_file):
        etk_widget_theme_file_set(<Etk_Widget*>self.obj, theme_file)

    property theme_file:
        def __get__(self):
            return self.theme_file_get()

        def __set__(self, value):
            self.theme_file_set(value)

    def theme_group_get(self):
        cdef char *__ret
        __ret = etk_widget_theme_group_get(<Etk_Widget*>self.obj)
        if __ret == NULL:
            __ret = None
        return (__ret)

    def theme_group_set(self, char* theme_group):
        etk_widget_theme_group_set(<Etk_Widget*>self.obj, theme_group)

    property theme_group:
        def __get__(self):
            return self.theme_group_get()

        def __set__(self, value):
            self.theme_group_set(value)

    def theme_parent_get(self):
        cdef Etk_Widget *wid
        wid = etk_widget_theme_parent_get(<Etk_Widget*>self.obj)
        return Object_from_instance(<Etk_Object*>wid)

    def theme_parent_set(self, Widget theme_parent):
        cdef Etk_Widget *tp
        if theme_parent is None:
            tp = NULL
        else:
            tp = <Etk_Widget*>theme_parent.obj
        etk_widget_theme_parent_set(<Etk_Widget*>self.obj, tp)

    property theme_parent:
        def __get__(self):
            return self.theme_parent_get()

        def __set__(self, value):
            self.theme_parent_set(value)

    def theme_part_text_set(self, char* part_name, char* text):
        etk_widget_theme_part_text_set(<Etk_Widget*>self.obj, part_name, text)

    def theme_set(self, char* theme_file, char* theme_group):
        etk_widget_theme_set(<Etk_Widget*>self.obj, theme_file, theme_group)

    property theme:
        def __set__(self, spec):
            self.theme_set(*spec)

    def theme_signal_emit(self, char* signal_name, int size_recalc):
        etk_widget_theme_signal_emit(<Etk_Widget*>self.obj, signal_name,
                                     <Etk_Bool>size_recalc)

    def toplevel_evas_get(self):
        cdef evas.c_evas.Evas *canvas
        canvas = etk_widget_toplevel_evas_get(<Etk_Widget*>self.obj)
        return evas.c_evas._Canvas_from_instance(<long>canvas)

    property toplevel_evas:
        def __get__(self):
            return self.toplevel_evas_get()

    def toplevel_parent_get(self):
        cdef Etk_Toplevel *wid
        wid = etk_widget_toplevel_parent_get(<Etk_Widget*>self.obj)
        return Object_from_instance(<Etk_Object*>wid)

    property toplevel_parent:
        def __get__(self):
            return self.toplevel_parent_get()

    property realized:
        def __get__(self):
            return bool((<Etk_Widget*>self.obj).realized)

    property theme_object:
        def __get__(self):
            return evas.c_evas._Object_from_instance(<long>(<Etk_Widget*>self.obj).theme_object)

    property smart_object:
        def __get__(self):
            return evas.c_evas._Object_from_instance(<long>(<Etk_Widget*>self.obj).theme_object)

    def unfocus(self):
        etk_widget_unfocus(<Etk_Widget*>self.obj)

    def unswallow_object(self, evas.c_evas.Object eobj):
        etk_widget_unswallow_object(<Etk_Widget*>self.obj, eobj.obj)

    def unswallow_widget(self, Widget swallowed):
        etk_widget_unswallow_widget(<Etk_Widget*>self.obj,
                                    <Etk_Widget*>swallowed.obj)

    property SHOWN_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_SHOWN_SIGNAL

    def on_shown(self, func, *a, **ka):
        self.connect(self.SHOWN_SIGNAL, func, *a, **ka)

    property HIDDEN_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_HIDDEN_SIGNAL

    def on_hidden(self, func, *a, **ka):
        self.connect(self.HIDDEN_SIGNAL, func, *a, **ka)

    property REALIZED_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_REALIZED_SIGNAL

    def on_realized(self, func, *a, **ka):
        self.connect(self.REALIZED_SIGNAL, func, *a, **ka)

    property UNREALIZED_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_UNREALIZED_SIGNAL

    def on_unrealized(self, func, *a, **ka):
        self.connect(self.UNREALIZED_SIGNAL, func, *a, **ka)

    property SIZE_REQUESTED_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_SIZE_REQUESTED_SIGNAL

    def on_size_requested(self, func, *a, **ka):
        self.connect(self.SIZE_REQUESTED_SIGNAL, func, *a, **ka)

    property MOUSE_IN_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_MOUSE_IN_SIGNAL

    def on_mouse_in(self, func, *a, **ka):
        self.connect(self.MOUSE_IN_SIGNAL, func, *a, **ka)

    property MOUSE_OUT_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_MOUSE_OUT_SIGNAL

    def on_mouse_out(self, func, *a, **ka):
        self.connect(self.MOUSE_OUT_SIGNAL, func, *a, **ka)

    property MOUSE_MOVE_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_MOUSE_MOVE_SIGNAL

    def on_mouse_move(self, func, *a, **ka):
        self.connect(self.MOUSE_MOVE_SIGNAL, func, *a, **ka)

    property MOUSE_DOWN_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_MOUSE_DOWN_SIGNAL

    def on_mouse_down(self, func, *a, **ka):
        self.connect(self.MOUSE_DOWN_SIGNAL, func, *a, **ka)

    property MOUSE_UP_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_MOUSE_UP_SIGNAL

    def on_mouse_up(self, func, *a, **ka):
        self.connect(self.MOUSE_UP_SIGNAL, func, *a, **ka)

    property MOUSE_CLICK_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_MOUSE_CLICK_SIGNAL

    def on_mouse_click(self, func, *a, **ka):
        self.connect(self.MOUSE_CLICK_SIGNAL, func, *a, **ka)

    property MOUSE_WHEEL_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_MOUSE_WHEEL_SIGNAL

    def on_mouse_wheel(self, func, *a, **ka):
        self.connect(self.MOUSE_WHEEL_SIGNAL, func, *a, **ka)

    property KEY_DOWN_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_KEY_DOWN_SIGNAL

    def on_key_down(self, func, *a, **ka):
        self.connect(self.KEY_DOWN_SIGNAL, func, *a, **ka)

    property KEY_UP_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_KEY_UP_SIGNAL

    def on_key_up(self, func, *a, **ka):
        self.connect(self.KEY_UP_SIGNAL, func, *a, **ka)

    property ENTERED_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_ENTERED_SIGNAL

    def on_entered(self, func, *a, **ka):
        self.connect(self.ENTERED_SIGNAL, func, *a, **ka)

    property LEFT_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_LEFT_SIGNAL

    def on_left(self, func, *a, **ka):
        self.connect(self.LEFT_SIGNAL, func, *a, **ka)

    property FOCUSED_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_FOCUSED_SIGNAL

    def on_focused(self, func, *a, **ka):
        self.connect(self.FOCUSED_SIGNAL, func, *a, **ka)

    property UNFOCUSED_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_UNFOCUSED_SIGNAL

    def on_unfocused(self, func, *a, **ka):
        self.connect(self.UNFOCUSED_SIGNAL, func, *a, **ka)

    property ENABLED_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_ENABLED_SIGNAL

    def on_enabled(self, func, *a, **ka):
        self.connect(self.ENABLED_SIGNAL, func, *a, **ka)

    property DISABLED_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_DISABLED_SIGNAL

    def on_disabled(self, func, *a, **ka):
        self.connect(self.DISABLED_SIGNAL, func, *a, **ka)

    property SCROLL_SIZE_CHANGED_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_SCROLL_SIZE_CHANGED_SIGNAL

    def on_scroll_size_changed(self, func, *a, **ka):
        self.connect(self.SCROLL_SIZE_CHANGED_SIGNAL, func, *a, **ka)

    property SELECTION_RECEIVED_SIGNAL:
        def __get__(self):
            return ETK_WIDGET_SELECTION_RECEIVED_SIGNAL

    def on_selection_received(self, func, *a, **ka):
        self.connect(self.SELECTION_RECEIVED_SIGNAL, func, *a, **ka)
