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
import ecore
import evas
import evas.decorators
import edje
import elementary

from controller import Controller, View
from desktop_handler import Handler
from desktop_part_listener import PartListener, PartHighlight
import desktop_part_handlers as part_handlers
from desktop_parts_manager import PartsManager


class Desktop(Controller):
    def __init__(self, parent):
        Controller.__init__(self, parent)

        self.e = parent.e
        self.e.event_callback_add("group.changed", self._group_load)
        self.e.event_callback_add("group.min.changed", self._group_min_load)
        self.e.event_callback_add("group.max.changed", self._group_max_load)
        self.e.event_callback_add("part.added",
                                  self._part_added)
        self.e.part.event_callback_add("part.changed",
                                       self._part_load)
        self.e.part.event_callback_add("part.unselected",
                                       self._part_load)
        self.e.part.state.event_callback_add("rel1x.changed",
                                             self._rel1x_load)
        self.e.part.state.event_callback_add("rel1y.changed",
                                             self._rel1y_load)
        self.e.part.state.event_callback_add("rel2x.changed",
                                             self._rel2x_load)
        self.e.part.state.event_callback_add("rel2y.changed",
                                             self._rel2y_load)

    def _view_load(self):
        self._view = DesktopView(self, self.parent.view)

    def _group_load(self, emissor, data):
        self._view.group = self.e._edje

    def _group_min_load(self, emissor, data):
        self._view.manager.group_min_set(*data)

    def _group_max_load(self, emissor, data):
        self._view.manager.group_max_set(*data)

    def _part_load(self, emissor, data):
        if data:
            self._view.part = self.e.part
        else:
            self._view.part = None

    def _part_added(self, emissor, data):
        self._view.manager.parts_manager.part_load(data)

    def _rel1x_load(self, emissor, data):
        self._view.part_rel1x_set(data)

    def _rel1y_load(self, emissor, data):
        self._view.part_rel1y_set(data)

    def _rel2x_load(self, emissor, data):
        self._view.part_rel2x_set(data)

    def _rel2y_load(self, emissor, data):
        self._view.part_rel2y_set(data)

class DesktopView(View, elementary.Scroller):

    def __init__(self, controller, parent_view):
        View.__init__(self, controller, parent_view)
        self._layout_load()

    def _layout_load(self):
        elementary.Scroller.__init__(self, self.parent_view)
        self.size_hint_weight_set(1.0, 1.0)
        self.size_hint_align_set(-1.0, -1.0)
        self.policy_set(elementary.ELM_SCROLLER_POLICY_AUTO,
                        elementary.ELM_SCROLLER_POLICY_AUTO)
        self.bounce_set(False, False)

        self.manager = EditManager(self.controller, self)
        self.content_set(self.manager)
        self.manager.show()

    # Group
    def _group_set(self, group):
        self._group = group
        self.manager.group = group

    group = property(fset=_group_set)

    # Part
    def _part_set(self, part):
        self._part = part
        if part:
            self.manager.part = self._group.part_object_get(self._part.name)
        else:
            self.manager.part = None

    part = property(fset=_part_set)

    def _get_relative_part(self, part):
        part = self._group.part_object_get(part)
        if part:
            return part
        return self._group

    def part_rel1x_set(self, data):
        self.rel1x_to, rel, ofs = data
        to = self._get_relative_part(self.rel1x_to)
        if self.rel1x_to and to == self._group:
            self.rel1x_to = ""
            self.part_rel1x_change(rel, ofs)
        else:
            self.manager.rel1x = (to, rel, ofs)

    def part_rel1y_set(self, data):
        self.rel1y_to, rel, ofs = data
        to = self._get_relative_part(self.rel1y_to)
        if self.rel1y_to and to == self._group:
            self.rel1y_to = ""
            self.part_rel1y_change(rel, ofs)
        else:
            self.manager.rel1y = (to, rel, ofs)

    def part_rel2x_set(self, data):
        self.rel2x_to, rel, ofs = data
        to = self._get_relative_part(self.rel2x_to)
        if self.rel2x_to and to == self._group:
            self.rel2x_to = ""
            self.part_rel2x_change(rel, ofs)
        else:
            self.manager.rel2x = (to, rel, ofs)

    def part_rel2y_set(self, data):
        self.rel2y_to, rel, ofs = data
        to = self._get_relative_part(self.rel2y_to)
        if self.rel2y_to and to == self._group:
            self.rel2y_to = ""
            self.part_rel2y_change(rel, ofs)
        else:
            self.manager.rel2y = (to, rel, ofs)

    def part_clicked(self, part):
        self.controller.e.part.name = part

    def part_rel1x_change(self, rel, ofs):
        self.controller.e.part.state.rel1x = (self.rel1x_to, rel, ofs)

    def part_rel1y_change(self, rel, ofs):
        self.controller.e.part.state.rel1y = (self.rel1y_to, rel, ofs)

    def part_rel2x_change(self, rel, ofs):
        self.controller.e.part.state.rel2x = (self.rel2x_to, rel, ofs)

    def part_rel2y_change(self, rel, ofs):
        self.controller.e.part.state.rel2y = (self.rel2y_to, rel, ofs)


