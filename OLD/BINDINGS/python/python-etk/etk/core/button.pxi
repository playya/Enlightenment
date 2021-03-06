# Copyright (C) 2007-2008 Caio Marcelo de Oliveira Filho
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

cdef public class Button(Bin) [object PyEtk_Button, type PyEtk_Button_Type]:
    def __init__(self, **kargs):
        if self.obj == NULL:
            self._set_obj(<Etk_Object*>etk_button_new())
        self._set_common_params(**kargs)

    def alignment_get(self):
        cdef float xalign
        cdef float yalign
        etk_button_alignment_get(<Etk_Button*>self.obj, &xalign, &yalign)
        return (xalign, yalign)

    def alignment_set(self, float xalign, float yalign):
        etk_button_alignment_set(<Etk_Button*>self.obj, xalign, yalign)

    def click(self):
        etk_button_click(<Etk_Button*>self.obj)

    def image_get(self):
        __ret = Object_from_instance(<Etk_Object*>etk_button_image_get(<Etk_Button*>self.obj))
        return (__ret)

    def image_set(self, Image image):
        etk_button_image_set(<Etk_Button*>self.obj, <Etk_Image*>image.obj)

    def label_get(self):
        cdef char *__char_ret
        __ret = None
        __char_ret = etk_button_label_get(<Etk_Button*>self.obj)
        if __char_ret != NULL:
            __ret = __char_ret
        return (__ret)

    def label_set(self, char* label):
        etk_button_label_set(<Etk_Button*>self.obj, label)

    def press(self):
        etk_button_press(<Etk_Button*>self.obj)

    def release(self):
        etk_button_release(<Etk_Button*>self.obj)

    def set_from_stock(self, int stock_id):
        etk_button_set_from_stock(<Etk_Button*>self.obj, <Etk_Stock_Id>stock_id)

    def stock_size_get(self):
        __ret = <int> etk_button_stock_size_get(<Etk_Button*>self.obj)
        return (__ret)

    def stock_size_set(self, int size):
        etk_button_stock_size_set(<Etk_Button*>self.obj, <Etk_Stock_Size>size)

    def style_get(self):
        __ret = <int> etk_button_style_get(<Etk_Button*>self.obj)
        return (__ret)

    def style_set(self, int style):
        etk_button_style_set(<Etk_Button*>self.obj, <Etk_Button_Style>style)

    property alignment:
        def __get__(self):
            return self.alignment_get()

        def __set__(self, alignment):
            self.alignment_set(*alignment)

    property image:
        def __get__(self):
            return self.image_get()

        def __set__(self, image):
            self.image_set(image)

    property label:
        def __get__(self):
            return self.label_get()

        def __set__(self, label):
            self.label_set(label)

    property stock_size:
        def __get__(self):
            return self.stock_size_get()

        def __set__(self, stock_size):
            self.stock_size_set(stock_size)

    property style:
        def __get__(self):
            return self.style_get()

        def __set__(self, style):
            self.style_set(style)

    def _set_common_params(self, image=None, label=None, stock_size=None, style=None, alignment=None, **kargs):
        if image is not None:
            self.image_set(image)
        if label is not None:
            self.label_set(label)
        if stock_size is not None:
            self.stock_size_set(stock_size)
        if style is not None:
            self.style_set(style)
        if alignment is not None:
            self.alignment_set(*alignment)

        if kargs:
            Bin._set_common_params(self, **kargs)

    property PRESSED_SIGNAL:
        def __get__(self):
            return ETK_BUTTON_PRESSED_SIGNAL

    def on_pressed(self, func, *a, **ka):
        self.connect(self.PRESSED_SIGNAL, func, *a, **ka)

    property RELEASED_SIGNAL:
        def __get__(self):
            return ETK_BUTTON_RELEASED_SIGNAL

    def on_released(self, func, *a, **ka):
        self.connect(self.RELEASED_SIGNAL, func, *a, **ka)

    property CLICKED_SIGNAL:
        def __get__(self):
            return ETK_BUTTON_CLICKED_SIGNAL

    def on_clicked(self, func, *a, **ka):
        self.connect(self.CLICKED_SIGNAL, func, *a, **ka)


class ButtonEnums:
    ICON = ETK_BUTTON_ICON
    TEXT = ETK_BUTTON_TEXT
    BOTH_HORIZ = ETK_BUTTON_BOTH_HORIZ
    BOTH_VERT = ETK_BUTTON_BOTH_VERT
