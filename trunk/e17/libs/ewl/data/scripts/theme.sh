#!/bin/sh

# Specify the default theme db
DB="./theme.db"

echo -n "Regenerating default theme.db"

# Add keys for boxes
echo -n "."
edb_ed $DB add "/appearance/box/horizontal/base" str "/appearance/button/default/base.bits.db"
edb_ed $DB add "/appearance/box/horizontal/base/visible" str "no"
edb_ed $DB add "/appearance/box/vertical/base" str "/appearance/button/default/base.bits.db"
edb_ed $DB add "/appearance/box/vertical/base/visible" str "no"

# Add keys for buttons
echo -n "."
edb_ed $DB add "/appearance/button/default/base" str "/appearance/button/default/base.bits.db"
edb_ed $DB add "/appearance/button/default/base/visible" str "yes"
edb_ed $DB add "/appearance/button/default/label/font" str "borzoib"
edb_ed $DB add "/appearance/button/default/label/font_size" int 8
edb_ed $DB add "/appearance/button/default/label/style" str "Default"
edb_ed $DB add "/appearance/button/check/base" str "/appearance/button/check/base.bits.db"
edb_ed $DB add "/appearance/button/check/base/visible" str "yes"
edb_ed $DB add "/appearance/button/check/label/font" str "borzoib"
edb_ed $DB add "/appearance/button/check/label/font_size" int 8
edb_ed $DB add "/appearance/button/check/label/style" str "Default"

edb_ed $DB add "/appearance/button/radio/base" str "/appearance/button/radio/base.bits.db"
edb_ed $DB add "/appearance/button/radio/base/visible" str "yes"
edb_ed $DB add "/appearance/button/radio/label/font" str "borzoib"
edb_ed $DB add "/appearance/button/radio/label/font_size" int 8
edb_ed $DB add "/appearance/button/radio/label/style" str "Default"

# Add keys for text entry widgets
echo -n "."
edb_ed $DB add "/appearance/cursor/default/base" str "/appearance/cursor/default/base.bits.db"
edb_ed $DB add "/appearance/cursor/default/base/visible" str "yes"
edb_ed $DB add "/appearance/selection/default/base" str "/appearance/selection/default/base.bits.db"
edb_ed $DB add "/appearance/selection/default/base/visible" str "yes"

edb_ed $DB add "/appearance/entry/default/base" str "/appearance/entry/default/base.bits.db"
edb_ed $DB add "/appearance/entry/default/base/visible" str "yes"
edb_ed $DB add "/appearance/entry/default/text/font" str "borzoib"
edb_ed $DB add "/appearance/entry/default/text/font_size" int 10
edb_ed $DB add "/appearance/entry/default/text/style" str "Default"

# Add keys for image widget
echo -n "."
edb_ed $DB add "/appearance/image/default/404" str "/appearance/image/default/404.bits.db"
edb_ed $DB add "/appearance/image/default/404/visible" str "yes"

# Add keys for list widget
echo -n "."
edb_ed $DB add "/appearance/list/default/base" str "/appearance/list/default/base.bits.db"
edb_ed $DB add "/appearance/list/default/base/visible" str "yes"
edb_ed $DB add "/appearance/list/marker/base" str "/appearance/list/marker/base.bits.db"
edb_ed $DB add "/appearance/list/marker/base/visible" str "yes"

# Add keys for notebook widget
echo -n "."
edb_ed $DB add "/appearance/notebook/default/base" str "/appearance/notebook/default/base.bits.db"
edb_ed $DB add "/appearance/notebook/default/base/visible" str "yes"
edb_ed $DB add "/appearance/notebook/content_box/base" str "/appearance/notebook/content_box/base.bits.db"
edb_ed $DB add "/appearance/notebook/content_box/base/visible" str "no"
edb_ed $DB add "/appearance/notebook/tab_box/base" str "/appearance/notebook/tab_box/base.bits.db"
edb_ed $DB add "/appearance/notebook/tab_box/base/visible" str "no"
edb_ed $DB add "/appearance/notebook/tab_button/base" str "/appearance/notebook/tab_button/base-top.bits.db"
edb_ed $DB add "/appearance/notebook/tab_button/base/visible" str "yes"
edb_ed $DB add "/appearance/notebook/tab_button/label/font" str "borzoib"
edb_ed $DB add "/appearance/notebook/tab_button/label/font_size" int 8
edb_ed $DB add "/appearance/notebook/tab_button/label/style" str "Default"


