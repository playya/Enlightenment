# Copyright (C) 2007-2008 ProFUSION embedded systems
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this Python-Edje. If not, see <http://www.gnu.org/licenses/>.

# This file is included verbatim by edje.edit.pyx

cdef class State:
    cdef EdjeEdit edje
    cdef object part
    cdef object _name
    cdef object _part_obj

    def __init__(self, Part part, char *name):
        self._part_obj = part
        self.edje = part.edje
        self.part = part.name
        self._name = name

    property name:
        def __get__(self):
            return self._name

    def part_get(self):
        return self._part_obj

    def name_set(self, new_name):
        return edje_edit_state_name_set(self.edje.obj, self.part, self._name,
                                        new_name)

    def copy_from(self, from_state):
        return bool(edje_edit_state_copy(self.edje.obj, self.part,
                                         from_state, self.name))

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

    def fill_origin_relative_get(self):
        cdef double x, y
        x = edje_edit_state_fill_origin_relative_x_get(self.edje.obj, self.part,
                                                       self.name)
        y = edje_edit_state_fill_origin_relative_y_get(self.edje.obj, self.part,
                                                       self.name)
        return (x, y)

    def fill_origin_relative_set(self, double x, double y):
        edje_edit_state_fill_origin_relative_x_set(self.edje.obj, self.part,
                                                   self.name, x)
        edje_edit_state_fill_origin_relative_y_set(self.edje.obj, self.part,
                                                   self.name, y)

    def fill_origin_offset_get(self):
        cdef int x, y
        x = edje_edit_state_fill_origin_offset_x_get(self.edje.obj, self.part,
                                                     self.name)
        y = edje_edit_state_fill_origin_offset_y_get(self.edje.obj, self.part,
                                                     self.name)
        return (x, y)

    def fill_origin_offset_set(self, x, y):
        edje_edit_state_fill_origin_offset_x_set(self.edje.obj, self.part,
                                                 self.name, x)
        edje_edit_state_fill_origin_offset_y_set(self.edje.obj, self.part,
                                                 self.name, y)

    def fill_size_relative_get(self):
        cdef double x, y
        x = edje_edit_state_fill_size_relative_x_get(self.edje.obj, self.part,
                                                     self.name)
        y = edje_edit_state_fill_size_relative_y_get(self.edje.obj, self.part,
                                                     self.name)
        return (x, y)

    def fill_size_relative_set(self, double x, double y):
        edje_edit_state_fill_size_relative_x_set(self.edje.obj, self.part,
                                                 self.name, x)
        edje_edit_state_fill_size_relative_y_set(self.edje.obj, self.part,
                                                 self.name, y)

    def fill_size_offset_get(self):
        cdef int x, y
        x = edje_edit_state_fill_size_offset_x_get(self.edje.obj, self.part,
                                                   self.name)
        y = edje_edit_state_fill_size_offset_y_get(self.edje.obj, self.part,
                                                   self.name)
        return (x, y)

    def fill_size_offset_set(self, x, y):
        edje_edit_state_fill_size_offset_x_set(self.edje.obj, self.part,
                                               self.name, x)
        edje_edit_state_fill_size_offset_y_set(self.edje.obj, self.part,
                                               self.name, y)

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

    def color_class_get(self):
        cdef char *cc
        cc = edje_edit_state_color_class_get(self.edje.obj, self.part,
                                             self.name)
        if cc == NULL:
            return None
        rcc = cc
        edje_edit_string_free(cc)
        return rcc

    def color_class_set(self, cc):
        if cc != "":
            edje_edit_state_color_class_set(self.edje.obj, self.part,
                                            self.name, cc)
        else:
            edje_edit_state_color_class_set(self.edje.obj, self.part,
                                            self.name, NULL)

    def external_params_get(self):
        cdef evas.c_evas.Eina_List *lst
        ret = []
        lst = edje_edit_state_external_params_list_get(self.edje.obj, self.part,
                                                       self.name)
        while lst:
            ret.append(edje.c_edje._ExternalParam_from_ptr(<long>lst.data))
            lst = lst.next
        return ret

    def external_param_get(self, param):
        cdef edje.c_edje.Edje_External_Param_Type type
        cdef void *value

        if not edje_edit_state_external_param_get(self.edje.obj, self.part,
                                                  self.name, param, &type,
                                                  &value):
            return None
        if type == edje.EDJE_EXTERNAL_PARAM_TYPE_INT:
            i = (<int *>value)[0]
            return (type, i)
        elif type == edje.EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
            d = (<double *>value)[0]
            return (type, d)
        elif type == edje.EDJE_EXTERNAL_PARAM_TYPE_STRING:
            s = <char *>value
            return (type, s)
        return None

    def external_param_int_get(self, param):
        cdef int value

        if not edje_edit_state_external_param_int_get(self.edje.obj, self.part,
                                                      self.name, param, &value):
            return None
        return value

    def external_param_double_get(self, param):
        cdef double value

        if not edje_edit_state_external_param_double_get(self.edje.obj, self.part,
                                                      self.name, param, &value):
            return None
        return value

    def external_param_string_get(self, param):
        cdef char *value

        if not edje_edit_state_external_param_string_get(self.edje.obj, self.part,
                                                      self.name, param, &value):
            return None
        r = value
        return r

    def external_param_set(self, param, value):
        if isinstance(value, (long, int)):
            return self.external_param_int_set(param, value)
        elif isinstance(value, float):
            return self.external_param_double_set(param, value)
        elif isinstance(value, basestring):
            return self.external_param_string_set(param, value)
        else:
            raise TypeError("invalid external parameter type '%s'" %
                            type(value).__name__)

    def external_param_int_set(self, param, value):
        return bool(edje_edit_state_external_param_int_set(self.edje.obj,
                                                           self.part, self.name,
                                                           param, value))

    def external_param_double_set(self, param, value):
        return bool(edje_edit_state_external_param_double_set(self.edje.obj,
                                                           self.part, self.name,
                                                           param, value))

    def external_param_string_set(self, param, value):
        return bool(edje_edit_state_external_param_string_set(self.edje.obj,
                                                           self.part, self.name,
                                                           param, value))

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

    def gradient_type_get(self):
        cdef char *t
        t = edje_edit_state_gradient_type_get(self.edje.obj, self.part,
                                              self.name)
        if t == NULL:
            return None
        ret = t
        edje_edit_string_free(t)
        return ret

    def gradient_type_set(self, type):
        return bool(edje_edit_state_gradient_type_set(self.edje.obj, self.part,
                                                      self.name, type))

    def gradient_spectra_get(self):
        cdef char *s
        s = edje_edit_state_gradient_spectra_get(self.edje.obj, self.part,
                                                 self.name)
        if s == NULL:
            return None
        ret = s
        edje_edit_string_free(s)
        return ret

    def gradient_spectra_set(self, s):
        edje_edit_state_gradient_spectra_set(self.edje.obj, self.part,
                                             self.name, s)

    def gradient_angle_get(self):
        return edje_edit_state_gradient_angle_get(self.edje.obj, self.part,
                                                  self.name)

    def gradient_angle_set(self, int angle):
        edje_edit_state_gradient_angle_set(self.edje.obj, self.part, self.name,
                                           angle)

    def gradient_use_fill_get(self):
        return bool(edje_edit_state_gradient_use_fill_get(self.edje.obj,
                                                          self.part, self.name))

    def gradient_rel1_relative_get(self):
        cdef double x, y
        x = edje_edit_state_gradient_rel1_relative_x_get(self.edje.obj,
                                                         self.part, self.name)
        y = edje_edit_state_gradient_rel1_relative_y_get(self.edje.obj,
                                                         self.part, self.name)
        return (x, y)

    def gradient_rel1_relative_set(self, double x, double y):
        edje_edit_state_gradient_rel1_relative_x_set(self.edje.obj, self.part,
                                                     self.name, x)
        edje_edit_state_gradient_rel1_relative_y_set(self.edje.obj, self.part,
                                                     self.name, y)

    def gradient_rel1_offset_get(self):
        cdef int x, y
        x = edje_edit_state_gradient_rel1_offset_x_get(self.edje.obj,
                                                       self.part, self.name)
        y = edje_edit_state_gradient_rel1_offset_y_get(self.edje.obj,
                                                       self.part, self.name)
        return (x, y)

    def gradient_rel1_offset_set(self, int x, int y):
        edje_edit_state_gradient_rel1_offset_x_set(self.edje.obj, self.part,
                                                   self.name, x)
        edje_edit_state_gradient_rel1_offset_y_set(self.edje.obj, self.part,
                                                   self.name, y)

    def gradient_rel2_relative_get(self):
        cdef double x, y
        x = edje_edit_state_gradient_rel2_relative_x_get(self.edje.obj,
                                                         self.part, self.name)
        y = edje_edit_state_gradient_rel2_relative_y_get(self.edje.obj,
                                                         self.part, self.name)
        return (x, y)

    def gradient_rel2_relative_set(self, double x, double y):
        edje_edit_state_gradient_rel2_relative_x_set(self.edje.obj, self.part,
                                                     self.name, x)
        edje_edit_state_gradient_rel2_relative_y_set(self.edje.obj, self.part,
                                                     self.name, y)

    def gradient_rel2_offset_get(self):
        cdef int x, y
        x = edje_edit_state_gradient_rel2_offset_x_get(self.edje.obj,
                                                       self.part, self.name)
        y = edje_edit_state_gradient_rel2_offset_y_get(self.edje.obj,
                                                       self.part, self.name)
        return (x, y)

    def gradient_rel2_offset_set(self, int x, int y):
        edje_edit_state_gradient_rel2_offset_x_set(self.edje.obj, self.part,
                                                   self.name, x)
        edje_edit_state_gradient_rel2_offset_y_set(self.edje.obj, self.part,
                                                   self.name, y)
