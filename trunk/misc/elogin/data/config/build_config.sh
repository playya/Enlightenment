#!/bin/sh -e
DB="./elogin_config.db"
rm $DB
edb_ed $DB add /elogin/welcome/mess str "Enter Your Username"
edb_ed $DB add /elogin/welcome/font/name str "notepad.ttf"
edb_ed $DB add /elogin/welcome/font/size int 20
edb_ed $DB add /elogin/welcome/font/r int 192
edb_ed $DB add /elogin/welcome/font/g int 192
edb_ed $DB add /elogin/welcome/font/b int 192
edb_ed $DB add /elogin/welcome/font/a int 220
edb_ed $DB add /elogin/passwd/mess str "Enter Your Password..."
edb_ed $DB add /elogin/passwd/font/name str "notepad.ttf"
edb_ed $DB add /elogin/passwd/font/size int 20
edb_ed $DB add /elogin/passwd/font/r int 192
edb_ed $DB add /elogin/passwd/font/g int 192
edb_ed $DB add /elogin/passwd/font/b int 192
edb_ed $DB add /elogin/passwd/font/a int 220
edb_ed $DB add /elogin/xinerama/screens/w int 1
edb_ed $DB add /elogin/xinerama/screens/h int 1
edb_ed $DB add /elogin/xinerama/on/w int 1
edb_ed $DB add /elogin/xinerama/on/h int 1
edb_ed $DB add /elogin/session/0/name str "E17"
edb_ed $DB add /elogin/session/0/path str "/usr/local/e17/bin/enlightenment"
edb_ed $DB add /elogin/session/1/name str "Enlightenment"
edb_ed $DB add /elogin/session/1/path str "/usr/bin/enlightenment"
edb_ed $DB add /elogin/session/2/name str "KDE"
edb_ed $DB add /elogin/session/2/path str "/usr/bin/kde"
edb_ed $DB add /elogin/session/3/name str "GNOME"
edb_ed $DB add /elogin/session/3/path str "/usr/bin/gnome-session"
edb_ed $DB add /elogin/session/4/name str "Blackbox"
edb_ed $DB add /elogin/session/4/path str "/usr/bin/blackbox"
edb_ed $DB add /elogin/session/5/name str "Sawfish"
edb_ed $DB add /elogin/session/5/path str "/usr/bin/sawfish"
edb_ed $DB add /elogin/session/6/name str "Failsafe"
edb_ed $DB add /elogin/session/6/path str "failsafe"
edb_ed $DB add /elogin/session/count int 7
