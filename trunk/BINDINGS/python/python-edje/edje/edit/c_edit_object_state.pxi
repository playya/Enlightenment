# Copyright (C) 2007-2008 Tiago Falcao, Ivan Briano
#
# This file is part of Python-Edje.
#
# Python-Edje is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# Python-Edje is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this Python-Edje.  If not, see <http://www.gnu.org/licenses/>.

# This file is included verbatim by edje.edit.pyx

cdef class State:
    cdef EdjeEdit edje
    cdef object part
    cdef object _name

    def __init__(self, Part part, char *name):
        self.edje = part.edje
        self.part = part.name
        self._name = name

    property name:
        def __get__(self):
            return self._name

    def rel1_relative_get(self):
        cdef double x, y
        x = edje_edit_state_rel1_relative_x_get(self.edje.obj, self.part,
                                                self.name)
        y = edje_edit_state_rel1_relative_y_get(self.edje.obj, self.part,
                                                self.name)
        return (x, y)

    def rel1_offset_get(self):
        cdef int x, y
        x = edje_edit_state_rel1_offset_x_get(self.edje.obj, self.part,
                                              self.name)
        y = edje_edit_state_rel1_offset_y_get(self.edje.obj, self.part,
                                              self.name)
        return (x, y)

    def rel1_to_get(self):
        cdef char *tx, *ty
        cdef object x, y
        tx = edje_edit_state_rel1_to_x_get(self.edje.obj, self.part, self.name)
        ty = edje_edit_state_rel1_to_y_get(self.edje.obj, self.part, self.name)
        if tx != NULL:
            x = tx
            edje_edit_string_free(tx)
        if ty != NULL:
            y = ty
            edje_edit_string_free(ty)
        return (x, y)

    def rel1_relative_set(self, double x, double y):
        edje_edit_state_rel1_relative_x_set(self.edje.obj, self.part, self.name,
                                            x)
        edje_edit_state_rel1_relative_y_set(self.edje.obj, self.part, self.name,
                                            y)
    def rel1_relative_x_set(self, double x):
        edje_edit_state_rel1_relative_x_set(self.edje.obj, self.part, self.name,
                                            x)
    def rel1_relative_y_set(self, double y):
        edje_edit_state_rel1_relative_y_set(self.edje.obj, self.part, self.name,
                                            y)

    def rel1_offset_set(self, int x, int y):
        edje_edit_state_rel1_offset_x_set(self.edje.obj, self.part, self.name,
                                          x)
        edje_edit_state_rel1_offset_y_set(self.edje.obj, self.part, self.name,
                                          y)
    def rel1_offset_x_set(self, int x):
        edje_edit_state_rel1_offset_x_set(self.edje.obj, self.part, self.name,
                                          x)
    def rel1_offset_y_set(self, int y):
        edje_edit_state_rel1_offset_y_set(self.edje.obj, self.part, self.name,
                                          y)

    def rel1_to_set(self, x, y):
        if x != "":
            edje_edit_state_rel1_to_x_set(self.edje.obj, self.part, self.name,
                                          x)
        else:
            edje_edit_state_rel1_to_x_set(self.edje.obj, self.part, self.name,
                                          NULL)
        if y != "":
            edje_edit_state_rel1_to_y_set(self.edje.obj, self.part, self.name,
                                          y)
        else:
            edje_edit_state_rel1_to_y_set(self.edje.obj, self.part, self.name,
                                          NULL)
        # remove when fixed in edje_edit
        edje_edit_part_selected_state_set(self.edje.obj, self.part, self.name)
    def rel1_to_x_set(self, x):
        if x != "":
            edje_edit_state_rel1_to_x_set(self.edje.obj, self.part, self.name,
                                          x)
        else:
            edje_edit_state_rel1_to_x_set(self.edje.obj, self.part, self.name,
                                          NULL)
        # remove when fixed in edje_edit
        edje_edit_part_selected_state_set(self.edje.obj, self.part, self.name)
    def rel1_to_y_set(self, y):
        if y != "":
            edje_edit_state_rel1_to_y_set(self.edje.obj, self.part, self.name,
                                          y)
        else:
            edje_edit_state_rel1_to_y_set(self.edje.obj, self.part, self.name,
                                          NULL)
        # remove when fixed in edje_edit
        edje_edit_part_selected_state_set(self.edje.obj, self.part, self.name)

    def rel2_relative_get(self):
        cdef double x, y
        x = edje_edit_state_rel2_relative_x_get(self.edje.obj, self.part,
                                                self.name)
        y = edje_edit_state_rel2_relative_y_get(self.edje.obj, self.part,
                                                self.name)
        return (x, y)

    def rel2_offset_get(self):
        cdef int x, y
        x = edje_edit_state_rel2_offset_x_get(self.edje.obj, self.part,
                                              self.name)
        y = edje_edit_state_rel2_offset_y_get(self.edje.obj, self.part,
                                              self.name)
        return (x, y)

    def rel2_to_get(self):
        cdef char *tx, *ty
        cdef object x, y
        tx = edje_edit_state_rel2_to_x_get(self.edje.obj, self.part, self.name)
        ty = edje_edit_state_rel2_to_y_get(self.edje.obj, self.part, self.name)
        if tx != NULL:
            x = tx
            edje_edit_string_free(tx)
        if ty != NULL:
            y = ty
            edje_edit_string_free(ty)
        return (x, y)

    def rel2_relative_set(self, double x, double y):
        edje_edit_state_rel2_relative_x_set(self.edje.obj, self.part, self.name,
                                            x)
        edje_edit_state_rel2_relative_y_set(self.edje.obj, self.part, self.name,
                                            y)
    def rel2_relative_x_set(self, double x):
        edje_edit_state_rel2_relative_x_set(self.edje.obj, self.part, self.name,
                                            x)
    def rel2_relative_y_set(self, double y):
        edje_edit_state_rel2_relative_y_set(self.edje.obj, self.part, self.name,
                                            y)

    def rel2_offset_set(self, int x, int y):
        edje_edit_state_rel2_offset_x_set(self.edje.obj, self.part, self.name,
                                          x)
        edje_edit_state_rel2_offset_y_set(self.edje.obj, self.part, self.name,
                                          y)
    def rel2_offset_x_set(self, int x):
        edje_edit_state_rel2_offset_x_set(self.edje.obj, self.part, self.name,
                                          x)
    def rel2_offset_y_set(self, int y):
        edje_edit_state_rel2_offset_y_set(self.edje.obj, self.part, self.name,
                                          y)

    def rel2_to_set(self, x, y):
        if x != "":
            edje_edit_state_rel2_to_x_set(self.edje.obj, self.part, self.name,
                                          x)
        else:
            edje_edit_state_rel2_to_x_set(self.edje.obj, self.part, self.name,
                                          NULL)
        if y != "":
            edje_edit_state_rel2_to_y_set(self.edje.obj, self.part, self.name,
                                          y)
        else:
            edje_edit_state_rel2_to_y_set(self.edje.obj, self.part, self.name,
                                          NULL)
        # remove when fixed in edje_edit
        edje_edit_part_selected_state_set(self.edje.obj, self.part, self.name)
    def rel2_to_x_set(self, x):
        if x != "":
            edje_edit_state_rel2_to_x_set(self.edje.obj, self.part, self.name,
                                          x)
        else:
            edje_edit_state_rel2_to_x_set(self.edje.obj, self.part, self.name,
                                          NULL)
        # remove when fixed in edje_edit
        edje_edit_part_selected_state_set(self.edje.obj, self.part, self.name)
    def rel2_to_y_set(self, y):
        if y != "":
            edje_edit_state_rel2_to_y_set(self.edje.obj, self.part, self.name,
                                          y)
        else:
            edje_edit_state_rel2_to_y_set(self.edje.obj, self.part, self.name,
                                          NULL)
        # remove when fixed in edje_edit
        edje_edit_part_selected_state_set(self.edje.obj, self.part, self.name)

    def color_get(self):
        cdef int r, g, b, a
        edje_edit_state_color_get(self.edje.obj, self.part, self.name,
                                  &r, &g, &b, &a)
        return (r, g, b, a)

    def color_set(self, int r, int g, int b, int a):
        edje_edit_state_color_set(self.edje.obj, self.part, self.name,
                                  r, g, b, a)

    def color2_get(self):
        cdef int r, g, b, a
        edje_edit_state_color2_get(self.edje.obj, self.part, self.name,
                                  &r, &g, &b, &a)
        return (r, g, b, a)

    def color2_set(self, int r, int g, int b, int a):
        edje_edit_state_color2_set(self.edje.obj, self.part, self.name,
                                  r, g, b, a)

    def color3_get(self):
        cdef int r, g, b, a
        edje_edit_state_color3_get(self.edje.obj, self.part, self.name,
                                  &r, &g, &b, &a)
        return (r, g, b, a)

    def color3_set(self, int r, int g, int b, int a):
        edje_edit_state_color3_set(self.edje.obj, self.part, self.name,
                                   r, g, b, a)

    def align_get(self):
        cdef double x, y
        x = edje_edit_state_align_x_get(self.edje.obj, self.part, self.name)
        y = edje_edit_state_align_y_get(self.edje.obj, self.part, self.name)
        return (x, y)

    def align_set(self, double x, double y):
        edje_edit_state_align_x_set(self.edje.obj, self.part, self.name, x)
        edje_edit_state_align_y_set(self.edje.obj, self.part, self.name, y)

    property align:
        def __get__(self):
            return self.align_get()
        def __set__(self, align):
            self.align_set(*align)

    def min_get(self):
        cdef int w, h
        w = edje_edit_state_min_w_get(self.edje.obj, self.part, self.name)
        h = edje_edit_state_min_h_get(self.edje.obj, self.part, self.name)
        return (w, h)

    def min_set(self, int w, int h):
        edje_edit_state_min_w_set(self.edje.obj, self.part, self.name, w)
        edje_edit_state_min_h_set(self.edje.obj, self.part, self.name, h)

    property min:
        def __get__(self):
            return self.min_get()
        def __set__(self, min):
            self.min_set(*min)

    def max_get(self):
        cdef int w, h
        w = edje_edit_state_max_w_get(self.edje.obj, self.part, self.name)
        h = edje_edit_state_max_h_get(self.edje.obj, self.part, self.name)
        return (w, h)

    def max_set(self, int w, int h):
        edje_edit_state_max_w_set(self.edje.obj, self.part, self.name, w)
        edje_edit_state_max_h_set(self.edje.obj, self.part, self.name, h)

    property max:
        def __get__(self):
            return self.max_get()
        def __set__(self, max):
            self.max_set(*max)

    def aspect_min_get(self):
        cdef double a
        a = edje_edit_state_aspect_min_get(self.edje.obj, self.part, self.name)
        return a

    def aspect_min_set(self, double a):
        edje_edit_state_aspect_min_set(self.edje.obj, self.part, self.name, a)

    def aspect_max_get(self):
        cdef double a
        a = edje_edit_state_aspect_max_get(self.edje.obj, self.part, self.name)
        return a

    def aspect_max_set(self, double a):
        edje_edit_state_aspect_max_set(self.edje.obj, self.part, self.name, a)

    def aspect_pref_get(self):
        cdef char a
        a = edje_edit_state_aspect_pref_get(self.edje.obj, self.part, self.name)
        return a

    def aspect_pref_set(self, char a):
        edje_edit_state_aspect_pref_set(self.edje.obj, self.part, self.name, a)

    # TODO: fill_*

    property visible:
        def __get__(self):
            return bool(edje_edit_state_visible_get(self.edje.obj, self.part,
                                                    self.name))
        def __set__(self, v):
            if v:
                edje_edit_state_visible_set(self.edje.obj, self.part, self.name,
                                            1)
            else:
                edje_edit_state_visible_set(self.edje.obj, self.part, self.name,                                            0)

    # TODO: color_class

    def text_get(self):
        cdef char *t
        t = edje_edit_state_text_get(self.edje.obj, self.part, self.name)
        if t == NULL: return None
        r = t
        edje_edit_string_free(t)
        return r

    def text_set(self, t):
        edje_edit_state_text_set(self.edje.obj, self.part, self.name, t)

    def font_get(self):
        cdef char *f
        f = edje_edit_state_font_get(self.edje.obj, self.part, self.name)
        if f == NULL: return None
        r = f
        edje_edit_string_free(f)
        return r

    def font_set(self, f):
        edje_edit_state_font_set(self.edje.obj, self.part, self.name, f)

    def text_size_get(self):
        return edje_edit_state_text_size_get(self.edje.obj, self.part,
                                             self.name)

    def text_size_set(self, int s):
        edje_edit_state_text_size_set(self.edje.obj, self.part, self.name, s)

    def text_align_get(self):
        cdef double x, y
        x = edje_edit_state_text_align_x_get(self.edje.obj, self.part,
                                             self.name)
        y = edje_edit_state_text_align_y_get(self.edje.obj, self.part,
                                             self.name)
        return (x, y)

    def text_align_set(self, double x, double y):
        edje_edit_state_text_align_x_set(self.edje.obj, self.part, self.name, x)
        edje_edit_state_text_align_y_set(self.edje.obj, self.part, self.name, y)

    property text_align:
        def __get__(self):
            return self.text_align_get()
        def __set__(self, align):
            self.text_align_set(*align)

    def text_elipsis_get(self):
        return edje_edit_state_text_elipsis_get(self.edje.obj, self.part,
                                                self.name)

    def text_elipsis_set(self, double e):
        edje_edit_state_text_elipsis_set(self.edje.obj, self.part, self.name, e)

    def text_fit_get(self):
        x = bool(edje_edit_state_text_fit_x_get(self.edje.obj, self.part,
                                                self.name))
        y = bool(edje_edit_state_text_fit_y_get(self.edje.obj, self.part,
                                                self.name))
        return (x, y)

    def text_fit_set(self, x, y):
        if x:
            edje_edit_state_text_fit_x_set(self.edje.obj, self.part, self.name,
                                           1)
        else:
            edje_edit_state_text_fit_x_set(self.edje.obj, self.part, self.name,
                                           0)
        if y:
            edje_edit_state_text_fit_y_set(self.edje.obj, self.part, self.name,
                                           1)
        else:
            edje_edit_state_text_fit_y_set(self.edje.obj, self.part, self.name,
                                           0)

    def image_get(self):
        cdef char *img
        img = edje_edit_state_image_get(self.edje.obj, self.part, self.name)
        if img == NULL:
            return None
        r = img
        edje_edit_string_free(img)
        return r

    def image_set(self, char *image):
        edje_edit_state_image_set(self.edje.obj, self.part, self.name, image)

    def image_border_get(self):
        cdef int l, r, t, b
        edje_edit_state_image_border_get(self.edje.obj, self.part, self.name,
                                         &l, &r, &t, &b)
        return (l, r, t, b)

    def image_border_set(self, int l, int r, int t, int b):
        edje_edit_state_image_border_set(self.edje.obj, self.part, self.name,
                                         l, r, t, b)

    def image_border_fill_get(self):
        cdef unsigned char r
        r = edje_edit_state_image_border_fill_get(self.edje.obj, self.part,
                                                  self.name)
        if r == 0:
            return False
        return True

    def image_border_fill_set(self, fill):
        if fill:
            edje_edit_state_image_border_fill_set(self.edje.obj, self.part,
                                                  self.name, 1)
        else:
            edje_edit_state_image_border_fill_set(self.edje.obj, self.part,
                                                  self.name, 0)

    property tweens:
        def __get__(self):
            "@rtype: list of str"
            cdef evas.c_evas.Eina_List *lst, *itr
            ret = []
            lst = edje_edit_state_tweens_list_get(self.edje.obj, self.part,
                                                  self.name)
            itr = lst
            while itr:
                ret.append(<char*>itr.data)
                itr = itr.next
            edje_edit_string_list_free(lst)
            return ret

    def tween_add(self, char *img):
        cdef unsigned char r
        r = edje_edit_state_tween_add(self.edje.obj, self.part, self.name, img)
        if r == 0:
            return False
        return True

    def tween_del(self, char *img):
        cdef unsigned char r
        r = edje_edit_state_tween_del(self.edje.obj, self.part, self.name, img)
        if r == 0:
            return False
        return True

