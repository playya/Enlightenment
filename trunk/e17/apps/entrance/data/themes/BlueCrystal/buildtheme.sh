#!/bin/sh -e
DB="./BlueCrystal.db"
rm -f $DB

edb_ed $DB add /entrance/welcome/font/name str "bedrock.ttf"
edb_ed $DB add /entrance/welcome/font/size int 20
edb_ed $DB add /entrance/welcome/font/style str "raised"
edb_ed $DB add /entrance/welcome/color str "#FFFFFFD8"
edb_ed $DB add /entrance/welcome/pos str "0.58:0.8"
edb_ed $DB add /entrance/welcome/offset str "0:0"

edb_ed $DB add /entrance/passwd/font/name str "bedrock.ttf"
edb_ed $DB add /entrance/passwd/font/size int 20
edb_ed $DB add /entrance/passwd/font/style str "raised"
edb_ed $DB add /entrance/passwd/color str "#FFFFFFD8"

edb_ed $DB add /entrance/entry/font/name str "notepad.ttf"
edb_ed $DB add /entrance/entry/font/size int 18
edb_ed $DB add /entrance/entry/font/style "normal"
edb_ed $DB add /entrance/entry/pos str "0.58:0.8"
edb_ed $DB add /entrance/entry/offset str "5:45"
edb_ed $DB add /entrance/entry/color str "#004020FF"
edb_ed $DB add /entrance/entry/box/pos str "0.58:0.8"
edb_ed $DB add /entrance/entry/box/offset str "0:35"
edb_ed $DB add /entrance/entry/box/size str "300:42"
edb_ed $DB add /entrance/entry/box/color str "#FFFFFF60"

edb_ed $DB add /entrance/sessions/selected/font/name str "casual.ttf"
edb_ed $DB add /entrance/sessions/selected/font/size int 24
edb_ed $DB add /entrance/sessions/selected/font/style str "shadow"
edb_ed $DB add /entrance/sessions/selected/icon/size str "48:48"
edb_ed $DB add /entrance/sessions/selected/icon/pos str "0.58:0.1"
edb_ed $DB add /entrance/sessions/selected/icon/offset str "0:0"
edb_ed $DB add /entrance/sessions/selected/text/pos str "0.58:0.1"
edb_ed $DB add /entrance/sessions/selected/text/offset str "58:6"
edb_ed $DB add /entrance/sessions/selected/text/color str "#FFFFFFFF"
edb_ed $DB add /entrance/sessions/selected/text/hicolor str "#C0FFC0FF"

edb_ed $DB add /entrance/sessions/list/text/font/name str "casual.ttf"
edb_ed $DB add /entrance/sessions/list/text/font/size int 16
edb_ed $DB add /entrance/sessions/list/text/font/style str "outline"
edb_ed $DB add /entrance/sessions/list/text/color str "#00000080"
edb_ed $DB add /entrance/sessions/list/seltext/font/name str "casual.ttf"
edb_ed $DB add /entrance/sessions/list/seltext/font/size str 16
edb_ed $DB add /entrance/sessions/list/seltext/font/style str "shadow"
edb_ed $DB add /entrance/sessions/list/seltext/color str "#000080FF"
edb_ed $DB add /entrance/sessions/list/box/color str "#C0D8FF80"
edb_ed $DB add /entrance/sessions/list/box/size str "180:200"
edb_ed $DB add /entrance/sessions/list/box/pos str "0.58:0.1"
edb_ed $DB add /entrance/sessions/list/box/offset str "58:58"

edb_ed $DB add /entrance/hostname/font/name str "casual.ttf"
edb_ed $DB add /entrance/hostname/font/size int 24
edb_ed $DB add /entrance/hostname/font/style str "shadow"
edb_ed $DB add /entrance/hostname/pos str "0.1:0.1"
edb_ed $DB add /entrance/hostname/offset str "0:0"
edb_ed $DB add /entrance/hostname/color str "#C0FFFFFF"

edb_ed $DB add /entrance/date/font/name str "casual.ttf"
edb_ed $DB add /entrance/date/font/size int 14
edb_ed $DB add /entrance/date/font/style str "normal"
edb_ed $DB add /entrance/date/pos str "0.1:0.1"
edb_ed $DB add /entrance/date/offset str "0:40"
edb_ed $DB add /entrance/date/color str "#D8FFFFFF"

edb_ed $DB add /entrance/time/font/name str "casual.ttf"
edb_ed $DB add /entrance/time/font/size int 14
edb_ed $DB add /entrance/time/font/style str "normal"
edb_ed $DB add /entrance/time/pos str "0.1:0.1"
edb_ed $DB add /entrance/time/offset str "0:60"
edb_ed $DB add /entrance/time/color str "#D8FFFFFF"

edb_ed $DB add /entrance/face/pos str "0.1:0.1"
edb_ed $DB add /entrance/face/offset str "0:95"
edb_ed $DB add /entrance/face/size str "192:223"
edb_ed $DB add /entrance/face/border int 2
edb_ed $DB add /entrance/face/color str "#202080C0"

edb_ed $DB add /entrance/background str "BlueCrystal.bg.db"
edb_ed $DB add /entrance/pointer str "pointer.png"

