# Copyright (c) 2008-2009 Simon Busch
#
# This file is part of python-elementary.
#
# python-elementary is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# python-elementary is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with python-elementary.  If not, see <http://www.gnu.org/licenses/>.
#

cdef class Spinner(Object):
    def __init__(self, c_evas.Object parent):
        self._set_obj(elm_spinner_add(parent.obj))

    def label_format_set(self, format):
        elm_spinner_label_format_set(self.obj, format)

    def label_format_get(self):
        cdef char *fmt
        fmt = elm_spinner_label_format_get(self.obj)
        return fmt

    def min_max_set(self, min, max):
        elm_spinner_min_max_set(self.obj, min, max)

    def step_set(self, step):
        elm_spinner_step_set(self.obj, step)

    def value_set(self, value):
        elm_spinner_value_set(self.obj, value)

    def value_get(self):
        cdef double value
        value = elm_spinner_value_get(self.obj)
        return value

    def wrap_set(self, wrap):
        if wrap:
            elm_spinner_wrap_set(self.obj, 1)
        else:
            elm_spinner_wrap_set(self.obj, 0)

    property changed:
        def __set__(self, value):
            self._callback_add("changed", value)

    property delay_changed:
        def __set__(self, value):
            self._callback_add("delay,changed", value)
