#!/bin/sh

# Specify the default theme db
DB="./theme.db"

echo -n "Regenerating default $DB"

# Add keys for boxes
echo -n "."
edb_ed $DB add "/box/base" str "appearance/box_vertical.bits.db"
edb_ed $DB add "/box/base/visible" str "no"

# Add keys for buttons
echo -n "."
edb_ed $DB add "/button/base" str "appearance/button.bits.db"
edb_ed $DB add "/button/base/visible" str "yes"
edb_ed $DB add "/button/label/font" str "nationff"
edb_ed $DB add "/button/label/font_size" int 12
edb_ed $DB add "/button/label/style" str "shadow"

edb_ed $DB add "/check/base" str "appearance/check.bits.db"
edb_ed $DB add "/check/base/visible" str "yes"

edb_ed $DB add "/button/check/base" str "appearance/checkbutton.bits.db"
edb_ed $DB add "/button/check/base/visible" str "no"

edb_ed $DB add "/button/check/label/font" str "nationff"
edb_ed $DB add "/button/check/label/font_size" int 12
edb_ed $DB add "/button/check/label/style" str "shadow"

edb_ed $DB add "/radio/base" str "appearance/radio.bits.db"
edb_ed $DB add "/radio/base/visible" str "yes"

edb_ed $DB add "/button/radio/base" str "appearance/radio.bits.db"
edb_ed $DB add "/button/radio/base/visible" str "yes"

edb_ed $DB add "/button/radio/label/font" str "nationff"
edb_ed $DB add "/button/radio/label/font_size" int 12
edb_ed $DB add "/button/radio/label/style" str "shadow"

# Add keys for text entry widgets
echo -n "."
edb_ed $DB add "/cursor/base" str "appearance/cursor.bits.db"
edb_ed $DB add "/cursor/base/visible" str "yes"
edb_ed $DB add "/selection/base" str "appearance/separator.bits.db"
edb_ed $DB add "/selection/base/visible" str "yes"

edb_ed $DB add "/entry/base" str "appearance/entry.bits.db"
edb_ed $DB add "/entry/base/visible" str "yes"
edb_ed $DB add "/entry/text/font" str "nationff"
edb_ed $DB add "/entry/text/font_size" int 12
edb_ed $DB add "/entry/text/style" str "shadow"

# Add keys for image widget
echo -n "."
edb_ed $DB add "/image/base" str "appearance/image.bits.db"
edb_ed $DB add "/image/base/visible" str "no"

# Add keys for list widget
echo -n "."
edb_ed $DB add "/list/base" str "appearance/list.bits.db"
edb_ed $DB add "/list/base/visible" str "yes"
edb_ed $DB add "/list/marker/base" str "appearance/list_marker.bits.db"
edb_ed $DB add "/list/marker/base/visible" str "yes"

# Add keys for notebook widget
echo -n "."
edb_ed $DB add "/notebook/base" str "appearance/notebook.bits.db"
edb_ed $DB add "/notebook/base/visible" str "yes"
edb_ed $DB add "/notebook/content_box/base/visible" str "no"
edb_ed $DB add "/notebook/tab_box/base/visible" str "no"
edb_ed $DB add "/notebook/tab_button/base" str "appearance/notebook/tab_button/base-top.bits.db"
edb_ed $DB add "/notebook/tab_button/base/visible" str "yes"
edb_ed $DB add "/notebook/tab_button/label/font" str "nationff"
edb_ed $DB add "/notebook/tab_button/label/font_size" int 12
edb_ed $DB add "/notebook/tab_button/label/style" str "shadow"


# Add keys for seeker widget
echo -n "."
edb_ed $DB add "/seeker/horizontal/base" str "appearance/seeker-horizontal.bits.db"
edb_ed $DB add "/seeker/horizontal/base/visible" str "yes"
edb_ed $DB add "/seeker/horizontal/dragbar/base" str "appearance/seeker_drag-horizontal.bits.db"
edb_ed $DB add "/seeker/horizontal/dragbar/base/visible" str "yes"

