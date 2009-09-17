# Copyright (C) 2007-2008 Gustavo Sverzut Barbieri, Ulisses Furquim
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

cimport evas.python as python
cimport evas.c_evas
import evas.c_evas
cimport edje.c_edje
import edje.c_edje
cimport edje.edit.c_edit
import edje.edit.c_edit

__extra_epydoc_fields__ = (
    ("parm", "Parameter", "Parameters"), # epydoc don't support pyrex properly
    )

include "c_edit_object.pxi"
include "c_edit_object_group.pxi"
include "c_edit_object_data.pxi"
include "c_edit_object_color_class.pxi" #todo(2)
include "c_edit_object_text_style.pxi"
include "c_edit_object_part.pxi" #todo(N)
include "c_edit_object_state.pxi" #todo(fill,color_class,properties?)



