#!/bin/sh -e
DB="./entrance_config.db"
rm -f $DB

# set auth to 1 for pam, 2 for shadow
edb_ed $DB add /entrance/auth int 1
#edb_ed $DB add /entrance/xinerama/screens/w int 1
#edb_ed $DB add /entrance/xinerama/screens/h int 1
#edb_ed $DB add /entrance/xinerama/on/w int 1
#edb_ed $DB add /entrance/xinerama/on/h int 1
edb_ed $DB add /entrance/theme str "default.eet"
edb_ed $DB add /entrance/date_format str "%A %B %e, %Y"
edb_ed $DB add /entrance/time_format str "%l:%M:%S %p"
#edb_ed $DB add /entrance/fonts/count int 2
#edb_ed $DB add /entrance/fonts/0/str str "/usr/share/fonts/truetype/"
#edb_ed $DB add /entrance/fonts/1/str str "/usr/X11R6/lib/X11/fonts/Truetype/"
edb_ed $DB add /entrance/session/count int 8
edb_ed $DB add /entrance/session/0/session str "default"
edb_ed $DB add /entrance/session/0/title str "Default"
edb_ed $DB add /entrance/session/0/icon str "default.png"
edb_ed $DB add /entrance/session/1/icon str "e.eet"
edb_ed $DB add /entrance/session/1/title str "Enlightenment"
edb_ed $DB add /entrance/session/1/session str "Enlightenment"
edb_ed $DB add /entrance/session/2/session str "kde"
edb_ed $DB add /entrance/session/2/title str "KDE"
edb_ed $DB add /entrance/session/2/icon str "kde.png"
edb_ed $DB add /entrance/session/3/session str "gnome"
edb_ed $DB add /entrance/session/3/title str "Gnome"
edb_ed $DB add /entrance/session/3/icon str "gnome.png"
edb_ed $DB add /entrance/session/4/session str "blackbox"
edb_ed $DB add /entrance/session/4/title str "Blackbox"
edb_ed $DB add /entrance/session/4/icon str "blackbox.png"
edb_ed $DB add /entrance/session/5/session str "sawfish"
edb_ed $DB add /entrance/session/5/title str "Sawfish"
edb_ed $DB add /entrance/session/5/icon str "default.png"
edb_ed $DB add /entrance/session/6/session str "xfce"
edb_ed $DB add /entrance/session/6/title str "XFce"
edb_ed $DB add /entrance/session/6/icon str "default.png"
edb_ed $DB add /entrance/session/7/session str "failsafe"
edb_ed $DB add /entrance/session/7/title str "Failsafe"
edb_ed $DB add /entrance/session/7/icon str "failsafe.png"
edb_ed $DB add /entrance/system/reboot int 1
edb_ed $DB add /entrance/system/halt int 1
#edb_ed $DB add /entrance/user/count int 1
#edb_ed $DB add /entrance/user/0/edje str "atmos.eet"
#edb_ed $DB add /entrance/user/0/user str "atmos"
#edb_ed $DB add /entrance/engine str "gl"
