#include <definitions>

__E_CFG_VERSION 0

/*
******************************************************************************
* Internally used and looked for actions
******************************************************************************
* ACTION_BUTTON_DRAG is looked for by e to determine what "action" by a user
* will be applied to every button to start a button drag. if u dont like just
* moving and dragging (in case you accidentally keep doing it to buttons)
* add a modifer like alt, so u need to hold alt down whilst dragging to
* actually drag the button around
*/
__ACLASS __BGN
  __NAME ACTION_BUTTON_DRAG
  __TOOLTIP_TEXT "Aby Przeci�gn�� Pulpit, klinij i przeci�gaj mysz�"
  __TOOLTIP_TEXT "(na ka�dym pulpicie za wyj�tkiem 0)"
  __TYPE __TYPE_NORMAL
  __ALLOW_ANY_BUTTON __ON
  __ALLOW_ANY_MODIFIER_KEYS __ON
  __ALLOW_ANY_KEY __ON
  __EVENT __MOUSE_PRESS
  __ACTION __A_DRAG_BUTTON
__END

/*
******************************************************************************
* Actionclasses for buttons
******************************************************************************
*/

__ACLASS __BGN
  __NAME ACTION_GOTO_DESK_NEXT
  __TOOLTIP_TEXT "Zmiana Pulpit�w"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Id� do nast�pnego Pulpitu."
  __ALLOW_ANY_BUTTON __ON
  __ALLOW_ANY_MODIFIER_KEYS __ON
  __EVENT __MOUSE_RELEASE
  __ACTION __A_DESKTOP_NEXT
__END

__ACLASS __BGN
  __NAME ACTION_GOTO_DESK_PREV
  __TOOLTIP_TEXT "Zmiana Pulpit�w"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Id� do nast�pnego Pulpitu."
  __ALLOW_ANY_BUTTON __ON
  __ALLOW_ANY_MODIFIER_KEYS __ON
  __EVENT __MOUSE_RELEASE
  __ACTION __A_DESKTOP_PREV
__END

__ACLASS __BGN
  __NAME ACTION_MOVE
  __TOOLTIP_TEXT "Przesuwaj"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Przesuwaj to Okno."
  __EVENT __MOUSE_PRESS
  __BUTTON 1
  __ACTION __A_MOVE
  __NEXT_ACTION
  __TOOLTIP_ACTION_TEXT "Usu� w Cie�/Wysu� z Cienia to Okno."
  __EVENT __MOUSE_PRESS
  __BUTTON 2
  __ACTION __A_SHADE
  __NEXT_ACTION
  __TOOLTIP_ACTION_TEXT "Poka� Menu Operacji Okna."
  __EVENT __MOUSE_PRESS
  __BUTTON 3
  __ACTION __A_SHOW_MENU "named WINOPS_MENU"
  __NEXT_ACTION
  __TOOLTIP_ACTION_TEXT "Prze��cznik stanu usuni�cia w Cie� tego Okna."
  __EVENT __DOUBLE_CLICK
  __ALLOW_ANY_BUTTON __ON
  __ACTION __A_SHADE
  __NEXT_ACTION
  __TOOLTIP_ACTION_TEXT "Poka�/Ukryj Obramowanie Grupy Okien."
  __EVENT __MOUSE_PRESS
  __BUTTON 2
  __MODIFIER_KEY __SHIFT
  __ACTION __A_SHOWHIDE_GROUP
  __NEXT_ACTION
  __TOOLTIP_ACTION_TEXT "Utw�rz grup�."
  __EVENT __MOUSE_PRESS
  __BUTTON 1
  __MODIFIER_KEY __SHIFT
  __ACTION __A_START_GROUP
  __NEXT_ACTION
  __TOOLTIP_ACTION_TEXT "Dodaj do Aktualnej grupy."
  __EVENT __MOUSE_PRESS
  __BUTTON 1
  __MODIFIER_KEY __CTRL
  __ACTION __A_ADD_TO_GROUP
  __NEXT_ACTION
  __TOOLTIP_ACTION_TEXT "Rozbij grup�, do kt�rej nale�y to Okno."
  __EVENT __MOUSE_PRESS
  __BUTTON 1
  __MODIFIER_KEY __CTRL_SHIFT
  __ACTION __A_BREAK_GROUP
__END