edb_ed $DB add "/seeker/vertical/base" str "appearance/seeker-vertical.bits.db"
edb_ed $DB add "/seeker/vertical/base/visible" str "yes"
edb_ed $DB add "/seeker/vertical/dragbar/base" str "appearance/seeker_drag-vertical.bits.db"
edb_ed $DB add "/seeker/vertical/dragbar/base/visible" str "yes"

# Add keys for scrollbar widget
echo -n "."
edb_ed $DB add "/scrollbar/horizontal/base" str "appearance/scrollbar-horizontal.bits.db"
edb_ed $DB add "/scrollbar/horizontal/base/visible" str "yes"
edb_ed $DB add "/scrollbar/horizontal/dragbar/base" str "appearance/scrollbar_drag-horizontal.bits.db"
edb_ed $DB add "/scrollbar/horizontal/dragbar/base/visible" str "yes"
edb_ed $DB add "/scrollbar/horizontal/button_increment/base" str "appearance/scrollbar_inc-horizontal.bits.db"
edb_ed $DB add "/scrollbar/horizontal/button_increment/base/visible" str "yes"
edb_ed $DB add "/scrollbar/horizontal/button_decrement/base" str "appearance/scrollbar_dec-horizontal.bits.db"
edb_ed $DB add "/scrollbar/horizontal/button_decrement/base/visible" str "yes"

edb_ed $DB add "/scrollbar/vertical/base" str "appearance/scrollbar-vertical.bits.db"
edb_ed $DB add "/scrollbar/vertical/base/visible" str "yes"
edb_ed $DB add "/scrollbar/vertical/dragbar/base" str "appearance/scrollbar_drag-vertical.bits.db"
edb_ed $DB add "/scrollbar/vertical/dragbar/base/visible" str "yes"
edb_ed $DB add "/scrollbar/vertical/button_increment/base" str "appearance/scrollbar_inc-vertical.bits.db"
edb_ed $DB add "/scrollbar/vertical/button_increment/base/visible" str "yes"
edb_ed $DB add "/scrollbar/vertical/button_decrement/base" str "appearance/scrollbar_dec-vertical.bits.db"
edb_ed $DB add "/scrollbar/vertical/button_decrement/base/visible" str "yes"

# Add keys for separator widget
echo -n "."
edb_ed $DB add "/separator/base" str "appearance/separator.bits.db"
edb_ed $DB add "/separator/base/visible" str "yes"

edb_ed $DB add "/separator/vertical/base" str "appearance/separator.bits.db"
edb_ed $DB add "/separator/vertical/base/visible" str "yes"

# Add keys for table widget
echo -n "."
edb_ed $DB add "/table/base/visible" str "no"

# Add keys for text widget
echo -n "."
edb_ed $DB add "/text/font" str "nationff"
edb_ed $DB add "/text/font_size" int 12
edb_ed $DB add "/text/style" str "shadow"

# Add keys for the textarea widget
echo -n "."
edb_ed $DB add "/textarea/base" str "appearance/textarea.bits.db"
edb_ed $DB add "/textarea/base/visible" str "yes"
edb_ed $DB add "/textarea/base/style" str "shadow"
edb_ed $DB add "/textarea/base/r" int 255
edb_ed $DB add "/textarea/base/g" int 255
edb_ed $DB add "/textarea/base/b" int 255
edb_ed $DB add "/textarea/base/a" int 255


# Add keys for window widget
echo -n "."
edb_ed $DB add "/window/base" str "appearance/window.bits.db"
edb_ed $DB add "/window/base/visible" str "yes"

# Add keys for floater widget
echo -n "."
edb_ed $DB add "/floater/base" str "appearance/separator.bits.db"
edb_ed $DB add "/floater/base/visible" str "yes"

# Setup author, licence and theme name.
echo -n "."
edb_ed $DB add "/theme/author" str "Nathan 'RbdPngn' Ingersoll"
edb_ed $DB add "/theme/license" str "BSD w/ Advertising Clause"
edb_ed $DB add "/theme/name" str "Default"

echo "Done"
echo "Theme database generation complete."
