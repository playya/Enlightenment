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

import os

import evas
import elementary

class FileSelector(elementary.Table):

    def __init__(self, parent):
        self._parent = parent
        elementary.Table.__init__(self, parent)
        self.size_hint_align_set(evas.EVAS_HINT_FILL,
                                 evas.EVAS_HINT_FILL)
        self.size_hint_weight_set(evas.EVAS_HINT_EXPAND,
                                  evas.EVAS_HINT_EXPAND)
        self.homogenous_set(False)

        self._filter_call = None
        self._home = os.getenv("HOME")

        self._navigator_init()
        self._files_init()
        self._actions_init()
        self._filter_init()

        self._path = ""
        self._file = ""
        self.path = os.getenv("PWD")
        self.multi = False

    def _navigator_init(self):
        bx = elementary.Box(self._parent)
        bx.size_hint_weight_set(evas.EVAS_HINT_EXPAND,
                                evas.EVAS_HINT_EXPAND)
        bx.size_hint_align_set(evas.EVAS_HINT_FILL,
                               evas.EVAS_HINT_FILL)
        self.pack(bx, 0, 0, 1, 4)
        bx.show()

        self._nav_home = elementary.Button(self._parent)
        self._nav_home.size_hint_weight_set(evas.EVAS_HINT_EXPAND, 0.0)
        self._nav_home.size_hint_align_set(evas.EVAS_HINT_FILL, 0.0)
        self._nav_home.label_set("Home")
        ic = elementary.Icon(self._parent)
        ic.standard_set("home")
        ic.scale_set(0, 0)
        self._nav_home.icon_set(ic)
        self._nav_home.callback_clicked_add(self._home_load)
        bx.pack_end(self._nav_home)
        self._nav_home.show()

        sp = elementary.Separator(self._parent)
        sp.size_hint_align_set(evas.EVAS_HINT_FILL, 0.0)
        sp.size_hint_weight_set(evas.EVAS_HINT_EXPAND, 0.0)
        sp.horizontal_set(True)
        bx.pack_end(sp)
        sp.show()

        self._nav_up = elementary.Button(self._parent)
        self._nav_up.size_hint_weight_set(evas.EVAS_HINT_EXPAND, 0.0)
        self._nav_up.size_hint_align_set(evas.EVAS_HINT_FILL, 0.0)
        self._nav_up.label_set("Up")
        ic = elementary.Icon(self._parent)
        ic.standard_set("arrow_up")
        ic.scale_set(0, 0)
        self._nav_up.icon_set(ic)
        self._nav_up.callback_clicked_add(self._parent_load)
        bx.pack_end(self._nav_up)
        self._nav_up.show()

        self._directories = elementary.List(self._parent)
        self._directories.size_hint_align_set(evas.EVAS_HINT_FILL,
                                            evas.EVAS_HINT_FILL)
        self._directories.size_hint_weight_set(evas.EVAS_HINT_EXPAND,
                                               evas.EVAS_HINT_EXPAND)
        self._directories.callback_selected_add(self._folder_change)
        bx.pack_end(self._directories)
        self._directories.show()

    def _files_init(self):
        bx = elementary.Box(self._parent)
        bx.size_hint_weight_set(evas.EVAS_HINT_EXPAND,
                                evas.EVAS_HINT_EXPAND)
        bx.size_hint_align_set(evas.EVAS_HINT_FILL,
                               evas.EVAS_HINT_FILL)
        self.pack(bx, 1, 0, 3, 4)
        bx.show()

        sc = elementary.Scroller(self._parent)
        sc.content_min_limit(0, 1)
        sc.policy_set(elementary.ELM_SCROLLER_POLICY_OFF,
                      elementary.ELM_SCROLLER_POLICY_OFF)
        sc.bounce_set(False, False)
        sc.size_hint_weight_set(evas.EVAS_HINT_EXPAND, 0.0)
        sc.size_hint_align_set(evas.EVAS_HINT_FILL, evas.EVAS_HINT_FILL)
        bx.pack_end(sc)
        sc.show()

        self._nav_path = elementary.Entry(self._parent)
        self._nav_path.single_line_set(True)
        self._nav_path.size_hint_weight_set(evas.EVAS_HINT_EXPAND,
                                            evas.EVAS_HINT_EXPAND)
        self._nav_path.size_hint_align_set(evas.EVAS_HINT_FILL, 0.5)
        self._nav_path.editable_set(False)
        self._nav_path.entry_set("PATH")
        self._nav_path.callback_anchor_clicked_add(self._path_go)
        self._nav_path.callback_changed_add(self._path_change)
        sc.content_set(self._nav_path)
        self._nav_path.show()

        self._files = elementary.List(self._parent)
        self._files.size_hint_align_set(evas.EVAS_HINT_FILL,
                                        evas.EVAS_HINT_FILL)
        self._files.size_hint_weight_set(evas.EVAS_HINT_EXPAND,
                                         evas.EVAS_HINT_EXPAND)
        bx.pack_end(self._files)
        self._files.show()

    def _multi_set(self, value):
        self._files.multi_select_set(value)
        self._multi = value

    def _multi_get(self):
        return self._multi

    multi = property(_multi_get, _multi_set)

    def _actions_init(self):
        sp = elementary.Separator(self._parent)
        sp.size_hint_align_set(evas.EVAS_HINT_FILL, 0.5)
        sp.size_hint_weight_set(evas.EVAS_HINT_EXPAND, 0.0)
        sp.size_hint_min_set(600, 1)
        sp.horizontal_set(True)
        self.pack(sp, 0, 4, 4, 1)
        sp.show()

        self._actions = elementary.Box(self._parent)
        self._actions.horizontal_set(True)
        self._actions.size_hint_weight_set(evas.EVAS_HINT_EXPAND, 0.0)
        self._actions.size_hint_align_set(1.0, evas.EVAS_HINT_FILL)
        self.pack(self._actions, 3, 5, 1, 1)
        self._actions.show()

    def _filter_init(self):
        bx = elementary.Box(self._parent)
        bx.horizontal_set(True)
        bx.size_hint_weight_set(evas.EVAS_HINT_EXPAND, 0.0)
        bx.size_hint_align_set(evas.EVAS_HINT_FILL,
                               evas.EVAS_HINT_FILL)
        self.pack(bx, 0, 5, 2, 1)
        bx.show()

        self._hidden = elementary.Check(self._parent)
        self._hidden.size_hint_align_set(evas.EVAS_HINT_FILL, 0.5)
        self._hidden.size_hint_weight_set(evas.EVAS_HINT_EXPAND, 0.0)
        self._hidden.state_set(False)
        self._hidden.label_set("Show hidden files")
        self._hidden.callback_changed_add(self._update)
        bx.pack_end(self._hidden)
        self._hidden.show()

        self._filter = elementary.Check(self._parent)
        self._filter.size_hint_align_set(evas.EVAS_HINT_FILL, 0.5)
        self._filter.size_hint_weight_set(evas.EVAS_HINT_EXPAND, 0.0)
        self._filter.label_set("Filter extensions")
        self._filter.state_set(False)
        self._filter.callback_changed_add(self._update)
        bx.pack_end(self._filter)

    def _home_load(self, bt):
        self.path = self._home

    def _parent_load(self, bt):
        head, tail = os.path.split(self._path)
        self.path = head

    def _path_go(self, obj, en):
        return

    def _update(self, obj):
        self._files.clear()
        self._directories.clear()

        hidden = self._hidden.state_get()
        filter = self._filter.state_get()

        path = self.path
        list =os.listdir(path)
        list.sort(key=str.lower)
        for file in list:
            if hidden or not file.startswith("."):
                full = os.path.join(path, file)
                if os.path.isdir(full):
                    ic = elementary.Icon(self._parent)
                    ic.standard_set("folder")
                    ic.scale_set(0, 0)
                    self._directories.item_append(file, ic, None, None, full)
                elif os.path.isfile(full):
                    if not filter or self._filter_call(full):
                        ic = elementary.Icon(self._parent)
                        ic.standard_set("file")
                        ic.scale_set(0, 0)
                        item = self._files.item_append(file, ic, None, None,
                                                       full)
                        if full == self.file:
                            item.selected_set(True)

        self._files.go()
        self._directories.go()

    def _folder_change(self, li, id):
        self.path = li.selected_item_get().data_get()[0][0]

    def _path_change(self, en):
        self.path = self._nav_path.entry_get()

    # PATH
    def _path_set(self, path):
        if path == self._path:
            return

        if os.path.isdir(path):
            self._path = path
            self.file = ""
            self._nav_path.entry_set(self._path)
            self._update(self)

    def _path_get(self):
        return self._path

    path = property(_path_get, _path_set)

    def _file_set(self, file):
        if file == self._file:
            return

        if os.path.isfile(file):
            self._file = file
            self.path = os.path.dirname(file)
        else:
            self._file = ""

    def _file_get(self):
        item =self._files.selected_item_get()
        if item:
            return item.data_get()[0][0]
        return ""

    file = property(_file_get, _file_set)

    def _files_get(self):
        ret = []
        for i in self._files.selected_items_get():
            ret.append(i.data_get()[0][0])
        return ret

    files = property(_files_get)

    def _filter_set(self, filter):
        if filter and self._filter_call != filter:
            self._filter.state_set(True)
            self._filter_call = filter
            self._filter.show()
            self._update(self)
        elif not filter and self._filter_call:
            self._filter.state_set(False)
            self._filter_call = None
            self._filter.hide()
            self._update(self)

    filter = property(fset=_filter_set)

    def action_add(self, label, func_cb, data=None, icon=None):
        btn = elementary.Button(self._parent)
        btn.size_hint_weight_set(evas.EVAS_HINT_EXPAND, 0.0)
        btn.size_hint_align_set(evas.EVAS_HINT_FILL, 0.0)
        btn.label_set(label)

        if func_cb:
            btn.callback_clicked_add(func_cb)
            btn.data["clicked"] = data

        if icon:
            ico = elementary.Icon(self._parent)
            ico.file_set(self.__theme_file, "editje/icon/" + icon)
            btn.icon_set(ico)
            ico.show()

        btn.show()
        self._actions.pack_end(btn)


if __name__ == "__main__":
    elementary.init()
    elementary.policy_set(elementary.ELM_POLICY_QUIT,
                          elementary.ELM_POLICY_QUIT_LAST_WINDOW_CLOSED)
    win = elementary.Window("fileselector", elementary.ELM_WIN_BASIC)
    win.title_set("FileSelector")
    win.autodel_set(True)
    win.resize(600, 480)
    win.maximized_set(True)

    bg = elementary.Background(win)
    win.resize_object_add(bg)
    bg.size_hint_weight_set(evas.EVAS_HINT_EXPAND, evas.EVAS_HINT_EXPAND)
    bg.show()

    fs = FileSelector(win)
    win.resize_object_add(fs)
    def filter(file):
        return file.endswith(".edj")
    fs.filter = filter
    fs.action_add("Ok", None)
    fs.action_add("Cancel", None)
    fs.show()

    win.show()

    elementary.run()
    elementary.shutdown()
