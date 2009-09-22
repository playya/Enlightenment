# Copyright (C) 2007-2008 Tiago Falcao
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this Python-Edje.  If not, see <http://www.gnu.org/licenses/>.

# This file is included verbatim by edje.edit.pyx

cdef class Part:
    cdef EdjeEdit edje
    cdef object _name

    property name:
        def __get__(self):
            return self._name
        def __set__(self, name):
            self.rename(name)

    def __init__(self, EdjeEdit edje, char *name):
        self.edje = edje
        self._name = name

    def restack_below(self):
        cdef unsigned char r
        r = edje_edit_part_restack_below(self.edje.obj, self.name)
        if r == 0:
            return False
        return True

    def restack_above(self):
        cdef unsigned char r
        r = edje_edit_part_restack_above(self.edje.obj, self.name)
        if r == 0:
            return False
        return True

    def rename(self, newname):
        cdef unsigned char r
        r = edje_edit_part_name_set(self.edje.obj, self.name, newname)
        if r == 0:
            return False
        self.name = newname
        return True

    property type:
        def __get__(self):
            return edje_edit_part_type_get(self.edje.obj, self.name)

    property states:
        def __get__(self):
            "@rtype: list of str"
            cdef evas.c_evas.Eina_List *lst, *itr
            ret = []
            lst = edje_edit_part_states_list_get(self.edje.obj, self.name)
            itr = lst
            while itr:
                ret.append(<char*>itr.data)
                itr = itr.next
            edje_edit_string_list_free(lst)
            return ret

    def state_get(self, char *sname):
        if self.state_exist(sname):
            return State(self, sname)

    def state_add(self, char *sname):
        edje_edit_state_add(self.edje.obj, self.name, sname)

    def state_del(self, char *sname):
        edje_edit_state_del(self.edje.obj, self.name, sname)

    def state_exist(self, char *sname):
        cdef unsigned char r
        r = edje_edit_state_exist(self.edje.obj, self.name, sname)
        if r == 0:
            return False
        return True

    def state_selected_get(self):
        cdef char *sel
        sel = edje_edit_part_selected_state_get(self.edje.obj, self.name)
        if sel == NULL: return None
        r = sel
        edje_edit_string_free(sel)
        return r

    def state_selected_set(self, char *state):
        edje_edit_part_selected_state_set(self.edje.obj, self.name, state)

    property clip_to:
        def __get__(self):
            cdef char *clipper
            clipper = edje_edit_part_clip_to_get(self.edje.obj, self.name)
            if clipper == NULL: return None
            r = clipper
            edje_edit_string_free(clipper)
            return r

        def __set__(self, clipper):
            if clipper == "" or clipper is None:
                edje_edit_part_clip_to_set(self.edje.obj, self.name, NULL)
            else:
                edje_edit_part_clip_to_set(self.edje.obj, self.name, clipper)

        def __del__(self):
            edje_edit_part_clip_to_set(self.edje.obj, self.name, NULL)

    property mouse_events:
        def __get__(self):
            return bool(edje_edit_part_mouse_events_get(self.edje.obj,
                                                        self.name))

        def __set__(self, me):
            if me:
                edje_edit_part_mouse_events_set(self.edje.obj, self.name, 1)
            else:
                edje_edit_part_mouse_events_set(self.edje.obj, self.name, 0)

    property repeat_events:
        def __get__(self):
            return bool(edje_edit_part_repeat_events_get(self.edje.obj,
                                                         self.name))

        def __set__(self, re):
            if re:
                edje_edit_part_repeat_events_set(self.edje.obj, self.name, 1)
            else:
                edje_edit_part_repeat_events_set(self.edje.obj, self.name, 0)

    property effect:
        def __get__(self):
            return edje_edit_part_effect_get(self.edje.obj, self.name)

        def __set__(self, effect):
            edje_edit_part_effect_set(self.edje.obj, self.name, effect)
