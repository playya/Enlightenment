#!/usr/bin/env python2

import c_evas

from c_evas import Canvas, SmartObject, ClippedSmartObject, Rectangle, Line, \
     Image, FilledImage, Gradient, Polygon, Text, Rect, EvasLoadError, \
     shutdown, render_method_lookup, render_method_list, \
     color_argb_premul, color_argb_unpremul, color_parse, \
     color_rgb_to_hsv, color_hsv_to_rgb


EVAS_CALLBACK_MOUSE_IN = 0
EVAS_CALLBACK_MOUSE_OUT = 1
EVAS_CALLBACK_MOUSE_DOWN = 2
EVAS_CALLBACK_MOUSE_UP = 3
EVAS_CALLBACK_MOUSE_MOVE = 4
EVAS_CALLBACK_MOUSE_WHEEL = 5
EVAS_CALLBACK_FREE = 6
EVAS_CALLBACK_KEY_DOWN = 7
EVAS_CALLBACK_KEY_UP = 8
EVAS_CALLBACK_FOCUS_IN = 9
EVAS_CALLBACK_FOCUS_OUT = 10
EVAS_CALLBACK_SHOW = 11
EVAS_CALLBACK_HIDE = 12
EVAS_CALLBACK_MOVE = 13
EVAS_CALLBACK_RESIZE = 14
EVAS_CALLBACK_RESTACK = 15
EVAS_CALLBACK_DEL = 16

EVAS_BUTTON_NONE = 0
EVAS_BUTTON_DOUBLE_CLICK = 1
EVAS_BUTTON_TRIPLE_CLICK = 2

EVAS_RENDER_BLEND = 0
EVAS_RENDER_BLEND_REL = 1
EVAS_RENDER_COPY = 2
EVAS_RENDER_COPY_REL = 3
EVAS_RENDER_ADD = 4
EVAS_RENDER_ADD_REL = 5
EVAS_RENDER_SUB = 6
EVAS_RENDER_SUB_REL = 7
EVAS_RENDER_TINT = 8
EVAS_RENDER_TINT_REL = 9
EVAS_RENDER_MASK = 10
EVAS_RENDER_MUL = 11

EVAS_TEXTURE_REFLECT = 0
EVAS_TEXTURE_REPEAT = 1
EVAS_TEXTURE_RESTRICT = 2
EVAS_TEXTURE_RESTRICT_REFLECT = 3
EVAS_TEXTURE_RESTRICT_REPEAT = 4
EVAS_TEXTURE_PAD = 5

EVAS_COLOR_SPACE_ARGB = 0
EVAS_COLOR_SPACE_AHSV = 1

EVAS_COLORSPACE_ARGB8888 = 0 # ARGB 32 bits per pixel, high-byte is Alpha
EVAS_COLORSPACE_YCBCR422P601_PL = 1 # YCbCr 4:2:2 Planar, ITU.BT-601 specs.
EVAS_COLORSPACE_YCBCR422P709_PL = 2 # YCbCr 4:2:2 Planar, ITU.BT-709 specs.
EVAS_COLORSPACE_RGB565_A5P = 3 # 16bit rgb565 + Alpha plane (5/8 bits) at end

EVAS_FONT_HINTING_NONE = 0
EVAS_FONT_HINTING_AUTO = 1
EVAS_FONT_HINTING_BYTECODE = 2

EVAS_TEXT_STYLE_PLAIN = 0
EVAS_TEXT_STYLE_SHADOW = 1
EVAS_TEXT_STYLE_OUTLINE = 2
EVAS_TEXT_STYLE_SOFT_OUTLINE = 3
EVAS_TEXT_STYLE_GLOW = 4
EVAS_TEXT_STYLE_OUTLINE_SHADOW = 5
EVAS_TEXT_STYLE_FAR_SHADOW = 6
EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW = 7
EVAS_TEXT_STYLE_SOFT_SHADOW = 8
EVAS_TEXT_STYLE_FAR_SOFT_SHADOW = 9

EVAS_OBJECT_POINTER_MODE_AUTOGRAB = 0
EVAS_OBJECT_POINTER_MODE_NOGRAB = 1

EVAS_IMAGE_ROTATE_NONE = 0
EVAS_IMAGE_ROTATE_90 = 1
EVAS_IMAGE_ROTATE_180 = 2
EVAS_IMAGE_ROTATE_270 = 3

c_evas.init()
