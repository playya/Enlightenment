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
import evas
import edje
import elementary

from floater import Wizard

class NewPart(Wizard):

    def __init__(self, parent):
        Wizard.__init__(self, parent, "New Part")
        self.page_add("default")
#        self.style_set("minimal")

        self._name_init()
        self._types_init()

        self.action_add("default", "Cancel", self._cancel, icon="cancel")
        self.action_add("default", "Add", self._add, icon="confirm")
        self.goto("default")
        edje.message_signal_process()
        self._name_changed = False
        self._name.callback_changed_add(self._name_changed_cb)

    def _name_init(self):
        bx2 = elementary.Box(self)
        bx2.horizontal_set(True)
        bx2.size_hint_weight_set(evas.EVAS_HINT_EXPAND, 0.0)
        bx2.size_hint_align_set(evas.EVAS_HINT_FILL,
                                evas.EVAS_HINT_FILL)
        self.content_append("default", bx2)
        bx2.show()

        lb = elementary.Label(self)
        lb.label_set("Name:")
        bx2.pack_end(lb)
        lb.show()

        scr = elementary.Scroller(self)
        scr.size_hint_weight_set(evas.EVAS_HINT_EXPAND,
                                 evas.EVAS_HINT_EXPAND)
        scr.size_hint_align_set(evas.EVAS_HINT_FILL,
                                evas.EVAS_HINT_FILL)
        scr.content_min_limit(False, True)
        scr.policy_set(elementary.ELM_SCROLLER_POLICY_OFF,
                       elementary.ELM_SCROLLER_POLICY_OFF)
        scr.bounce_set(False, False)
        bx2.pack_end(scr)

        self._name = elementary.Entry(self)
        self._name.single_line_set(True)
        self._name.size_hint_weight_set(evas.EVAS_HINT_EXPAND, 0.0)
        self._name.size_hint_align_set(evas.EVAS_HINT_FILL, 0.0)
        #self._name.entry_set("")
        self._name.show()

        self._name_changed = False

        scr.content_set(self._name)
        scr.show()

    def _name_changed_cb(self, obj):
        self._name_changed = True

    def _types_init(self):
        list = elementary.List(self)
        list.size_hint_weight_set(1.0, 1.0)
        list.size_hint_align_set(-1.0, -1.0)

        list.item_append("Rectangle", None, None, self._type_select,
                         edje.EDJE_PART_TYPE_RECTANGLE).selected_set(False)
        list.item_append("Text", None, None, self._type_select,
                         edje.EDJE_PART_TYPE_TEXT)
        list.item_append("Image", None, None, self._type_select,
                         edje.EDJE_PART_TYPE_IMAGE)
        list.item_append("Swallow", None, None, self._type_select,
                         edje.EDJE_PART_TYPE_SWALLOW)
        list.item_append("TextBlock", None, None, self._type_select,
                         edje.EDJE_PART_TYPE_TEXTBLOCK)
        list.item_append("Gradient", None, None, self._type_select,
                         edje.EDJE_PART_TYPE_GRADIENT)
        list.item_append("Group", None, None, self._type_select,
                         edje.EDJE_PART_TYPE_GROUP)
