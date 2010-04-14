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

from desktop_handler import Handler
from desktop_part_listener import PartListener
from operation import Operation


class PartHandler(Handler, PartListener):
    def __init__(self, editable_grp, desktop_scroller, canvas, theme_file,
                 rel1_move_offset_inform_cb=None,
                 rel2_move_offset_inform_cb=None, op_stack_cb=None,
                 group="editje/desktop/part/resize_handler"):
        Handler.__init__(
            self, editable_grp, desktop_scroller, canvas, theme_file, group,
            op_stack_cb)
        PartListener.__init__(self)
        self._rel1_move_offset_inform_cb = rel1_move_offset_inform_cb
        self._rel2_move_offset_inform_cb = rel2_move_offset_inform_cb

    def down(self, x, y):
        if self._part:
            self._geometry = self._part.geometry

    # one time only calls to move() (undo/redo) will call this
    def _part_and_state_select(self, part, state):
        self._edit_grp.part.name = part
        self._edit_grp.part.state.name = state


class PartHandler_Move(PartHandler):
    def __init__(self, editable_grp, desktop_scroller, canvas, theme_file,
                 rel1_move_offset_inform_cb=None,
                 rel2_move_offset_inform_cb=None, op_stack_cb=None,
                 group="editje/desktop/part/move_handler"):
        PartHandler.__init__(
            self, editable_grp, desktop_scroller, canvas, theme_file,
            rel1_move_offset_inform_cb, rel2_move_offset_inform_cb,
            op_stack_cb, group)
        self.size = (10, 10)

    def part_move(self, obj):
        self.center = obj.center
        self.show()

    def move(self, dw, dh, part_name=None, state_name=None):
        if part_name and state_name:
            self._part_and_state_select(part_name, state_name)

        if self._part:
            x, y, w, h = self._geometry
            self._part.pos = (x + dw, y + dh)

    def up(self, dw, dh):
        if not self._part:
            return

        if (dw, dh) != (0, 0):
            op = Operation("part moving")

            args = self._edit_grp.part.name, self._edit_grp.part.state.name

            op.redo_callback_add(self.move, dw, dh, *args)
            op.redo_callback_add(self.part_move, self._part)
            op.redo_callback_add(self._rel1_move_offset_inform_cb, dw, dh)
            op.redo_callback_add(self._rel2_move_offset_inform_cb, dw, dh)

            op.undo_callback_add(self.move, -dw, -dh, *args)
            op.undo_callback_add(self.part_move, self._part)
            op.undo_callback_add(self._rel1_move_offset_inform_cb, -dw, -dh)
            op.undo_callback_add(self._rel2_move_offset_inform_cb, -dw, -dh)
            self._operation_stack_cb(op)

        self._rel1_move_offset_inform_cb(dw, dh)
        self._rel2_move_offset_inform_cb(dw, dh)


class PartHandler_T(PartHandler):
    def part_move(self, obj):
        self.bottom_center = obj.top_center
        if obj.size[0] < 10:
            self.hide()
        else:
            self.show()

    def move(self, dw, dh, part_name=None, state_name=None):
        if part_name:
            self._part_and_state_select(part_name, state_name)

        if self._part:
            x, y, w, h = self._geometry
            self._part.geometry = (x, y + dh, w, h - dh)

    def up(self, dw, dh):
        if not self._part:
            return

        if dh != 0:
            op = Operation("part resing (from top)")

            args = self._edit_grp.part.name, self._edit_grp.part.state.name

            op.redo_callback_add(self.move, 0, dh, *args)
            op.redo_callback_add(self.part_move, self._part)
            op.redo_callback_add(self._rel1_move_offset_inform_cb, 0, dh)

            op.undo_callback_add(self.move, 0, -dh, *args)
            op.undo_callback_add(self.part_move, self._part)
            op.undo_callback_add(self._rel1_move_offset_inform_cb, 0, -dh)
            self._operation_stack_cb(op)

        self._rel1_move_offset_inform_cb(0, dh)