class EditManager(View, evas.ClippedSmartObject):

    def __init__(self, controller, parent_view):
        View.__init__(self, controller, parent_view)
        evas.ClippedSmartObject.__init__(self, parent_view.evas)

        self._group = None
        self._group_min = (0, 0)
        self._group_max = (0, 0)
        self._part = None
        self._objs = []
        self._group_listeners = []
        self._part_listeners = []

        self.outside_area = self.evas.Rectangle(color=(0, 0, 0, 0))
        self.member_push(self.outside_area)
        self.outside_area.on_mouse_down_add(self._outside_area_clicked)
        self.outside_area.show()

        # Border
        self.border = GroupBorder(self)
        self.listener_member_push(self.border)

        #Group Handler
        self.group_handler = GroupResizeHandler(self)
        self.listener_member_push(self.group_handler)

        # Highlight
#        self.highlight1x = PartHighlight(self,
#                                     group = "editje/desktop/rel1/highlight")
#        self.member_push(self.highlight1x)
#        self.highlight1y = PartHighlight(self,
#                                     group = "editje/desktop/rel1/highlight")
#        self.member_push(self.highlight1y)
#        self.highlight2x = PartHighlight(self,
#                                     group = "editje/desktop/rel2/highlight")
#        self.member_push(self.highlight2x)
#        self.highlight2y = PartHighlight(self,
#                                     group = "editje/desktop/rel2/highlight")
#        self.member_push(self.highlight2y)

        self.parts_manager = PartsManager(self.evas)
        self.parts_manager.select = self.parent_view.part_clicked
        self.member_push(self.parts_manager)

        self._handlers_init()

        self.parent_view.on_resize_add(self._padding_init)

    def _handlers_init(self):
        #Hilight
        self.highlight = PartHighlight(self)
        self.listener_member_push(self.highlight)
        #Move
        self.handler_move = part_handlers.PartHandler_Move(self)
        self.listener_member_push(self.handler_move)
        #Top Right
        self.handler_tr = part_handlers.PartHandler_TR(self)
        self.listener_member_push(self.handler_tr)
        #Bottom Left
        self.handler_bl = part_handlers.PartHandler_BL(self)
        self.listener_member_push(self.handler_bl)
        # Top
        self.handler_t = part_handlers.PartHandler_T(self)
        self.listener_member_push(self.handler_t)
        # Left
        self.handler_l = part_handlers.PartHandler_L(self)
        self.listener_member_push(self.handler_l)
        # Bottom
        self.handler_b = part_handlers.PartHandler_B(self)
        self.listener_member_push(self.handler_b)
        # Right
        self.handler_r = part_handlers.PartHandler_R(self)
        self.listener_member_push(self.handler_r)
        #Top Left
        self.handler_tl = part_handlers.PartHandler_TL(self)
        self.listener_member_push(self.handler_tl)
        #Bottom Right
        self.handler_br = part_handlers.PartHandler_BR(self)
        self.listener_member_push(self.handler_br)

    def member_push(self, obj):
        self._objs.append(obj)
        self.member_add(obj)

    def listener_member_push(self, obj):
        if isinstance(obj, GroupListener):
            self._group_listeners.append(obj)
        if isinstance(obj, PartListener):
            self._part_listeners.append(obj)
        self.member_push(obj)

    def resize(self, w, h):
        self.outside_area.resize(w, h)

    def move(self, x, y):
        self.outside_area.move(x, y)

        if self._group is None:
            return
        ox, oy = self.pos
        dx = x - ox
        dy = y - oy
        self._group.move_relative(dx, dy)

    def delete(self):
        evas.ClippedSmartObject.delete(self)

    def _outside_area_clicked(self, o, ev):
        self.controller.e.part.name = ""

    #  GROUP CHANGED
    def group_min_set(self, w, h):
        self._group_min = (w, h)
        self.group_resize(*self._group.size)

    def group_max_set(self, w, h):
        self._group_max = (w, h)
        self.group_resize(*self._group.size)

    def group_resize(self, w, h):
        max_w, max_h = self._group_max
        min_w, min_h = self._group_min
        if max_w and w > max_w:
            w = max_w
        elif min_w and w < min_w:
            w = min_w

        if max_h and h > max_h:
            h = max_h
        elif min_h and h < min_h:
            h = min_h
        self._group.resize(w, h)
        self.padding_update()

    # Group
    def _group_member_add(self):
        for obj in self._objs:
            self.member_del(obj)
        self.member_add(self._group)
        for obj in self._objs:
            self.member_add(obj)

    def _group_set(self, group):
        if self._group:
            self._group.hide()
            self.member_del(self._group)
        self._group = group
        self.group_resize(300, 300)
        self._padding_init(self)
        self._group_member_add()
        self.outside_area.lower()
        group.show()
        for obj in self._group_listeners:
            obj.group = group
        self.parts_manager.edje = group

    def _group_get(self):
        return self._group

    group = property(_group_get, _group_set)

    # Part
    def _part_set(self, part):
        self._part = part
        self.rel1x = (self._group, 0.0, 0)
        self.rel1y = (self._group, 0.0, 0)
        self.rel2x = (self._group, 1.0, -1)
        self.rel2y = (self._group, 1.0, -1)
        for obj in self._part_listeners:
            obj.part = part

    def _part_get(self):
        return self._part

    part = property(_part_get, _part_set)

    # Rel1 X
    def _rel1x_set(self, value):
        self._rel1x = value