__ACLASS __BGN
  __NAME ACTION_RESIZE
  __TOOLTIP_TEXT "Zmiana Rozmiaru"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Zmie� Rozmiar tego Okna."
  __EVENT __MOUSE_PRESS
  __BUTTON 1
  __ACTION __A_RESIZE
  __NEXT_ACTION
  __TOOLTIP_ACTION_TEXT "Przesu� to Okno."
  __BUTTON 3
  __ACTION __A_MOVE
__END

__ACLASS __BGN
  __NAME ACTION_RESIZE_H
  __TOOLTIP_TEXT "Zmie� Rozmiar Poziomy"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Zmie� rozmiar Poziomy tego Okna."
  __EVENT __MOUSE_PRESS
  __ALLOW_ANY_MODIFIER_KEYS __ON
  __BUTTON 1
  __ACTION __A_RESIZE_H
  __NEXT_ACTION
  __TOOLTIP_ACTION_TEXT "Przesu� to Okno."
  __BUTTON 3
  __ACTION __A_MOVE
__END

__ACLASS __BGN
  __NAME ACTION_RESIZE_V
  __TOOLTIP_TEXT "Zmie� Rozmiar Pionowy"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Zmie� Rozmiar Pionowy tego Okna."
  __EVENT __MOUSE_PRESS
  __ALLOW_ANY_MODIFIER_KEYS __ON
  __BUTTON 1
  __ACTION __A_RESIZE_V
  __NEXT_ACTION
  __TOOLTIP_ACTION_TEXT "Przesu� to Okno."
  __BUTTON 3
  __ACTION __A_MOVE
__END

__ACLASS __BGN
  __NAME ACTION_KILL
  __TOOLTIP_TEXT "Zamknij"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Zamknij to Okno."
  __EVENT __MOUSE_RELEASE
  __ALLOW_ANY_MODIFIER_KEYS __ON
  __BUTTON 1
  __ACTION __A_KILL
  __NEXT_ACTION
  __TOOLTIP_ACTION_TEXT "Zniszcz to Okno."
  __BUTTON 3
  __ACTION __A_KILL_NASTY
__END

__ACLASS __BGN
  __NAME ACTION_MAXH
  __TOOLTIP_TEXT "Ustaw Maksymaln� Wysoko��"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Prze��cznik pomi�dzy maksymaln� wysoko�ci� ekranu a wysoko�ci� normaln�."
  __EVENT __MOUSE_RELEASE
  __ALLOW_ANY_MODIFIER_KEYS __ON
  __ALLOW_ANY_BUTTON __ON
  __ACTION __A_MAX_HEIGHT conservative
__END

__ACLASS __BGN
  __NAME ACTION_MAXW
  __TOOLTIP_TEXT "Ustaw Maksymaln� Szeroko��"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Prze��cznik pomi�dzy maksymaln� szeroko�ci� ekranu a szeroko�ci� normaln�."
  __EVENT __MOUSE_RELEASE
  __ALLOW_ANY_MODIFIER_KEYS __ON
  __ALLOW_ANY_BUTTON __ON
  __ACTION __A_MAX_WIDTH conservative
__END

__ACLASS __BGN
  __NAME ACTION_MAX
  __TOOLTIP_TEXT "Ustaw Maksymalny Rozmiar"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Prze��cznik pomi�dzy maksymalnym rozmiarem ekranu a rozmiarem normalnym."
  __EVENT __MOUSE_RELEASE
  __ALLOW_ANY_MODIFIER_KEYS __ON
  __BUTTON 1
  __ACTION __A_MAX_SIZE conservative
  __NEXT_ACTION
  __TOOLTIP_ACTION_TEXT "Prze��cznik pomi�dzy maksymaln� szeroko�ci� ekranu a szeroko�ci� normaln�."
  __BUTTON 2
  __ACTION __A_MAX_WIDTH conservative
  __NEXT_ACTION
  __TOOLTIP_ACTION_TEXT "Prze��cznik pomi�dzy maksymaln� wysoko�ci� ekranu a wysoko�ci� normaln�."
  __BUTTON 3
  __ACTION __A_MAX_HEIGHT conservative
__END

__ACLASS __BGN
  __NAME ACTION_SEND_NEXT
  __TOOLTIP_TEXT "Wy�lij na inny Pulpit"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Wy�lij to Okno na nast�pny Pulpit."
  __EVENT __MOUSE_RELEASE
  __ALLOW_ANY_MODIFIER_KEYS __ON
  __ALLOW_ANY_BUTTON __ON
  __ACTION __A_SEND_TO_NEXT_DESK