#        list.item_append("Box", None, None, self._type_select,
#                         edje.EDJE_PART_TYPE_BOX)
#        list.item_append("Table", None, None, self._type_select,
#                         edje.EDJE_PART_TYPE_TABLE)
        list.item_append("External Widget", None, None, self._type_select,
                         edje.EDJE_PART_TYPE_EXTERNAL)
        list.go()

        self.content_append("default", list)
        list.show()

    def _type_select(self, li, it, type):
        self._type = type
        if not self._name_changed:
            self._name.entry_set(it.label_get().replace(" ", ""))
            edje.message_signal_process()
            self._name_changed = False

    def _add(self, popup, data):
        name = self._name.entry_get().replace("<br>", "")
        if name == "":
            self._notify("Please set part name")
            return

        if self._type == edje.EDJE_PART_TYPE_EXTERNAL:
            self._part_external_select(name)
        else:
            self._part_add(name)

    def _part_add(self, name, source=""):
        success = self._parent.e.part_add(name, self._type, source)
        if success:
            self._part_init(name, self._type)
        else:
            self._notify("Choose another name")

    def _part_init(self, name, type):
        part = self._parent.e._edje.part_get(name)
        statename = part.state_selected_get()
        state = part.state_get(statename)
        if type == edje.EDJE_PART_TYPE_RECTANGLE:
            self._part_init_rectangle(part, state)
        elif type == edje.EDJE_PART_TYPE_TEXT:
            self._part_init_text(part, state)
        elif type == edje.EDJE_PART_TYPE_EXTERNAL:
            self._part_init_external(part, state)
        self.close()

    def _part_init_rectangle(self, part, state):
        part.mouse_events = False

        state.color_set(0, 255, 0, 128)
        state.rel1_relative_set(0.3, 0.3)
        state.rel2_relative_set(0.7, 0.7)

    def _part_init_text(self, part, state):
        part.mouse_events = False

        state.color_set(0, 0, 0, 255)
        state.text_set("YOUR TEXT HERE")
        state.font_set("Sans")
        state.text_size_set(16)
        state.rel1_relative_set(0.3, 0.3)
        state.rel2_relative_set(0.7, 0.7)

    def _part_init_external(self, name, state):
        pass

    def _part_external_select(self, name):
        self.page_add("external", "Select Widget")

        self.external = ExternalSelector(self)
        self.content_append("external", self.external)
        self.external.show()

        self.action_add("external", "Ok", self._external_ok, name)
        self.action_add("external", "Cancel", self._cancel, name)

        self.goto("external")

    def _external_ok(self, popup, data):
        if not self.external.type:
            self.close()
            return
        self._part_add(data, self.external.type)
        self._parent.e._edje.external_add(self.external.module)

    def _cancel(self, popup, data):
        self.close()

class ExternalSelector(elementary.Box):
    def __init__(self, parent):
        elementary.Box.__init__(self, parent)
        self.horizontal_set(True)
        self.size_hint_weight_set(evas.EVAS_HINT_EXPAND,
                                  evas.EVAS_HINT_EXPAND)
        self.size_hint_align_set(evas.EVAS_HINT_FILL,
                                 evas.EVAS_HINT_FILL)

        self._module = ""
        self._type = ""

        self._types_load()
        self._modules_init()
        self._types_init()
        self._modules_load()

    def _type_get(self):
        return self._type

    type = property(_type_get)

    def _module_get(self):
        return self._module

    module = property(_module_get)

    def _types_load(self):
        self._loaded_types = {}
        for type in edje.ExternalIterator():
            module = type.module
            name = type.name
            label = type.label_get()

            list = self._loaded_types.get(module)
            if not list:
                list = []
                self._loaded_types[module] = list
            elif (name, label) in list:
                continue
            list.append((name, label))

    def _modules_init(self):
        self._modules = elementary.List(self)
        self._modules.size_hint_weight_set(evas.EVAS_HINT_EXPAND,
                                              evas.EVAS_HINT_EXPAND)
        self._modules.size_hint_align_set(evas.EVAS_HINT_FILL,
                                             evas.EVAS_HINT_FILL)
        self.pack_end(self._modules)
        self._modules.show()

    def _types_init(self):
        self._types = elementary.List(self)
        self._types.size_hint_weight_set(evas.EVAS_HINT_EXPAND,
                                         evas.EVAS_HINT_EXPAND)
        self._types.size_hint_align_set(evas.EVAS_HINT_FILL,
                                        evas.EVAS_HINT_FILL)
        self.pack_end(self._types)
        self._types.show()

    def _modules_load(self):
        self._module = ""
        self._modules.clear()
        list = self._loaded_types.keys()

        list.sort(key=str.lower)

        if list:
            self._modules.item_append(list[0], None, None, self._module_select,
                                      list[0]).selected_set(True)
        for item in list[1:]:
            self._modules.item_append(item, None, None,
                                         self._module_select, item)
        self._modules.go()

    def _module_select(self, li, it, module):
        self._module = module
        self._types.clear()
        list = self._loaded_types.get(module)

        list.sort(key=lambda x:(str.lower(x[0])))

        if list:
            name, label = list[0]
            self._types.item_append(label, None, None, self._type_select,
                                    name).selected_set(True)
        for (name, label) in list[1:]:
            self._types.item_append(label, None, None, self._type_select, name)

        self._types.go()

    def _type_select(self, li, it, type):
        self._type = type