#        self.highlight1x.part = value[0]

    def _rel1x_get(self):
        return self._rel1x

    rel1x = property(_rel1x_get, _rel1x_set)

    # Rel1 Y
    def _rel1y_set(self, value):
        self._rel1y = value
#        self.highlight1y.part = value[0]

    def _rel1y_get(self):
        return self._rel1y

    rel1y = property(_rel1y_get, _rel1y_set)

    # Rel2 X
    def _rel2x_set(self, value):
        self._rel2x = value
#        self.highlight2x.part = value[0]

    def _rel2x_get(self):
        return self._rel2x

    rel2x = property(_rel2x_get, _rel2x_set)

    # Rel2 Y
    def _rel2y_set(self, value):
        self._rel2y = value
#        self.highlight2y.part = value[0]

    def _rel2y_get(self):
        return self._rel2y

    rel2y = property(_rel2y_get, _rel2y_set)

    # Padding

    def _padding_init(self, obj):
        if not self._group:
            return

        w, h = self._group.size
        w += 600
        h += 600
        scr_x, scr_y, scr_w, scr_h = self.parent_view.region_get()
        if w < scr_w:
            w = scr_w
        if h < scr_h:
            h = scr_h
        self.size_hint_min_set(w, h)
        self._group.center = self.center

        self.parent_view.region_show((w - scr_w) / 2, (h - scr_h) / 2,
                                     scr_w, scr_h)

    def padding_update_grow(self):
        w, h = self._group.size
        w += 600
        h += 600
        scr_x, scr_y, scr_w, scr_h = self.parent_view.region_get()
        if w < scr_w:
            w = scr_w
        if h < scr_h:
            h = scr_h

        w0, h0 = self.size_hint_min_get()
        if w0 > w:
            w = w0
        if h0 > h:
            h = h0
        self.size_hint_min_set(w, h)

    def padding_update(self):
        w, h = self._group.size
        w += 600
        h += 600
        scr_x, scr_y, scr_w, scr_h = self.parent_view.region_get()
        if w < scr_w:
            w = scr_w
        if h < scr_h:
            h = scr_h
        self.size_hint_min_set(w, h)

    # Actions
    def part_move1(self, dw, dh):
        dw += self._rel1x[2]
        self.parent_view.part_rel1x_change(self._rel1x[1], dw)
        dh += self._rel1y[2]
        self.parent_view.part_rel1y_change(self._rel1y[1], dh)

    def part_move2(self, dw, dh):
        dw += self._rel2x[2]
        self.parent_view.part_rel2x_change(self._rel2x[1], dw)
        dh += self._rel2y[2]
        self.parent_view.part_rel2y_change(self._rel2y[1], dh)