__END

__ACLASS __BGN
  __NAME ACTION_SEND_PREV
  __TOOLTIP_TEXT "Wy�lij na inny Pulpit"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Wy�lij to Okno na poprzedni Pulpit."
  __EVENT __MOUSE_RELEASE
  __ALLOW_ANY_MODIFIER_KEYS __ON
  __ALLOW_ANY_BUTTON __ON
  __ACTION __A_SEND_TO_PREV_DESK
__END

__ACLASS __BGN
  __NAME ACTION_SNAPSHOT
  __TOOLTIP_TEXT "Zdj�cie Ekranu"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Ten przycisk nie robi niczego interesujacego."
  __ALLOW_ANY_BUTTON __ON
  __EVENT __MOUSE_RELEASE
  __ACTION __A_SNAPSHOT
__END

__ACLASS __BGN
  __NAME ACTION_SHADE
  __TOOLTIP_TEXT "Usu� w Cie�"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Usu� to Okno w Cie� (zwi�)."
  __ALLOW_ANY_BUTTON __ON
  __EVENT __MOUSE_RELEASE
  __ACTION __A_SHADE
__END

__ACLASS __BGN
  __NAME ACTION_UNSHADE
  __TOOLTIP_TEXT "Wysu� z Cienia"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Wysu� to Okno z Cienia (rozwi�)."
  __ALLOW_ANY_BUTTON __ON
  __EVENT __MOUSE_RELEASE
  __ACTION __A_SHADE
__END

__ACLASS __BGN
  __NAME ACTION_MENU
  __TOOLTIP_TEXT "Window Options"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Open the Window Options Menu."
  __ALLOW_ANY_MODIFIER_KEYS __ON
  __EVENT __MOUSE_PRESS
  __ALLOW_ANY_BUTTON __ON
  __ACTION __A_SHOW_MENU "named WINOPS_MENU"
__END


__ACLASS __BGN
  __NAME ACTION_ICONIFY
  __TOOLTIP_TEXT "Zwi� do Ikony"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Zwi� to Okno do Ikony."
  __ALLOW_ANY_MODIFIER_KEYS __ON
  __EVENT __MOUSE_RELEASE
  __BUTTON 1
  __ACTION __A_ICONIFY
  __NEXT_ACTION
  __TOOLTIP_ACTION_TEXT "Otw�rz Menu Opcji Okna."
  __EVENT __MOUSE_PRESS
  __BUTTON 3
  __ACTION __A_SHOW_MENU "named WINOPS_MENU"
__END

__ACLASS __BGN
  __NAME ACTION_WINDOW_SLIDEOUT
  __TOOLTIP_TEXT "Wi�cej Przycisk�w"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Poka� wi�cej przycisk�w."
  __ALLOW_ANY_BUTTON __ON
  __EVENT __MOUSE_RELEASE
  __ACTION __A_SLIDEOUT mainwindowslider
__END

__ACLASS __BGN
  __NAME ACTION_RAISE
  __TOOLTIP_TEXT "Podnie�"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Podnie� to okno na wy�sz� warstw�."
  __EVENT __MOUSE_RELEASE
  __ALLOW_ANY_MODIFIER_KEYS __ON
  __ALLOW_ANY_BUTTON __ON
  __ACTION __A_RAISE
__END

__ACLASS __BGN
  __NAME ACTION_LOWER
  __TOOLTIP_TEXT "Obni�"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Przenie� to okno do ni�szej warstwy."
  __EVENT __MOUSE_RELEASE
  __ALLOW_ANY_MODIFIER_KEYS __ON
  __ALLOW_ANY_BUTTON __ON
  __ACTION __A_LOWER
__END

__ACLASS __BGN
  __NAME ACTION_STICK
  __TOOLTIP_TEXT "Przyklej/Odklej"
  __TYPE __TYPE_NORMAL
  __TOOLTIP_ACTION_TEXT "Prze��cznik stanu przyklejenia Okna (b�dzie ono widoczne na wszystkich Pulpitach)."
  __EVENT __MOUSE_RELEASE
  __ALLOW_ANY_MODIFIER_KEYS __ON
  __ALLOW_ANY_BUTTON __ON
  __ACTION __A_STICK
__END

