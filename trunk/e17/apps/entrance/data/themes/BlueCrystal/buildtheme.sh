#!/bin/sh -e
DB="./BlueCrystal.db"
rm -f $DB

edb_ed $DB add /entrance/welcome/font/name str "bedrock.ttf"
edb_ed $DB add /entrance/welcome/font/size int 20
edb_ed $DB add /entrance/welcome/font/style str "raised"
edb_ed $DB add /entrance/welcome/color/r int 255
edb_ed $DB add /entrance/welcome/color/g int 255
edb_ed $DB add /entrance/welcome/color/b int 255
edb_ed $DB add /entrance/welcome/color/a int 216
edb_ed $DB add /entrance/welcome/pos/x float 0.58
edb_ed $DB add /entrance/welcome/pos/y float 0.8
edb_ed $DB add /entrance/welcome/offset/x int 0
edb_ed $DB add /entrance/welcome/offset/y int 0
edb_ed $DB add /entrance/passwd/font/name str "bedrock.ttf"
edb_ed $DB add /entrance/passwd/font/size int 20
edb_ed $DB add /entrance/passwd/font/style str "raised"
edb_ed $DB add /entrance/passwd/color/r int 255
edb_ed $DB add /entrance/passwd/color/g int 255
edb_ed $DB add /entrance/passwd/color/b int 255
edb_ed $DB add /entrance/passwd/color/a int 216
edb_ed $DB add /entrance/entry/font/name str "notepad.ttf"
edb_ed $DB add /entrance/entry/font/size int 18
edb_ed $DB add /entrance/entry/font/style "normal"
edb_ed $DB add /entrance/entry/pos/x float 0.58
edb_ed $DB add /entrance/entry/pos/y float 0.8
edb_ed $DB add /entrance/entry/offset/x int 5
edb_ed $DB add /entrance/entry/offset/y int 45
edb_ed $DB add /entrance/entry/color/r int 0
edb_ed $DB add /entrance/entry/color/g int 64
edb_ed $DB add /entrance/entry/color/b int 32
edb_ed $DB add /entrance/entry/color/a int 255
edb_ed $DB add /entrance/entry/box/pos/x float 0.58
edb_ed $DB add /entrance/entry/box/pos/y float 0.8
edb_ed $DB add /entrance/entry/box/offset/x int 0
edb_ed $DB add /entrance/entry/box/offset/y int 35
edb_ed $DB add /entrance/entry/box/width int 300
edb_ed $DB add /entrance/entry/box/height int 42
edb_ed $DB add /entrance/entry/box/color/r int 255
edb_ed $DB add /entrance/entry/box/color/g int 255
edb_ed $DB add /entrance/entry/box/color/b int 255
edb_ed $DB add /entrance/entry/box/color/a int 176
edb_ed $DB add /entrance/sessions/selected/font/name str "orbit.ttf"
edb_ed $DB add /entrance/sessions/selected/font/size int 24
edb_ed $DB add /entrance/sessions/selected/font/style str "shadow"
edb_ed $DB add /entrance/sessions/selected/icon/width int 48
edb_ed $DB add /entrance/sessions/selected/icon/height int 48
edb_ed $DB add /entrance/sessions/selected/icon/pos/x float 0.58
edb_ed $DB add /entrance/sessions/selected/icon/pos/y float 0.1
edb_ed $DB add /entrance/sessions/selected/icon/offset/x int 0
edb_ed $DB add /entrance/sessions/selected/icon/offset/y int 0
edb_ed $DB add /entrance/sessions/selected/text/pos/x float 0.58
edb_ed $DB add /entrance/sessions/selected/text/pos/y float 0.1
edb_ed $DB add /entrance/sessions/selected/text/offset/x int 58
edb_ed $DB add /entrance/sessions/selected/text/offset/y int 6
edb_ed $DB add /entrance/sessions/selected/text/color/r int 255
edb_ed $DB add /entrance/sessions/selected/text/color/g int 255
edb_ed $DB add /entrance/sessions/selected/text/color/b int 255
edb_ed $DB add /entrance/sessions/selected/text/color/a int 255
edb_ed $DB add /entrance/sessions/selected/text/hicolor/r int 192
edb_ed $DB add /entrance/sessions/selected/text/hicolor/g int 255
edb_ed $DB add /entrance/sessions/selected/text/hicolor/b int 192
edb_ed $DB add /entrance/sessions/selected/text/hicolor/a int 255
edb_ed $DB add /entrance/sessions/list/text/font/name str "orbit.ttf"
edb_ed $DB add /entrance/sessions/list/text/font/size int 16
edb_ed $DB add /entrance/sessions/list/text/font/style str "outline"
edb_ed $DB add /entrance/sessions/list/text/color/r int 0
edb_ed $DB add /entrance/sessions/list/text/color/g int 0
edb_ed $DB add /entrance/sessions/list/text/color/b int 0
edb_ed $DB add /entrance/sessions/list/text/color/a int 128
edb_ed $DB add /entrance/sessions/list/seltext/font/name str "orbit.ttf"
edb_ed $DB add /entrance/sessions/list/seltext/font/size str 16
edb_ed $DB add /entrance/sessions/list/seltext/font/style str "shadow"
edb_ed $DB add /entrance/sessions/list/seltext/color/r int 0
edb_ed $DB add /entrance/sessions/list/seltext/color/g int 0
edb_ed $DB add /entrance/sessions/list/seltext/color/b int 128
edb_ed $DB add /entrance/sessions/list/seltext/color/a int 255
edb_ed $DB add /entrance/sessions/list/box/color/r int 192
edb_ed $DB add /entrance/sessions/list/box/color/g int 216
edb_ed $DB add /entrance/sessions/list/box/color/b int 255
edb_ed $DB add /entrance/sessions/list/box/color/a int 128
edb_ed $DB add /entrance/sessions/list/box/width int 180
edb_ed $DB add /entrance/sessions/list/box/height int 200
edb_ed $DB add /entrance/sessions/list/box/pos/x float 0.58
edb_ed $DB add /entrance/sessions/list/box/pos/y float 0.1
edb_ed $DB add /entrance/sessions/list/box/offset/x int 58
edb_ed $DB add /entrance/sessions/list/box/offset/y int 58
edb_ed $DB add /entrance/hostname/font/name str "orbit.ttf"
edb_ed $DB add /entrance/hostname/font/size int 24
edb_ed $DB add /entrance/hostname/font/style str "shadow"
edb_ed $DB add /entrance/hostname/pos/x float 0.1
edb_ed $DB add /entrance/hostname/pos/y float 0.1
edb_ed $DB add /entrance/hostname/offset/x int 0
edb_ed $DB add /entrance/hostname/offset/y int 0
edb_ed $DB add /entrance/hostname/color/r int 192
edb_ed $DB add /entrance/hostname/color/g int 255
edb_ed $DB add /entrance/hostname/color/b int 255
edb_ed $DB add /entrance/hostname/color/a int 255
edb_ed $DB add /entrance/date/font/name str "orbit.ttf"
edb_ed $DB add /entrance/date/font/size int 14
edb_ed $DB add /entrance/date/font/style str "normal"
edb_ed $DB add /entrance/date/pos/x float 0.1
edb_ed $DB add /entrance/date/pos/y float 0.1
edb_ed $DB add /entrance/date/offset/x int 0
edb_ed $DB add /entrance/date/offset/y int 40
edb_ed $DB add /entrance/date/color/r int 216
edb_ed $DB add /entrance/date/color/g int 255
edb_ed $DB add /entrance/date/color/b int 255
edb_ed $DB add /entrance/date/color/a int 255
edb_ed $DB add /entrance/time/font/name str "orbit.ttf"
edb_ed $DB add /entrance/time/font/size int 14
edb_ed $DB add /entrance/time/font/style str "normal"
edb_ed $DB add /entrance/time/pos/x float 0.1
edb_ed $DB add /entrance/time/pos/y float 0.1
edb_ed $DB add /entrance/time/offset/x int 0
edb_ed $DB add /entrance/time/offset/y int 60
edb_ed $DB add /entrance/time/color/r int 216
edb_ed $DB add /entrance/time/color/g int 255
edb_ed $DB add /entrance/time/color/b int 255
edb_ed $DB add /entrance/time/color/a int 255
edb_ed $DB add /entrance/face/pos/x float 0.1
edb_ed $DB add /entrance/face/pos/y float 0.1
edb_ed $DB add /entrance/face/offset/x int 0
edb_ed $DB add /entrance/face/offset/y int 95
edb_ed $DB add /entrance/face/width int 192
edb_ed $DB add /entrance/face/height int 223
edb_ed $DB add /entrance/face/border int 2
edb_ed $DB add /entrance/face/color/r int 32
edb_ed $DB add /entrance/face/color/g int 32
edb_ed $DB add /entrance/face/color/b int 128
edb_ed $DB add /entrance/face/color/a int 192
edb_ed $DB add /entrance/background str "BlueCrystal.bg.db"
edb_ed $DB add /entrance/pointer str "pointer.png"

sudo cp $DB /usr/local/share/entrance/data/themes/BlueCrystal/
sudo cp BlueCrystal.bg.db /usr/local/share/entrance/data/themes/BlueCrystal/