class GroupListener(object):
    def __init__(self):
        self._group = None

    def _group_set(self, group):
        if self._group:
            self._group.on_move_del(self.group_move)
            self._group.on_resize_del(self.group_move)

        if group:
            self._group = group
            self.top_left = group.bottom_right
            group.on_move_add(self.group_move)
            group.on_resize_add(self.group_move)
            group.on_del_add(self._group_del_cb)
            self.group_move(group)
            self.show()
        else:
            self._group = None
            self.hide()

    def _group_del_cb(self, obj):
        self._group = None

    group = property(fset=_group_set)

    def group_move(self, obj):
        print "MOVE", obj

class GroupBorder(GroupListener, edje.Edje):
    def __init__(self, parent):
        self._parent = parent
        edje.Edje.__init__(self, parent.evas, file=parent.theme,
                           group="editje/desktop/frame")
        GroupListener.__init__(self)

    def group_move(self, obj):
        if self._group:
            self.geometry = obj.geometry
            self.show()

class GroupResizeHandler(Handler, GroupListener):
    def __init__(self, parent):
        Handler.__init__(self, parent, "editje/desktop/handler")
        GroupListener.__init__(self)

    def group_move(self, obj):
        self.show()
        self.top_left = obj.bottom_right

    def down(self, x, y):
        if self._group:
            self._size = self._group.size

    def move(self, w, h):
        if self._group:
            self._parent.group_resize(self._size[0] + w, self._size[1] + h)
            self._parent.padding_update_grow()

    def up(self, w, h):
        if self._group:
            self._parent.padding_update()
            del self._size

class PartHighlight(PartListener, edje.Edje):
    def __init__(self, parent):
        edje.Edje.__init__(self, parent.evas, file=parent.theme,
                           group="editje/desktop/highlight")
        PartListener.__init__(self)

    def _part_set(self, part):
        PartListener._part_set(self, part)
        if self._part:
            self.signal_emit("animate", "")

    part = property(fset=_part_set)

    def part_move(self, obj):
        self.geometry = obj.geometry
        self.show()

class RelativePartListener(PartListener):
    def __init__(self):
        PartListener.__init__(self)
        self._rel_to = None
        self._rel_rel = None
        self._rel_ofs = None

    def _rel_set(self, rel):
        if self._rel_to:
            self._rel_to.on_move_del(self.part_move)
            self._rel_to.on_resize_add(self.part_move)

        if part:
            self._part = part
            part.on_move_add(self.part_move)
            part.on_resize_add(self.part_move)
            self.part_move(part)
            self.show()
        else:
            self._part = None
            self.hide()

    rel = property(fset=_rel_set)
