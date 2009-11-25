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
        self._name.entry_set("")
        self._name.show()

        scr.content_set(self._name)
        scr.show()

    def _types_init(self):
        list = elementary.List(self)
        list.size_hint_weight_set(1.0, 1.0)
        list.size_hint_align_set(-1.0, -1.0)

        list.item_append("Rectangle", None, None, self._type_select,
                         edje.EDJE_PART_TYPE_RECTANGLE).selected_set(True)
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

        self.goto("external")

    def _external_ok(self, popup, data):
        print "ADD EXTERNAL", self.external.type
        self._part_add(data, self.external.type)

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

        self._types_load()
        self._namespaces_init()
        self._types_init()
        self._namespaces_load()

        self.type = ""

    def _name_split(self, name):
        names = name.rsplit("/", 1)
        if len(names) == 2:
            namespace = names[0]
            name = names[1]
        else:
            namespace = ""
        return (namespace, name)

    def _type_get(self):
        if self._namespace:
            return self._namespace + "/" + self._type
        return self._type

    def _type_set(self, type):
        self._namespace, self._type = self._name_split(type)

    type = property(_type_get, _type_set)

    def _types_load(self):
        self._loaded_types = {}
        for type in edje.ExternalIterator():
            print type.name
            namespace, name = self._name_split(type.name)

            list = self._loaded_types.get(namespace)
            if not list:
                list = []
                self._loaded_types[namespace] = list
            elif name in list:
                continue
            list.append(name)

    def _namespaces_init(self):
        self._namespaces = elementary.List(self)
        self._namespaces.size_hint_weight_set(evas.EVAS_HINT_EXPAND,
                                              evas.EVAS_HINT_EXPAND)
        self._namespaces.size_hint_align_set(evas.EVAS_HINT_FILL,
                                             evas.EVAS_HINT_FILL)
        self.pack_end(self._namespaces)
        self._namespaces.show()

    def _types_init(self):
        self._types = elementary.List(self)
        self._types.size_hint_weight_set(evas.EVAS_HINT_EXPAND,
                                         evas.EVAS_HINT_EXPAND)
        self._types.size_hint_align_set(evas.EVAS_HINT_FILL,
                                        evas.EVAS_HINT_FILL)
        self.pack_end(self._types)
        self._types.show()

    def _namespaces_load(self):
        self._namespace = ""
        self._namespaces.clear()
        list = self._loaded_types.keys()
        try:
            list.remove("")
        except ValueError:
            pass
        for item in list:
            self._namespaces.item_append(item, None, None,
                                         self._namespace_select, item)
        self._namespaces.go()

    def _namespace_select(self, li, it, namespace):
        self._namespace = namespace
        self._types.clear()
        list = self._loaded_types.get(namespace)
        for item in list:
            self._types.item_append(item, None, None, self._type_select, item)
        self._types.go()

    def _type_select(self, li, it, type):
        self._type = type



