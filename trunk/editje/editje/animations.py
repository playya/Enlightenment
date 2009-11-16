#
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with Editje.  If not, see
# <http://www.gnu.org/licenses/>.
import edje
import elementary

from details import EditjeDetails
from details_widget_entry import WidgetEntry
from details_widget_boolean import WidgetBoolean
from details_widget_color import WidgetColor
from details_widget_button import WidgetButton
from details_widget_combo import WidgetCombo
from floater import Floater
from prop import Property, PropertyTable

class NewAnimationPopUp(Floater):

    def __init__(self, parent):
        Floater.__init__(self, parent)
        self.title_set("New animation...")

        box = elementary.Box(parent)
        box.size_hint_weight_set(1.0, 1.0)
        box.size_hint_align_set(-1.0, -1.0)

        bx2 = elementary.Box(parent)
        bx2.horizontal_set(True)
        bx2.size_hint_weight_set(1.0, 0.0)
        bx2.size_hint_align_set(-1.0, 0.0)

        lb = elementary.Label(parent)
        lb.size_hint_weight_set(0.0, 1.0)
        lb.size_hint_align_set(-1.0, -1.0)
        lb.label_set("Name:")
        bx2.pack_end(lb)
        lb.show()

        self.name_entry = elementary.Entry(parent)
        self.name_entry.size_hint_weight_set(1.0, 1.0)
        self.name_entry.size_hint_align_set(-1.0, -1.0)
        self.name_entry.single_line_set(True)
        self.name_entry.style_set("editje_dialog")
        bx2.pack_end(self.name_entry)
        self.name_entry.show()

        box.pack_end(bx2)
        bx2.show()

        self.duration = elementary.Spinner(parent)
        self.duration.label_format_set("%.1fs")
        self.duration.min_max_set(0, 10)
        self.duration.step_set(0.1)
        self.duration.wrap_set(False)
        self.duration.size_hint_weight_set(1.0, 0.0)
        self.duration.size_hint_align_set(-1.0, 0.0)
        box.pack_end(self.duration)
        self.duration.show()

        self.content_set(box)
        box.show()

        self.action_add("Add", self._add_anim)
        self.action_add("Cancel", self._cancel)

    def _add_anim(self, popup, data):
        name = self.name_entry.entry_get().replace("<br>", "")
        if name == "":
            return
        self._parent.e.animation_add(name, self.duration.value_get())
        self.close()

    def _cancel(self, popup, data):
        self.close()


class AnimationDetails(EditjeDetails):

    def __init__(self, parent):
        EditjeDetails.__init__(self, parent,
                               group="editje/collapsable/part_properties")

        self.title_set("animation")

        self._transitions = ['None', 'Linear', 'Sinusoidal', 'Accelerate',
                             'Decelerate']

        self._header_table = PropertyTable(parent)

        prop = Property(parent, "name")
        wid = WidgetEntry(self)
        prop.widget_add("n", wid)
        self._header_table.property_add(prop)

        prop = Property(parent, "length")
        wid = WidgetEntry(self)
        wid.disabled_set(True)
        wid.type_float()
        #wid.parser_in = lambda x: str(x)
        #wid.parser_out = lambda x: float(x)
        prop.widget_add("l", wid)
        self._header_table.property_add(prop)

        self.content_set("part_name.swallow", self._header_table)

        prop = Property(parent, "current")
        wid = WidgetEntry(self)
        wid.disabled_set(True)
        prop.widget_add("c", wid)
        self["main"].property_add(prop)

        prop = Property(parent, "previous")
        wid = WidgetEntry(self)
        wid.disabled_set(True)
        prop.widget_add("p", wid)
        self["main"].property_add(prop)

        prop = Property(parent, "next")
        wid = WidgetEntry(self)
        wid.disabled_set(True)
        prop.widget_add("n", wid)
        self["main"].property_add(prop)

        prop = Property(parent, "transition")
        wid = WidgetCombo(self)
        for null, i in enumerate(self._transitions):
            wid.item_add(i)
        prop.widget_add("type", wid)
        wid = WidgetEntry(self)
        wid.disabled_set(True)
        wid.type_float()
        #wid.parser_in = lambda x: str(x)
        #wid.parser_out = lambda x: float(x)
        prop.widget_add("length", wid)
        self["main"].property_add(prop)

        self._parent.main_edje.signal_callback_add("timestop", "*",
                                                   self._timeline_cb)

    def editable_set(self, editable):
        self.e = editable
        self.e.animation.event_callback_add("animation.changed", self._update)
        self.e.animation.event_callback_add("state.added", self._timestop_add)
        self.e.animation.event_callback_add("state.changed", self._update_states)

    def _update(self, emissor, data):
        self._header_table["name"].value = data
        self._header_table["length"].value = "%.1gs" % self.e.animation.length
        self._timeline_update()

    def _timeline_cb(self, obj, emission, source):
        t = float(source)
        if not t in self.e.animation.timestamps:
            self.e.animation.state_add(t)
        self.e.animation.state = t

    def _timeline_update(self):
        for i in range(1, 11):
            sig = "ts,%.1g," % (i/10.0)
            self._parent.main_edje.signal_emit(sig + "disable", "editje")
            self._parent.main_edje.signal_emit(sig + "unselected", "editje")
        for s in self.e.animation.timestamps:
            sig = "ts,%.1g,enable" % s
            self._parent.main_edje.signal_emit(sig, "editje")

    def _timestop_add(self, emissor, data):
        self._parent.main_edje.signal_emit("ts,%.1g,enable" % data, "editje")
        self._header_table["length"].value = "%.1gs" % self.e.animation.length

    def _update_states(self, emissor, data):
        step = self.e.animation.state
        self["main"]["current"].value = str(step.timestamp)
        prev = self.e.animation.state_prev()
        if prev is None:
            self["main"]["previous"].value = "None"
        else:
            self["main"]["previous"].value = str(prev.timestamp)
        next = self.e.animation.state_next()
        if next is None:
            self["main"]["next"].value = "None"
        else:
            self["main"]["next"].value = str(next.timestamp)

        t = self._transitions[self.e.animation.program.transition]
        self["main"]["transition"].value = (t, str(step.length))

        sig = "ts,%.1g,selected" % self.e.animation.state.timestamp
        self._parent.main_edje.signal_emit(sig, "editje")
        if hasattr(self, "_last_timestamp"):
            sig = "ts,%.1g,unselected" % self._last_timestamp
        self._parent.main_edje.signal_emit(sig, "editje")
        self._last_timestamp = self.e.animation.state.timestamp

    def prop_value_changed(self, prop, value, group):
        if prop == "transition":
            t = self["main"]["transition"]["type"]
            p = self.e.program_get(self.e.animation.state.program_name)
            p.transition = self._transitions.index(t)