class PartHandler_TL(PartHandler):
    def part_move(self, obj):
        self.show()
        self.bottom_right = obj.top_left

    def move(self, dw, dh, part_name=None, state_name=None):
        if part_name:
            self._part_and_state_select(part_name, state_name)

        if self._part:
            x, y, w, h = self._geometry
            self._part.geometry = (x + dw, y + dh, w - dw, h - dh)

    def up(self, dw, dh):
        if not self._part:
            return

        if (dw, dh) != (0, 0):
            op = Operation("part resing (from top-left)")

            args = self._edit_grp.part.name, self._edit_grp.part.state.name

            op.redo_callback_add(self.move, dw, dh, *args)
            op.redo_callback_add(self.part_move, self._part)
            op.redo_callback_add(self._rel1_move_offset_inform_cb, dw, dh)

            op.undo_callback_add(self.move, -dw, -dh, *args)
            op.undo_callback_add(self.part_move, self._part)
            op.undo_callback_add(self._rel1_move_offset_inform_cb, -dw, -dh)
            self._operation_stack_cb(op)

        self._rel1_move_offset_inform_cb(dw, dh)


class PartHandler_TR(PartHandler):
    def part_move(self, obj):
        self.show()
        self.bottom_left = obj.top_right

    def move(self, dw, dh, part_name=None, state_name=None):
        if part_name:
            self._part_and_state_select(part_name, state_name)

        if self._part:
            x, y, w, h = self._geometry
            self._part.geometry = (x, y + dh, w + dw, h - dh)

    def up(self, dw, dh):
        if not self._part:
            return

        if (dw, dh) != (0, 0):
            op = Operation("part resing (from top-left)")

            args = self._edit_grp.part.name, self._edit_grp.part.state.name

            op.redo_callback_add(self.move, dw, dh, *args)
            op.redo_callback_add(self.part_move, self._part)
            op.redo_callback_add(self._rel1_move_offset_inform_cb, 0, dh)
            op.redo_callback_add(self._rel2_move_offset_inform_cb, dw, 0)

            op.undo_callback_add(self.move, -dw, -dh, *args)
            op.undo_callback_add(self.part_move, self._part)
            op.undo_callback_add(self._rel1_move_offset_inform_cb, 0, -dh)
            op.undo_callback_add(self._rel2_move_offset_inform_cb, -dw, 0)
            self._operation_stack_cb(op)

        self._rel1_move_offset_inform_cb(0, dh)
        self._rel2_move_offset_inform_cb(dw, 0)


class PartHandler_B(PartHandler):
    def part_move(self, obj):
        self.top_center = obj.bottom_center
        if obj.size[0] < 10:
            self.hide()
        else:
            self.show()

    def move(self, dw, dh, part_name=None, state_name=None):
        if part_name:
            self._part_and_state_select(part_name, state_name)

        if self._part:
            x, y, w, h = self._geometry
            self._part.geometry = (x, y, w, h + dh)

    def up(self, dw, dh):
        if not self._part:
            return

        if dh != 0:
            op = Operation("part resing (from bottom)")

            args = self._edit_grp.part.name, self._edit_grp.part.state.name

            op.redo_callback_add(self.move, 0, dh, *args)
            op.redo_callback_add(self.part_move, self._part)
            op.redo_callback_add(self._rel2_move_offset_inform_cb, 0, dh)

            op.undo_callback_add(self.move, 0, -dh, *args)
            op.undo_callback_add(self.part_move, self._part)
            op.undo_callback_add(self._rel2_move_offset_inform_cb, 0, -dh)
            self._operation_stack_cb(op)

        self._rel2_move_offset_inform_cb(0, dh)


class PartHandler_BR(PartHandler):
    def part_move(self, obj):
        self.show()
        self.top_left = obj.bottom_right

    def move(self, dw, dh, part_name=None, state_name=None):
        if part_name:
            self._part_and_state_select(part_name, state_name)

        if self._part:
            x, y, w, h = self._geometry
            self._part.geometry = (x, y, w + dw, h + dh)

    def up(self, dw, dh):
        if not self._part:
            return

        if (dw, dh) != (0, 0):
            op = Operation("part resing (from bottom-right)")

            args = self._edit_grp.part.name, self._edit_grp.part.state.name

            op.redo_callback_add(self.move, dw, dh, *args)
            op.redo_callback_add(self.part_move, self._part)
            op.redo_callback_add(self._rel2_move_offset_inform_cb, dw, dh)

            op.undo_callback_add(self.move, -dw, -dh, *args)
            op.undo_callback_add(self.part_move, self._part)
            op.undo_callback_add(self._rel2_move_offset_inform_cb, -dw, -dh)
            self._operation_stack_cb(op)

        self._rel2_move_offset_inform_cb(dw, dh)


