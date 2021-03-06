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

cdef extern from "etk_signal.h":
    ####################################################################
    # Enumerations
    ####################################################################
    # Structures
    ctypedef struct Etk_Signal
    ctypedef struct Etk_Signal_Connect_Desc

    ####################################################################
    # Functions
    Etk_Signal_Callback *etk_signal_connect_full_by_code(int signal_code,
        Etk_Object *object, Etk_Callback callback, void *data,
        Etk_Bool swapped, Etk_Bool after)
    Etk_Signal_Callback *etk_signal_connect_full_by_name(
        char *signal_name, Etk_Object *object, Etk_Callback callback,
        void *data, Etk_Bool swapped, Etk_Bool after)

    Etk_Signal_Callback *etk_signal_connect_by_code(int signal_code,
        Etk_Object *object, Etk_Callback callback, void *data)
    Etk_Signal_Callback *etk_signal_connect(char *signal_name,
        Etk_Object *object, Etk_Callback callback, void *data)
    Etk_Signal_Callback *etk_signal_connect_after_by_code(
        int signal_code, Etk_Object *object, Etk_Callback callback, void *data)
    Etk_Signal_Callback *etk_signal_connect_after(char *signal_name,
        Etk_Object *object, Etk_Callback callback, void *data)
    Etk_Signal_Callback *etk_signal_connect_swapped_by_code(
        int signal_code, Etk_Object *object, Etk_Callback callback, void *data)
    Etk_Signal_Callback *etk_signal_connect_swapped(
        char *signal_name, Etk_Object *object, Etk_Callback callback,
        void *data)

    void etk_signal_connect_multiple(Etk_Signal_Connect_Desc *desc,
                                     Etk_Object *object, void *data)

    void etk_signal_disconnect_by_code(int signal_code, Etk_Object *object,
                                       Etk_Callback callback, void *data)
    void etk_signal_disconnect(char *signal_name, Etk_Object *object,
                               Etk_Callback callback, void *data)
    void etk_signal_disconnect_scb_by_code(int signal_code, Etk_Object *object,
                                           Etk_Signal_Callback *scb)
    void etk_signal_disconnect_scb(char *signal_name, Etk_Object *object,
                                   Etk_Signal_Callback *scb)
    void etk_signal_disconnect_all_by_code(int signal_code, Etk_Object *object)
    void etk_signal_disconnect_all(char *signal_name, Etk_Object *object)

    void etk_signal_disconnect_multiple(Etk_Signal_Connect_Desc *desc,
                                        Etk_Object *object)

    void etk_signal_block_by_code(int signal_code, Etk_Object *object,
                                  Etk_Callback callback, void *data)
    void etk_signal_block(char *signal_name, Etk_Object *object,
                          Etk_Callback callback, void *data)
    void etk_signal_block_scb_by_code(int signal_code, Etk_Object *object,
                                      Etk_Signal_Callback *scb)
    void etk_signal_block_scb(char *signal_name, Etk_Object *object,
                              Etk_Signal_Callback *scb)

    void etk_signal_unblock_by_code(int signal_code, Etk_Object *object,
                                    Etk_Callback callback, void *data)
    void etk_signal_unblock(char *signal_name, Etk_Object *object,
                            Etk_Callback callback, void *data)
    void etk_signal_unblock_scb_by_code(int signal_code, Etk_Object *object,
                                        Etk_Signal_Callback *scb)
    void etk_signal_unblock_scb(char *signal_name, Etk_Object *object,
                                Etk_Signal_Callback *scb)