# Add keys for seeker widget
echo -n "."
edb_ed $DB add "/appearance/seeker/horizontal/base" str "/appearance/seeker/horizontal/base.bits.db"
edb_ed $DB add "/appearance/seeker/horizontal/base/visible" str "yes"
edb_ed $DB add "/appearance/seeker/horizontal/dragbar/base" str "/appearance/seeker/horizontal/dragbar/base.bits.db"
edb_ed $DB add "/appearance/seeker/horizontal/dragbar/base/visible" str "yes"

edb_ed $DB add "/appearance/seeker/vertical/base" str "/appearance/seeker/vertical/base.bits.db"
edb_ed $DB add "/appearance/seeker/vertical/base/visible" str "yes"
edb_ed $DB add "/appearance/seeker/vertical/dragbar/base" str "/appearance/seeker/vertical/dragbar/base.bits.db"
edb_ed $DB add "/appearance/seeker/vertical/dragbar/base/visible" str "yes"

# Add keys for scrollbar widget
echo -n "."
edb_ed $DB add "/appearance/scrollbar/horizontal/base" str "/appearance/scrollbar/horizontal/base.bits.db"
edb_ed $DB add "/appearance/scrollbar/horizontal/base/visible" str "yes"
edb_ed $DB add "/appearance/scrollbar/horizontal/dragbar/base" str "/appearance/scrollbar/horizontal/dragbar/base.bits.db"
edb_ed $DB add "/appearance/scrollbar/horizontal/dragbar/base/visible" str "yes"
edb_ed $DB add "/appearance/scrollbar/horizontal/button_increment/base" str "/appearance/scrollbar/horizontal/button_increment/base.bits.db"
edb_ed $DB add "/appearance/scrollbar/horizontal/button_increment/base/visible" str "yes"
edb_ed $DB add "/appearance/scrollbar/horizontal/button_decrement/base" str "/appearance/scrollbar/horizontal/button_decrement/base.bits.db"
edb_ed $DB add "/appearance/scrollbar/horizontal/button_decrement/base/visible" str "yes"

edb_ed $DB add "/appearance/scrollbar/vertical/base" str "/appearance/scrollbar/vertical/base.bits.db"
edb_ed $DB add "/appearance/scrollbar/vertical/base/visible" str "yes"
edb_ed $DB add "/appearance/scrollbar/vertical/dragbar/base" str "/appearance/scrollbar/vertical/dragbar/base.bits.db"
edb_ed $DB add "/appearance/scrollbar/vertical/dragbar/base/visible" str "yes"
edb_ed $DB add "/appearance/scrollbar/vertical/button_increment/base" str "/appearance/scrollbar/vertical/button_increment/base.bits.db"
edb_ed $DB add "/appearance/scrollbar/vertical/button_increment/base/visible" str "yes"
edb_ed $DB add "/appearance/scrollbar/vertical/button_decrement/base" str "/appearance/scrollbar/vertical/button_decrement/base.bits.db"
edb_ed $DB add "/appearance/scrollbar/vertical/button_decrement/base/visible" str "yes"

# Add keys for separator widget
echo -n "."
edb_ed $DB add "/appearance/separator/horizontal/base" str "/appearance/separator/horizontal/base.bits.db"
edb_ed $DB add "/appearance/separator/horizontal/base/visible" str "yes"

edb_ed $DB add "/appearance/separator/vertical/base" str "/appearance/separator/vertical/base.bits.db"
edb_ed $DB add "/appearance/separator/vertical/base/visible" str "yes"

# Add keys for table widget
echo -n "."
edb_ed $DB add "/appearance/table/default/base" str "/appearance/button/default/base.bits.db"
edb_ed $DB add "/appearance/table/default/base/visible" str "no"

# Add keys for text widget
echo -n "."
edb_ed $DB add "/appearance/text/default/font" str "borzoib"
edb_ed $DB add "/appearance/text/default/font_size" int 10
edb_ed $DB add "/appearance/text/default/style" str "Default"

# Add keys for window widget
echo -n "."
edb_ed $DB add "/appearance/window/default/base" str "/appearance/window/default/base.bits.db"
edb_ed $DB add "/appearance/window/default/base/visible" str "yes"

# Setup author, licence and theme name.
echo -n "."
edb_ed $DB add "/theme/author" str "Christopher 'smugg' Rosendahl and Nathan 'RbdPngn' Ingersoll"
edb_ed $DB add "/theme/licence" str "General Public Licence"
edb_ed $DB add "/theme/name" str "Default"

echo
echo "Theme regeneration complete."
