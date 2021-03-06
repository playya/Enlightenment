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

cdef public class Entry(Widget) [object PyEtk_Entry, type PyEtk_Entry_Type]:
    def __init__(self, **kargs):
        if self.obj == NULL:
            self._set_obj(<Etk_Object*>etk_entry_new())
        self._set_common_params(**kargs)

    def clear(self):
        etk_entry_clear(<Etk_Entry*>self.obj)

    def clear_button_add(self):
        etk_entry_clear_button_add(<Etk_Entry*>self.obj)

    def image_get(self, int position):
        __ret = Object_from_instance(<Etk_Object*>etk_entry_image_get(<Etk_Entry*>self.obj, <Etk_Entry_Image_Position>position))
        return (__ret)

    def image_highlight_set(self, int position, int highlight):
        etk_entry_image_highlight_set(<Etk_Entry*>self.obj, <Etk_Entry_Image_Position>position, <Etk_Bool>highlight)

    def image_set(self, int position, Image image):
        etk_entry_image_set(<Etk_Entry*>self.obj, <Etk_Entry_Image_Position>position, <Etk_Image*>image.obj)

    def password_mode_get(self):
        __ret = bool(<int> etk_entry_password_mode_get(<Etk_Entry*>self.obj))
        return (__ret)

    def password_mode_set(self, int password_mode):
        etk_entry_password_mode_set(<Etk_Entry*>self.obj, <Etk_Bool>password_mode)

    def text_get(self):
        cdef char *__char_ret
        __ret = None
        __char_ret = etk_entry_text_get(<Etk_Entry*>self.obj)
        if __char_ret != NULL:
            __ret = __char_ret
        return (__ret)

    def text_set(self, char* text):
        etk_entry_text_set(<Etk_Entry*>self.obj, text)

    property password_mode:
        def __get__(self):
            return self.password_mode_get()

        def __set__(self, password_mode):
            self.password_mode_set(password_mode)

    property text:
        def __get__(self):
            return self.text_get()

        def __set__(self, text):
            self.text_set(text)

    def _set_common_params(self, password_mode=None, text=None, **kargs):
        if password_mode is not None:
            self.password_mode_set(password_mode)
        if text is not None:
            self.text_set(text)

        if kargs:
            Widget._set_common_params(self, **kargs)

    property TEXT_CHANGED_SIGNAL:
        def __get__(self):
            return ETK_ENTRY_TEXT_CHANGED_SIGNAL

    def on_text_changed(self, func, *a, **ka):
        self.connect(self.TEXT_CHANGED_SIGNAL, func, *a, **ka)

    property TEXT_ACTIVATED_SIGNAL:
        def __get__(self):
            return ETK_ENTRY_TEXT_ACTIVATED_SIGNAL

    def on_text_activated(self, func, *a, **ka):
        self.connect(self.TEXT_ACTIVATED_SIGNAL, func, *a, **ka)


class EntryEnums:
    PRIMARY = ETK_ENTRY_IMAGE_PRIMARY
    SECONDARY = ETK_ENTRY_IMAGE_SECONDARY
