# Copyright (C) 2007-2008 Caio Marcelo de Oliveira Filho
#
# This file is part of Python-Etk.
#
# Python-Etk is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# Python-Etk is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this Python-Etk.  If not, see <http://www.gnu.org/licenses/>.

cdef extern from "etk_separator.h":
    ####################################################################
    # Signals

    ####################################################################
    # Enumerations
    ####################################################################
    # Structures
    ctypedef struct Etk_VSeparator
    ctypedef struct Etk_HSeparator
    ctypedef struct Etk_Separator

    ####################################################################
    # Functions
    Etk_Type* etk_hseparator_type_get()
    Etk_Type* etk_separator_type_get()
    Etk_Type* etk_vseparator_type_get()

    Etk_Widget *etk_hseparator_new()
    Etk_Widget *etk_vseparator_new()

#########################################################################
# Objects
cdef public class Separator(Widget) [object PyEtk_Separator, type PyEtk_Separator_Type]:
    pass

cdef public class VSeparator(Separator) [object PyEtk_VSeparator, type PyEtk_VSeparator_Type]:
    pass

cdef public class HSeparator(Separator) [object PyEtk_HSeparator, type PyEtk_HSeparator_Type]:
    pass
