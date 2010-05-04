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

import evas
import edje
import elementary

from details_widget import Widget
from floater import Floater
import colorpicker
from details_widget_entry import WidgetEntryValidator
from misc import validator_rgba


class WidgetColor(Widget, WidgetEntryValidator):
    padding_x = 20
    padding_y = 20

    def __init__(self, parent, editable):
        Widget.__init__(self)
        WidgetEntryValidator.__init__(self)
        self.validator_set(validator_rgba)
        self._edit_grp = editable
        self.color = (255, 255, 255, 255)
        self.parent = parent
        self.entry = elementary.Entry(parent)
        self.entry.single_line_set(1)
        self.entry.style_set("editje.details")
        self.entry.size_hint_weight_set(1.0, 0.0)
        self.entry.context_menu_disabled_set(True)
        self.entry.callback_activated_add(self._entry_activate_cb)
        self.entry.callback_changed_add(self._entry_changed_cb)
        self.entry.callback_double_clicked_add(self._dblclick_cb)
        self.entry.callback_focused_add(self._focused_cb)
        self.entry.callback_unfocused_add(self._unfocused_cb)
        self.entry.show()

        self.scr = elementary.Scroller(parent)
        self.scr.style_set("editje.details")
        self.scr.size_hint_weight_set(1.0, 0.0)
        self.scr.size_hint_align_set(-1.0, -1.0)
        self.scr.policy_set(elementary.ELM_SCROLLER_POLICY_OFF,
                            elementary.ELM_SCROLLER_POLICY_OFF)
        self.scr.bounce_set(False, False)
        self.scr.content_set(self.entry)
        self.scr.content_min_limit(False, True)
        self.scr.show()

        ed = parent.edje_get()
        file = ed.file_get()[0]
        self.rect = edje.Edje(ed.evas, file=file, group="colorpreviewer")
        self.rect.size_hint_align_set(-1.0, -1.0)
        self.rect.size_hint_min_set(*self.rect.size_min_get())
        self.rect.on_mouse_down_add(self._sample_clicked_cb)
        self.rect.show()

        self.pop = Floater(self.parent, self.rect)
        self.picker = colorpicker.Colorpicker(self.parent)
        self.picker.show()
        self.pop.content_set(self.picker)
        self.pop.title_set("Color")
        self.pop.action_add("Set", self._set_clicked)
        self.pop.action_add("Cancel", self._popup_hide)
        self._edit_grp.part.callback_add(
            "part.unselected", self._popup_hide)
        self._edit_grp.part.callback_add(
            "part.changed", self._popup_hide)

        self.obj = elementary.Box(parent)
        self.obj.horizontal_set(True)
        self.obj.size_hint_weight_set(1.0, 0.0)
        self.obj.size_hint_align_set(-1.0, -1.0)
        self.obj.pack_end(self.scr)
        self.obj.pack_end(self.rect)
        self.obj.show()

        self.delayed_callback = 0

    def _value_set(self, val):
        self.entry.entry_set("%d %d %d %d" % val)
        self.color = val

    def _value_get(self):
        return self.color

    value = property(_value_get, _value_set)

    def show(self):
        self.scr.show()
        self.rect.show()

    def hide(self):
        self.scr.hide()
        self.rect.hide()

    def _entry_changed_cb(self, obj, *args, **kwargs):
        entry = self.entry.entry_get()
        val = self.entry.markup_to_utf8(entry)
        if not self._validator_call(self.obj, val):
            self.rect.color_class_set("colorpicker.sample", 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0)
            return

        if val[0] == "#":
            try:
                r, g, b, a = evas.color_parse(val)
            except Exception, e:
                return
        else:
            try:
                t = []
                t = val.split(None)
                if len(t) < 3:
                    return None
                elif len(t) == 4:
                    r, g, b, a = t
                else:
                    r, g, b = t
                    a = 255
            except Exception, e:
                return
        try:
            r = int(r)
            g = int(g)
            b = int(b)
            a = int(a)

        except Exception, e:
            print "Error parsing integer values for color: ", str(e)
            return

        self.color = (r, g, b, a)
        self.rect.color_class_set("colorpicker.sample", r, g, b, a,
                                  0, 0, 0, 0, 0, 0, 0, 0)
        if self.delayed_callback:
            self._callback_call("changed")
            self.delayed_callback = 0

    def _entry_activate_cb(self, obj, *args, **kwargs):
        if not self._validated:
            self._value_set(self.color)
            return

        self._callback_call("changed")

    def _dblclick_cb(self, obj):
        self.entry.select_all()

    def _focused_cb(self, obj):
        self.entry.select_all()

    def _unfocused_cb(self, obj):
        self.entry.select_none()

    def _sample_clicked_cb(self, obj, event):
        self.picker.current_color_set(*self.color)
        self.pop.show()

    def _set_clicked(self, popup, data):
        self.value = self.picker.current_color_get()
        self.delayed_callback = 1
        self.pop.hide()

    def _popup_hide(self, *args):
        self.pop.hide()
