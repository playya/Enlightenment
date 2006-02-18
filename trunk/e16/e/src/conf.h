/*
 * Copyright (C) 2000-2006 Carsten Haitzler, Geoff Harrison and various contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#define CONFIG_CONTROL 0
#define CONFIG_TEXT 1
#define CONFIG_MENU 2
#define CONFIG_BORDER 3
#define CONFIG_BUTTON 4
#define CONFIG_DESKTOP 5
#define CONFIG_ICONBOX 6
#define CONFIG_KEYBIND 8
#define CONFIG_SOUND 9
#define CONFIG_SLIDERS 10
#define CONFIG_ACTIONCLASS 11
#define CONFIG_IMAGECLASS 12
#define CONFIG_WINDOWMATCH 14
#define CONFIG_COLORMOD 15
#define CONFIG_SLIDEOUT 16
#define CONFIG_TOOLTIP 17
#define CONFIG_FX 18
#define CONFIG_IBOX 19
#define CONFIG_EXTRAS 20
#define CONFIG_FONTS 21

#define CONFIG_CLASSNAME 100
#define CONFIG_MODIFIER  101
#define CONFIG_TYPE 102
#define CONFIG_ANYMOD 103
#define CONFIG_ACTION 104
#define CONFIG_NEXT 105
#define CONFIG_INHERIT 106
#define CONFIG_ACTION_TOOLTIP 107

#define TEXT_ORIENTATION 200
#define TEXT_JUSTIFICATION 201
#define TEXT_MODE 202
#define TEXT_FONTNAME 203
#define TEXT_EFFECT 204
#define TEXT_FG_COL 205
#define TEXT_BG_COL 206

#define ICLASS_NAME 350
#define ICLASS_NORMAL 351
#define ICLASS_CLICKED 352
#define ICLASS_HILITED 353
#define ICLASS_ACTIVE_NORMAL 354
#define ICLASS_ACTIVE_CLICKED 355
#define ICLASS_ACTIVE_HILITED 356
#define ICLASS_LRTB 357
#define ICLASS_PADDING 358
#define ICLASS_STICKY_NORMAL 359
#define ICLASS_STICKY_CLICKED 360
#define ICLASS_STICKY_HILITED 361
#define ICLASS_STICKY_ACTIVE_NORMAL 362
#define ICLASS_STICKY_ACTIVE_CLICKED 363
#define ICLASS_STICKY_ACTIVE_HILITED 364
#define ICLASS_SUPERIMPOSE 365
#define ICLASS_DISABLED 366
#define ICLASS_ACTIVE_DISABLED 367
#define ICLASS_STICKY_DISABLED 368
#define ICLASS_STICKY_ACTIVE_DISABLED 369
#define ICLASS_COLORMOD 370
#define ICLASS_FILLRULE 371
#define ICLASS_TRANSPARENT 372

#define DESKTOP_DRAGDIR 400
#define DESKTOP_DRAGBAR_WIDTH 401
#define DESKTOP_DRAGBAR_ORDERING 402
#define DESKTOP_DRAGBAR_LENGTH 403
#define DESKTOP_SLIDEIN 404
#define DESKTOP_SLIDESPEED 405
#define DESKTOP_HIQUALITYBG 406
#define DESKTOP_AREA_SIZE 407

#define ACLASS_NAME 420
#define ACLASS_TYPE 421
#define ACLASS_MODIFIER 422
#define ACLASS_ANYMOD 423
#define ACLASS_ANYBUT 424
#define ACLASS_BUT 425
#define ACLASS_ANYKEY 426
#define ACLASS_KEY 427
#define ACLASS_EVENT_TRIGGER 428
#define ACLASS_ACTION 429

#define ACLASS_TYPE_ACLASS 5

#define BORDERPART_ICLASS 450
#define BORDERPART_ACLASS 451
#define BORDERPART_TEXTCLASS 452
#define BORDERPART_ONTOP 453
#define BORDERPART_FLAGS 454
#define BORDERPART_ISREGION 455
#define BORDERPART_WMIN 456
#define BORDERPART_WMAX 457
#define BORDERPART_TXP 458
#define BORDERPART_TXA 459
#define BORDERPART_TYP 460
#define BORDERPART_TYA 461
#define BORDERPART_BORIGIN 462
#define BORDERPART_BXP 463
#define BORDERPART_BXA 464
#define BORDERPART_BYP 465
#define BORDERPART_BYA 466
#define BORDERPART_TORIGIN 467
#define BORDERPART_HMIN 468
#define BORDERPART_HMAX 469
#define BORDERPART_KEEPSHADE 470

#define WINDOWMATCH_USEBORDER 480
#define WINDOWMATCH_MATCHNAME 481
#define WINDOWMATCH_MATCHCLASS 482
#define WINDOWMATCH_MATCHTITLE 483
#define WINDOWMATCH_WIDTH 484
#define WINDOWMATCH_HEIGHT 485
#define WINDOWMATCH_TRANSIENT 486
#define WINDOWMATCH_NO_RESIZE_H 487
#define WINDOWMATCH_NO_RESIZE_V 488
#define WINDOWMATCH_SHAPED 489
#define WINDOWMATCH_ICON 490
#define WINDOWMATCH_DESKTOP 491
#define WINDOWMATCH_MAKESTICKY 492

#define BORDER_NAME 500
#define BORDER_LEFT 501
#define BORDER_RIGHT 502
#define BORDER_TOP 503
#define BORDER_BOTTOM 504
#define BORDER_INIT 505
#define SHADEDIR 506
#define BORDER_CHANGES_SHAPE 507
#define BORDER_GROUP_NAME 508

#define BUTTON_NAME 520
#define BUTTON_ACLASS 521
#define BUTTON_ICLASS 522
#define BUTTON_MINW 523
#define BUTTON_MAXW 524
#define BUTTON_FLAGS 525
#define BUTTON_MAXH 526
#define BUTTON_MINH 527
#define BUTTON_XO 528
#define BUTTON_YO 529
#define BUTTON_XA 530
#define BUTTON_XR 531
#define BUTTON_YA 532
#define BUTTON_YR 533
#define BUTTON_XSR 534
#define BUTTON_YSR 535
#define BUTTON_XSA 536
#define BUTTON_YSA 537
#define BUTTON_SIMG 538
#define BUTTON_DESK 539
#define BUTTON_STICKY 540
#define BUTTON_INTERNAL 541
#define BUTTON_SHOW 542
#define BUTTON_LABEL 543

#define BG_RGB 560
#define BG_BG1 561
#define BG_BG2 562
#define BG_NAME 563
#define BG_DESKNUM 564

#define KEY_CLASSNAME 580
#define KEY_MODIFIER 581
#define KEY_MOD 582
#define KEY_ANYBUT 583
#define KEY_BUT 584
#define KEY_ANYKEY 585
#define KEY_KEY 586
#define KEY_BINDACLASS 587

#define COLORMOD_RED 600
#define COLORMOD_GREEN 601
#define COLORMOD_BLUE 602

#define SLIDEOUT_DIRECTION 620

#define TOOLTIP_DRAWICLASS 640
#define TOOLTIP_BUBBLE1 641
#define TOOLTIP_BUBBLE2 642
#define TOOLTIP_BUBBLE3 643
#define TOOLTIP_BUBBLE4 644
#define TOOLTIP_DISTANCE 645
#define TOOLTIP_HELP_PIC 646

#define MENU_STYLE 699
#define MENU_BG_ICLASS 700
#define MENU_ITEM_ICLASS 701
#define MENU_SUBMENU_ICLASS 702
#define MENU_USE_ITEM_BACKGROUND 703
#define MENU_MAX_COLUMNS 704
#define MENU_MAX_ROWS 705
#define MENU_USE_STYLE 706
#define MENU_ITEM 707
#define MENU_SUBMENU 708
#define MENU_ACTION 709
#define MENU_PREBUILT 710
#define MENU_TITLE 711

#define MASK_NONE  0
#define MASK_SHIFT 900
#define MASK_LOCK 901
#define MASK_CTRL 902
#define MASK_MOD1 903
#define MASK_MOD2 904
#define MASK_MOD3 905
#define MASK_MOD4 906
#define MASK_MOD5 907
#define MASK_CTRL_ALT 910
#define MASK_CTRL_SHIFT 911
#define MASK_SHIFT_ALT 912
#define MASK_CTRL_SHIFT_ALT 913
#define MASK_CTRL_META4 914
#define MASK_SHIFT_META4 915
#define MASK_CTRL_META4_SHIFT 916
#define MASK_CTRL_META5 917
#define MASK_SHIFT_META5 918
#define MASK_CTRL_META5_SHIFT 919
#define MASK_WINDOWS_SHIFT 920
#define MASK_WINDOWS_CTRL 921
#define MASK_WINDOWS_ALT 922

#define CONFIG_ANYBUT 930
#define CONFIG_ANYKEY 931
#define CONFIG_OPEN  999
#define CONFIG_CLOSE 1000
#define CONFIG_NORMAL 5

#define CONFIG_CURSOR 800
#define CURS_BG_RGB 801
#define CURS_FG_RGB 802
#define XBM_FILE 803
#define NATIVE_ID 804

#define CONFIG_VERSION 1001
#define CONFIG_INVALID 9999

#define CONFIG_TRANSPARENCY 2001
#define CONFIG_SHOW_NAMES 2002
#define CONFIG_ICON_SIZE 2003
#define CONFIG_ICON_MODE 2004
#define CONFIG_SCROLLBAR_SIDE 2005
#define CONFIG_SCROLLBAR_ARROWS 2006
#define CONFIG_AUTOMATIC_RESIZE 2007
#define CONFIG_SHOW_ICON_BASE 2008
#define CONFIG_SCROLLBAR_AUTOHIDE 2009
#define CONFIG_COVER_HIDE 2010
#define CONFIG_RESIZE_ANCHOR 2011
#define CONFIG_IB_ANIMATE 2012
