//$Id: dndwindow.h,v 1.3 2004/01/29 20:47:33 murrayc Exp $ -*- c++ -*-

/* gtkmm example Copyright (C) 2002 gtkmm development team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef TEST_DND_WINDOW_H
#define TEST_DND_WINDOW_H

#include <gtkmm/label.h>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>

#include "TestWindow.h"

class TestDnDWindow : public TestWindow
{

public:
  TestDnDWindow (const std::string &helpImage);
  virtual ~TestDnDWindow ();

protected:
  //Signal handlers:
  virtual void on_button_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& context, Gtk::SelectionData& selection_data, guint info, guint time);
  virtual void on_label_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time);

  //Member widgets:
  Gtk::HBox m_HBox;
  Gtk::Button m_Button_Drag;
  Gtk::Label m_Label_Drop;
};

#endif // TEST_DND_WINDOW_H
