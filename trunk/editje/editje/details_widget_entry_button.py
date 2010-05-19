# Copyright (C) 2009 Samsung Electronics.
#
# This file is part of Editje.
#
# Editje is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# Editje is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with Editje. If not, see <http://www.gnu.org/licenses/>.

import elementary

from details_widget_entry import WidgetEntry


class WidgetEntryButton(WidgetEntry):
    pop_min_w = 200
    pop_min_h = 300

    def __init__(self, parent):
        WidgetEntry.__init__(self, parent)

        self.selection_list = []

        self.rect = elementary.Button(parent)
        self.rect.label_set("...")
        self.rect.size_hint_align_set(-1.0, -1.0)
        self.rect.size_hint_min_set(30, 16)
        self.rect.callback_clicked_add(self._open)
        self.rect.style_set("editje.details")
        self.rect.show()

        self.box = elementary.Box(parent)
        self.box.horizontal_set(True)
        self.box.size_hint_weight_set(1.0, 0.0)
        self.box.size_hint_align_set(-1.0, -1.0)
        self.box.pack_end(self.scr)
        self.box.pack_end(self.rect)
        self.box.show()

        self.obj = self.box


    def _value_set(self, val):
        self._internal_value_set(val)

    def _value_get(self):
        return self._value

    value = property(_value_get, _value_set)

    def _entry_changed_cb(self, obj, *args, **kwargs):
        WidgetEntry._entry_changed_cb(self, obj, *args, **kwargs)

    def _internal_value_set(self, val):
        WidgetEntry._internal_value_set(self, val)
        self.entry.select_all()