class PartHandler_BL(PartHandler):
    def part_move(self, obj):
        self.show()
        self.top_right = obj.bottom_left

    def move(self, dw, dh, part_name=None, state_name=None):
        if part_name:
            self._part_and_state_select(part_name, state_name)

        if self._part:
            x, y, w, h = self._geometry
            self._part.geometry = (x + dw, y, w - dw, h + dh)

    def up(self, dw, dh):
        if not self._part:
            return

        if (dw, dh) != (0, 0):
            op = Operation("part resing (from bottom-left)")

            args = self._edit_grp.part.name, self._edit_grp.part.state.name

            op.redo_callback_add(self.move, dw, dh, *args)
            op.redo_callback_add(self.part_move, self._part)
            op.redo_callback_add(self._rel1_move_offset_inform_cb, dw, 0)
            op.redo_callback_add(self._rel2_move_offset_inform_cb, 0, dh)

            op.undo_callback_add(self.move, -dw, -dh, *args)
            op.undo_callback_add(self.part_move, self._part)
            op.undo_callback_add(self._rel1_move_offset_inform_cb, -dw, 0)
            op.undo_callback_add(self._rel2_move_offset_inform_cb, 0, -dh)
            self._operation_stack_cb(op)

        self._rel1_move_offset_inform_cb(dw, 0)
        self._rel2_move_offset_inform_cb(0, dh)


class PartHandler_L(PartHandler):
    def part_move(self, obj):
        self.right_center = obj.left_center
        if obj.size[1] < 10:
            self.hide()
        else:
            self.show()

    def move(self, dw, dh, part_name=None, state_name=None):
        if part_name:
            self._part_and_state_select(part_name, state_name)

        if self._part:
            x, y, w, h = self._geometry
            self._part.geometry = (x + dw, y, w - dw, h)

    def up(self, dw, dh):
        if not self._part:
            return

        if dw != 0:
            op = Operation("part resing (from left)")

            args = self._edit_grp.part.name, self._edit_grp.part.state.name

            op.redo_callback_add(self.move, dw, 0, *args)
            op.redo_callback_add(self.part_move, self._part)
            op.redo_callback_add(self._rel1_move_offset_inform_cb, dw, 0)

            op.undo_callback_add(self.move, -dw, 0, *args)
            op.undo_callback_add(self.part_move, self._part)
            op.undo_callback_add(self._rel1_move_offset_inform_cb, -dw, 0)
            self._operation_stack_cb(op)

        self._rel1_move_offset_inform_cb(dw, 0)


class PartHandler_R(PartHandler):
    def part_move(self, obj):
        self.left_center = obj.right_center
        if obj.size[1] < 10:
            self.hide()
        else:
            self.show()

    def move(self, dw, dh, part_name=None, state_name=None):
        if part_name:
            self._part_and_state_select(part_name, state_name)

        if self._part:
            x, y, w, h = self._geometry
            self._part.geometry = (x, y, w + dw, h)

    def up(self, dw, dh):
        if not self._part:
            return

        if dw != 0:
            op = Operation("part resing (from right)")

            args = self._edit_grp.part.name, self._edit_grp.part.state.name

            op.redo_callback_add(self.move, dw, 0, *args)
            op.redo_callback_add(self.part_move, self._part)
            op.redo_callback_add(self._rel2_move_offset_inform_cb, dw, 0)

            op.undo_callback_add(self.move, -dw, 0, *args)
            op.undo_callback_add(self.part_move, self._part)
            op.undo_callback_add(self._rel2_move_offset_inform_cb, -dw, 0)
            self._operation_stack_cb(op)

        self._rel2_move_offset_inform_cb(dw, 0)
