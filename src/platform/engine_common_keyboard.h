/*
    Conservative Creator's Engine - open source engine for making games.
    Copyright (C) 2020-2022 Andrey Gaivoronskiy

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
    USA
*/

#ifndef ENGINE_COMMON_KEYBOARD_H
#define ENGINE_COMMON_KEYBOARD_H

#define CCE_KEY_NOKEY         0x00
#define CCE_KEY_PAGEUP        0x01
#define CCE_KEY_END           0x03
#define CCE_KEY_MENU          0x05
#define CCE_KEY_BACKSPACE     0x08
#define CCE_KEY_TAB           0x09
#define CCE_KEY_ENTER         0x0A
#define CCE_KEY_PAGEDOWN      0x0C
#define CCE_KEY_HOME          0x0D
#define CCE_KEY_PAUSE         0x18
#define CCE_KEY_INSERT        0x1A
#define CCE_KEY_ESCAPE        0x1B

#define CCE_KEY_SPACE         0x20
#define CCE_KEY_APOSTROPHE    0x27 /*'*/
#define CCE_KEY_COMMA         0x2C /*,*/
#define CCE_KEY_MINUS         0x2D /*-*/
#define CCE_KEY_PERIOD        0x2E /*.*/
#define CCE_KEY_SLASH         0x2F /*/*/
#define CCE_KEY_0             0x30
#define CCE_KEY_1             0x31
#define CCE_KEY_2             0x32
#define CCE_KEY_3             0x33
#define CCE_KEY_4             0x34
#define CCE_KEY_5             0x35
#define CCE_KEY_6             0x36
#define CCE_KEY_7             0x37
#define CCE_KEY_8             0x38
#define CCE_KEY_9             0x39
#define CCE_KEY_SEMICOLON     0x3B /*;*/
#define CCE_KEY_EQUAL         0x3D
#define CCE_KEY_UNKNOWN       0x3F
#define CCE_KEY_A             0x41
#define CCE_KEY_B             0x42
#define CCE_KEY_C             0x43
#define CCE_KEY_D             0x44
#define CCE_KEY_E             0x45
#define CCE_KEY_F             0x46
#define CCE_KEY_G             0x47
#define CCE_KEY_H             0x48
#define CCE_KEY_I             0x49
#define CCE_KEY_J             0x4A
#define CCE_KEY_K             0x4B
#define CCE_KEY_L             0x4C
#define CCE_KEY_M             0x4D
#define CCE_KEY_N             0x4E
#define CCE_KEY_O             0x4F
#define CCE_KEY_P             0x50
#define CCE_KEY_Q             0x51
#define CCE_KEY_R             0x52
#define CCE_KEY_S             0x53
#define CCE_KEY_T             0x54
#define CCE_KEY_U             0x55
#define CCE_KEY_V             0x56
#define CCE_KEY_W             0x57
#define CCE_KEY_X             0x58
#define CCE_KEY_Y             0x59
#define CCE_KEY_Z             0x5A
#define CCE_KEY_LEFT_BRACKET  0x5B
#define CCE_KEY_BACKSLASH     0x5C /*\*/
#define CCE_KEY_RIGHT_BRACKET 0x5D
#define CCE_KEY_GRAVE_ACCENT  0x60 /*`*/
#define CCE_KEY_DELETE        0x7F

#define CCE_KEY_LEFT_ARROW    0x3C
#define CCE_KEY_RIGHT_ARROW   0x3E
#define CCE_KEY_UP_ARROW      0x5E
#define CCE_KEY_DOWN_ARROW    0x76

#define CCE_KEY_LSHIFT        0x28
#define CCE_KEY_RSHIFT        0x29
#define CCE_KEY_LCONTROL      0x7B
#define CCE_KEY_RCONTROL      0x7D
#define CCE_KEY_LSUPER        0x6E
#define CCE_KEY_RSUPER        0x75
#define CCE_KEY_LALT          0x70
#define CCE_KEY_RALT          0x71

#define CCE_KEY_NUMLOCK       0x21
#define CCE_KEY_PRINTSCREEN   0x24
#define CCE_KEY_CAPSLOCK      0x40
#define CCE_KEY_SCROLLLOCK    0x5F

#define CCE_KEY_KP_ENTER      0x8A
#define CCE_KEY_KP_DIVIDE     0xAF /*/*/
#define CCE_KEY_KP_MULTIPLY   0xAA /***/
#define CCE_KEY_KP_PLUS       0xAB
#define CCE_KEY_KP_COMMA      0xAC /*,*/
#define CCE_KEY_KP_MINUS      0xAD /*-*/

