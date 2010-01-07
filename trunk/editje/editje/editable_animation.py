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
from edje.edit import EdjeEdit

from event_manager import Manager
from editable_program import EditableProgram

class EditableAnimation(Manager, object):

    def __init__(self, editable):
        Manager.__init__(self)

        self.e = editable
        self._name = ""
        self.timestops = None

        self._states_init()

        self.program = EditableProgram(self.e)

        self._animation_unselect_cb(self, None)
        self.e.callback_add("group.changed", self._animation_unselect_cb)
        self.e.callback_add("animation.removed", self._animation_removed_cb)


    def _animation_unselect_cb(self, emissor, data):
        self.name = None

    def _animation_removed_cb(self, emissor, data):
        if self._name == data:
            self.name = None

    # Name
    def _name_set(self, value):
        if not self.e._edje:
            value = None

        if self._name != value:
            if value:
                if value in self.e.animations:
                    self._name = value
                    self.event_emit("animation.changed", self._name)
            else:
                self._name = ""
                self.event_emit("animation.unselected")

    def _name_get(self):
        return self._name

    name = property(_name_get, _name_set)

    # Play
    def play(self):
        if self.name:
            self.e._edje.signal_callback_add("animation,end", self._name, self._play_end)
            self.e._edje.program_get(self.program._program.afters_get()[0]).run()

    def _play_end(self, obj, emission, source):
        self.e._edje.signal_callback_del("animation,end", self._name, self._play_end)
        self.state = self.state
        self.event_emit("animation.play.end")

    def stop(self):
        self.e._edje.signal_emit("animation,stop", self._name)

    # States
    def _states_init(self):
        self.timestops = []
        self.callback_add("animation.changed", self._states_reload_cb)
        self.callback_add("animation.unselected", self._states_reload_cb)
        self.callback_add("animation.changed", self._state_reload_cb)
        self.callback_add("animation.unselected", self._state_reload_cb)

    def _states_reload_cb(self, emissor, data):
        self.timestops = []

        if not data:
            return

        p = self.e.program_get("@%s@0.00" % self._name)
        t = p.name[-4:]
        while t != "@end":
            self.timestops.append(float(t))
            p = self.e.program_get(p.afters[0])
            t = p.name[-4:]

        self.event_emit("states.changed", self.timestops)

    def state_add(self, time):
        if not self._name:
            return
        if time < 0.0:
            return

        # Search
        idx = 0
        for t in self.timestops:
            if time == t:
                return idx
            if t > time:
                break
            idx += 1

        # Defines
        prev = self.timestops[idx - 1]
        prevname = "@%s@%.2f" % (self._name, prev)
        name = "@%s@%.2f" % (self._name, time)

        # States
        prevstatename = prevname + " 0.00"
        statename = name + " 0.00"

        # Create
        self.e.program_add(name)
        prog = self.e.program_get(name)
        prog.state_set(name)
        prog.transition = edje.EDJE_TWEEN_MODE_LINEAR
        prog.transition_time = time - prev
        for p in self.e.parts:
            prog.target_add(p)
            part = self.e._edje.part_get(p)
            part.state_add(name)
            state = part.state_get(statename)
            state.copy_from(prevstatename)

        # Link Prev
        prevprog = self.e.program_get(prevname)
        nextname = prevprog.afters[0]
        prog.after_add(nextname)
        prevprog.afters_clear()
        prevprog.after_add(name)

        # Link Next
        next = nextname[-4:]
        if not next == "@end":
            next = float(next)
            nextprog = self.e.program_get(nextname)
            nextprog.transition_time = next - time

        # Stop
        stopname = "@%s@stop" % self._name
        self.e.program_add(stopname)
        prog = self.e.program_get(stopname)
        prog.action = edje.EDJE_ACTION_TYPE_ACTION_STOP
        prog.signal = "animation,stop"
        prog.target_add(name)

        self.timestops.insert(idx, time)
        self.event_emit("state.added", time)

        return idx

    def state_del(self, time):
        if time == 0.0 or time == "end":
            return

        # Search
        idx = self.timestops.index(time)

        # Unlink
        prev = self.timestops[idx - 1]
        prevname = "@%s@%.2f" % (self._name, prev)
        prevprog = self.e.program_get(prevname)
        nextname = prog.afters[0]
        prevprog.afters_clear()
        prevprog.after_add(nextname)

        # Fix Next
        next = nextname[-4:]
        if not next == "@end":
            next = float(next)
            nextprog = self.e.program_get(nextname)
            nextprog.transition_time = next - prev

        self.timestops.pop(idx)
        self.event_emit("state.removed", time)

    def _state_set(self, time):
        if not self._name:
            return
        self._current_idx = self.timestops.index(time)
        self._current = time
        self.program.name = "@%s@%.2f" % (self._name, time)
        if not self.e.part.name:
            self.e.part.name = self.e.parts[0]
        statename = self.program.name + " 0.00"
        self.e.part.state.name = statename
        for p in self.e.parts:
            part = self.e._edje.part_get(p)
            part.state_selected_set(statename)
        self.event_emit("state.changed", self.e.part.state.name)

    def _state_get(self):
        return self._current

    state = property(_state_get, _state_set)

    def state_next(self):
        if not self._name:
            return None
        if self._current_idx == len(self.timestops) - 1:
            return None
        return self.timestops[self._current_idx + 1]

    def state_next_goto(self):
        state = self.state_next()
        if state is not None:
            self.state = state

    def state_prev(self):
        if not self._name:
            return None
        if self._current_idx == 0:
            return None
        return self.timestops[self._current_idx - 1]

    def state_prev_goto(self):
        state = self.state_prev()
        if state is not None:
            self.state = state

    def _state_reload_cb(self, emissor, data):
        if data and self.timestops:
            self.state = 0.0
        else:
            self._current = None
            self._idx = 0
            self._prog = None

    # Info
    def _length_get(self):
        if self.timestops:
            return self.timestops[-1]
        return 0.0

    length = property(_length_get)
