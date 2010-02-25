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

from details_widget_button import WidgetButton
from floater_opener import FloaterListOpener


# Populates a list with the parts of the group under edition, except for
# the one marked as selected, if any.
class WidgetPartList(FloaterListOpener, WidgetButton):
    def __init__(self, parent, title=None, list_get_cb=None,
                 sel_part_get_cb=None):
        def null_sel():
            return None

        self._sel_part_get_cb = sel_part_get_cb or null_sel
        FloaterListOpener.__init__(self, list_get_cb)
        WidgetButton.__init__(self, parent)

        self._value = None
        self._title = title or ""
        self.clicked = self._open

    def show(self):
        for o in self.objs:
            o.show()

    def hide(self):
        for o in self.objs:
            o.hide()

    def _open(self, widget, bt, *args):
        self._floater_open(bt)

    def _internal_value_set(self, val):
        self._value = val
        self._update()

    def _internal_value_get(self):
        return self._value

    def _update(self):
        if not self.obj:
            return

        if self._value:
            self.obj.label_set(self._value)
        else:
            self.obj.label_set("< None >")

    def _floater_list_items_update(self):
        list = []
        sel_part = self._sel_part_get_cb()

        for item in self._list_get_cb():
            if item == sel_part:
                continue
            list.append((item, item))
        return list

    def _floater_title_init(self):
        self._floater.title_set(self._title)

    def _floater_actions_init(self):
        self._floater.action_add("None", self._none_selected)
        FloaterListOpener._floater_actions_init(self)

    def _none_selected(self, *args):
        self.value_set("")
        self._floater_cancel()

    def value_set(self, value):
        self._value = value
        self._update()
        self._callback_call("changed")