#define CCE_KEY_KP_0          0xB0
#define CCE_KEY_KP_1          0xB1
#define CCE_KEY_KP_2          0xB2
#define CCE_KEY_KP_3          0xB3
#define CCE_KEY_KP_4          0xB4
#define CCE_KEY_KP_5          0xB5
#define CCE_KEY_KP_6          0xB6
#define CCE_KEY_KP_7          0xB7
#define CCE_KEY_KP_8          0xB8
#define CCE_KEY_KP_9          0xB9

#define CCE_KEY_F1            0xC1
#define CCE_KEY_F2            0xC2
#define CCE_KEY_F3            0xC3
#define CCE_KEY_F4            0xC4
#define CCE_KEY_F5            0xC5
#define CCE_KEY_F6            0xC6
#define CCE_KEY_F7            0xC7
#define CCE_KEY_F8            0xC8
#define CCE_KEY_F9            0xC9
#define CCE_KEY_F10           0xCA
#define CCE_KEY_F11           0xCB
#define CCE_KEY_F12           0xCC
#define CCE_KEY_F13           0xCD
#define CCE_KEY_F14           0xCE
#define CCE_KEY_F15           0xCF
#define CCE_KEY_F16           0xD0
#define CCE_KEY_F17           0xD1
#define CCE_KEY_F18           0xD2
#define CCE_KEY_F19           0xD3
#define CCE_KEY_F20           0xD4
#define CCE_KEY_F21           0xD5
#define CCE_KEY_F22           0xD6
#define CCE_KEY_F23           0xD7
#define CCE_KEY_F24           0xD8
#define CCE_KEY_F25           0xD9

#define CCE_KEY_LAST          CCE_KEY_F25

#define CCE_KEY_BREAK         CCE_KEY_PAUSE
#define CCE_KEY_PRSCN         CCE_KEY_PRINTSCREEN
#define CCE_KEY_SYSRQ         CCE_KEY_PRINTSCREEN
#define CCE_KEY_ESC           CCE_KEY_ESCAPE
#define CCE_KEY_HYPHEN        CCE_KEY_MINUS
#define CCE_KEY_DEL           CCE_KEY_DELETE
#define CCE_KEY_LCTRL         CCE_KEY_LCONTROL
#define CCE_KEY_RCTRL         CCE_KEY_RCONTROL
#define CCE_KEY_ALTGR         CCE_KEY_RALT
#define CCE_KEY_LBRACKET      CCE_KEY_LEFT_BRACKET
#define CCE_KEY_RBRACKET      CCE_KEY_RIGHT_BRACKET
#define CCE_KEY_KP_NUMLOCK    CCE_KEY_NUMLOCK
#define CCE_KEY_LEFT_ALT      CCE_KEY_LALT
#define CCE_KEY_RIGHT_ALT     CCE_KEY_RALT
#define CCE_KEY_LEFT_CONTROL  CCE_KEY_LCONTROL
#define CCE_KEY_RIGHT_CONTROL CCE_KEY_RCONTROL
#define CCE_KEY_LEFT_SUPER    CCE_KEY_LSUPER
#define CCE_KEY_RIGHT_SUPER   CCE_KEY_RSUPER
#define CCE_KEY_LEFT_SHIFT    CCE_KEY_LSHIFT
#define CCE_KEY_RIGHT_SHIFT   CCE_KEY_RSHIFT

#include <stdint.h>

#include "../../include/cce/engine_common.h"

struct cce_keys
{
   struct cce_u8vec2 horizontalAxis;
   struct cce_u8vec2 verticalAxis;
   struct cce_u8vec2 buttonA;
   struct cce_u8vec2 buttonB;
   struct cce_u8vec2 buttonX;
   struct cce_u8vec2 buttonY;
   struct cce_u8vec2 buttonL;
   struct cce_u8vec2 buttonR;
   struct cce_u8vec2 triggerL;
   struct cce_u8vec2 triggerR;
   struct cce_u8vec2 select;
   struct cce_u8vec2 start;
};

uint8_t cceKeyFromName (const char *name);
#define cceStringToKey(str) cceKeyFromName(str)
#define cceStringToKeys1(str) ((struct cce_u8vec1){cceKeyFromName(str)})
struct cce_u8vec2 cceStringToKeys2 (const char *str);
struct cce_u8vec3 cceStringToKeys3 (const char *str);
struct cce_u8vec4 cceStringToKeys4 (const char *str);

#endif // ENGINE_COMMON_KEYBOARD_H
