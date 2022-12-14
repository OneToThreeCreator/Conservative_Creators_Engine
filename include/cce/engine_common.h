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

#ifndef ENGINE_COMMON_H
#define ENGINE_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdint.h>
#include "config.h"

#include "cce_exports.h"
#define CCE_PUBLIC_OPTIONS CCE_EXPORTS

typedef uint8_t cce_void;
typedef uint8_t cce_enum;

#define CCE_VEC1_ST(prefix, type) struct prefix ## vec1 {type x;}
#define CCE_VEC2_ST(prefix, type) struct prefix ## vec2 {type x; type y;}
#define CCE_VEC3_ST(prefix, type) struct prefix ## vec3 {type x; type y; type z;}
#define CCE_VEC4_ST(prefix, type) struct prefix ## vec4 {type x; type y; type z; type w;}

#define CCE_VEC_STS(prefix, type) CCE_VEC1_ST(prefix, type); CCE_VEC2_ST(prefix, type); CCE_VEC3_ST(prefix, type); CCE_VEC4_ST(prefix, type)

CCE_VEC_STS(cce_u8,  uint8_t);
CCE_VEC_STS(cce_i8,  int8_t);
CCE_VEC_STS(cce_u16, uint16_t);
CCE_VEC_STS(cce_i16, int16_t);
CCE_VEC_STS(cce_u32, uint32_t);
CCE_VEC_STS(cce_i32, int32_t);
CCE_VEC_STS(cce_f32, float);
CCE_VEC_STS(cce_u64, uint64_t);
CCE_VEC_STS(cce_i64, int64_t);
CCE_VEC_STS(cce_f64, double);

struct cce_timer
{
   uint64_t initTime;
   uint32_t delay; /* Limited to about 71 minutes. Rarely needed, taking into account microsecond timer precision (a lot less than one frame) */
   uint8_t  flags;
};

#define CCE_COLOR_RGB 0x00
// HSV with decimal point at 0.1 [0 - 3599]
#define CCE_COLOR_HSV 0x20
// HSL with decimal point at 0.1 [0 - 3599]
#define CCE_COLOR_HSL 0x40
// HCL with decimal point at 0.1 [0 - 3599]
#define CCE_COLOR_HCL 0x60

#define CCE_COLOR_GET_HUE(color) (color.hsv.h & 0x1FFF)
// sets hue value of the color in range 0 - 7199 (clamped to 3599), don't disturb type
#define CCE_COLOR_SET_HUE(color, hue) (color.hsv.h &= ~0x1FFF, color.hsv.h |= (hue & 0x1FFF) - (-((hue & 0x1FFF) > 3600) & 3600))
#define CCE_COLOR_SET_TYPE(color, type) (color.rgb.type &= ~0xE0, color.rgb.type |= type & 0xE0)

#define CCE_COLOR_SET_RGB(r, g, b) (union cce_color) {{CCE_COLOR_RGB, r, g, b}}
#define CCE__COLOR_SET_HXX(h, x1, x2, type) (union cce_color) {.hsv = {((h & 0x1FFF) - (-((h & 0x1FFF) > 3600) & 3600)) | (type << 8), x1, x2}}
#define CCE_COLOR_SET_HSV(h, s, v) CCE__COLOR_SET_HXX(h, s, v, CCE_COLOR_HSV)
#define CCE_COLOR_SET_HSL(h, s, l) CCE__COLOR_SET_HXX(h, s, l, CCE_COLOR_HSL)
#define CCE_COLOR_SET_HCL(h, c, l) CCE__COLOR_SET_HXX(h, c, l, CCE_COLOR_HCL)

union cce_color
{
   struct
   {
      uint8_t type;
      uint8_t r;
      uint8_t g;
      uint8_t b;
   } rgb;
   struct
   {
      // 13-byte variable, 3 bytes reserved for color type
      uint16_t h;
      uint8_t  s;
      uint8_t  v;
   } hsv;
   struct
   {
      // 13-byte variable, 3 bytes reserved for color type
      uint16_t h;
      uint8_t  s;
      uint8_t  l;
   } hsl;
   struct
   {
      // 13-byte variable, 3 bytes reserved for color type
      uint16_t h;
      uint8_t  c;
      uint8_t  l;
   } hcl;
};

#define CCE_COLLIDER_RECT_ST(components, posbits, sizebits) \
struct cce_collider_rect ## components ## D_ ## posbits ## _ ## sizebits {struct cce_i ## posbits ## vec ## components position; struct cce_u ## sizebits ## vec ## components size;}

#define CCE_COLLIDER_RECT_STS(posbits, sizebits) CCE_COLLIDER_RECT_ST(1, posbits, sizebits); CCE_COLLIDER_RECT_ST(2, posbits, sizebits); CCE_COLLIDER_RECT_ST(3, posbits, sizebits); CCE_COLLIDER_RECT_ST(4, posbits, sizebits)

#define CCE__COLLIDER_RECT_STS(posbits) CCE_COLLIDER_RECT_STS(posbits, 8); CCE_COLLIDER_RECT_STS(posbits, 16); CCE_COLLIDER_RECT_STS(posbits, 32); CCE_COLLIDER_RECT_STS(posbits, 64)

CCE__COLLIDER_RECT_STS(8);
CCE__COLLIDER_RECT_STS(16);
CCE__COLLIDER_RECT_STS(32);
CCE__COLLIDER_RECT_STS(64);

/*
struct cce_collider_rectND_X_Y
{
   struct cce_iXvecN position;
   struct cce_uYvecN size;
};
*/

#define CCE_COLLIDER_CIR_ST(components, posbits, sizebits) \
struct cce_collider_cir ## components ## D_ ## posbits ## _ ## sizebits {struct cce_i ## posbits ## vec ## components position; uint ## sizebits ## _t radius;}

#define CCE_COLLIDER_CIR_STS(posbits, sizebits) CCE_COLLIDER_CIR_ST(1, posbits, sizebits); CCE_COLLIDER_CIR_ST(2, posbits, sizebits); CCE_COLLIDER_CIR_ST(3, posbits, sizebits); CCE_COLLIDER_CIR_ST(4, posbits, sizebits)

#define CCE__COLLIDER_CIR_STS(posbits) CCE_COLLIDER_CIR_STS(posbits, 8); CCE_COLLIDER_CIR_STS(posbits, 16); CCE_COLLIDER_CIR_STS(posbits, 32); CCE_COLLIDER_CIR_STS(posbits, 64)

CCE__COLLIDER_CIR_STS(8);
CCE__COLLIDER_CIR_STS(16);
CCE__COLLIDER_CIR_STS(32);
CCE__COLLIDER_CIR_STS(64);

/*
struct cce_collider_cirND_X_Y
{
   struct cce_iXvecN position;
   uintY_t radius;
};
*/

#define CCE_POW1(x) (x)
#define CCE_POW2(x) (x)*(x)
#define CCE_POW3(x) (x)*(x)*(x)
#define CCE_POW4(x) (CCE_POW2(x))*(CCE_POW2(x))

CCE_PUBLIC_OPTIONS uint64_t cceGetDeltaTime    (void);
CCE_PUBLIC_OPTIONS uint64_t cceGetTime  (void);

#define CCE_INI_CALLBACK_FREE_DATA 0x1
#define CCE_INI_CALLBACK_DO_NOT_INIT 0x2

CCE_PUBLIC_OPTIONS uint16_t cceRegisterIniCallback (const char **lowercasenames, void *data, int (*callback)(void*, const char*, const char*), int (*init)(void*), uint8_t flags);
CCE_PUBLIC_OPTIONS void cceSetAxisChangeCallback (void (*callback)(int8_t, int8_t), cce_enum axePair);
CCE_PUBLIC_OPTIONS void cceSetButtonCallback (void (*callback)(uint16_t buttonState, uint16_t diff));
CCE_PUBLIC_OPTIONS void cceSetKeyCallback (void (*callback)(cce_enum key, cce_enum state));
CCE_PUBLIC_OPTIONS union cce_color cceHSVtoRGB (union cce_color color);
CCE_PUBLIC_OPTIONS union cce_color cceHSLtoRGB (union cce_color color);
CCE_PUBLIC_OPTIONS union cce_color cceHCLtoRGB (union cce_color color);
CCE_PUBLIC_OPTIONS union cce_color cceHSVtoHCL (union cce_color color);
CCE_PUBLIC_OPTIONS union cce_color cceHSLtoHCL (union cce_color color);
CCE_PUBLIC_OPTIONS union cce_color cceHCLtoHSV (union cce_color color);
CCE_PUBLIC_OPTIONS union cce_color cceHCLtoHSL (union cce_color color);
CCE_PUBLIC_OPTIONS union cce_color cceHSVtoHSL (union cce_color color);
CCE_PUBLIC_OPTIONS union cce_color cceHSLtoHSV (union cce_color color);
CCE_PUBLIC_OPTIONS union cce_color cceRGBtoHSV (union cce_color color);
CCE_PUBLIC_OPTIONS union cce_color cceRGBtoHSL (union cce_color color);
CCE_PUBLIC_OPTIONS union cce_color cceRGBtoHCL (union cce_color color);

#define cce__checkCollisionRect(element1, element2, comp) \
   element1.position. ## comp < (element2.position. ## comp + element2.size. ## comp) && (element1.position. ## comp + element1.size. ## comp) > element2.position. ## comp

#define cceCheckCollisionRect1D(element1, element2) \
   cce__checkCollisionRect(element1, element2, x)
   
#define cceCheckCollisionRect2D(element1, element2) \
   cce__checkCollisionRect(element1, element2, x) && cce__checkCollision(element1, element2, y)

#define cceCheckCollisionRect3D(element1, element2) \
   cce__checkCollisionRect(element1, element2, x) && cce__checkCollision(element1, element2, y) && cce__checkCollision(element1, element2, z)

#define cceCheckCollisionRect4D(element1, element2) \
   cce__checkCollisionRect(element1, element2, x) && cce__checkCollision(element1, element2, y) && cce__checkCollision(element1, element2, z) && cce__checkCollision(element1, element2, w)
   
#define cce__getCirPosDiff(element1, element2, comp) (element1.position. ## comp - element2.position. ## comp)
#define cce__getCirPosDiffSqC2(element1, element2) cce__getCirPosDiffSqC1(element1, element2) + CCE_POW2(cce__getCirPosDiff(element1, element2, y))
#define cce__getCirPosDiffSqC3(element1, element2) cce__getCirPosDiffSqC2(element1, element2) + CCE_POW2(cce__getCirPosDiff(element1, element2, z))
#define cce__getCirPosDiffSqC4(element1, element2) cce__getCirPosDiffSqC3(element1, element2) + CCE_POW2(cce__getCirPosDiff(element1, element2, w))

#define cceCheckCollisionCir1D(element1, element2) cce__getCirPosDiff(element1, element2, x)  < element1.radius + element2.radius
#define cceCheckCollisionCir2D(element1, element2) cce__getCirPosDiffSqC2(element1, element2) < CCE_POW2(element1.radius + element2.radius)
#define cceCheckCollisionCir3D(element1, element2) cce__getCirPosDiffSqC3(element1, element2) < CCE_POW2(element1.radius + element2.radius)
#define cceCheckCollisionCir4D(element1, element2) cce__getCirPosDiffSqC4(element1, element2) < CCE_POW2(element1.radius + element2.radius)

#define cceColorToRGB(color) \
(((color.rgb.type & 0xE0) == CCE_COLOR_RGB) ? color              : ((color.rgb.type & 0xE0) == CCE_COLOR_HSV) ? cceHSVtoRGB(color) : \
 ((color.rgb.type & 0xE0) == CCE_COLOR_HSL) ? cceHSLtoRGB(color) : cceHCLtoRGB(color))
 
#define cceColorToHSV(color) \
(((color.rgb.type & 0xE0) == CCE_COLOR_RGB) ? cceRGBtoHSV(color) : ((color.rgb.type & 0xE0) == CCE_COLOR_HSV) ? color : \
 ((color.rgb.type & 0xE0) == CCE_COLOR_HSL) ? cceHSLtoHSV(color) : cceHCLtoHSV(color))
 
#define cceColorToHSL(color) \
(((color.rgb.type & 0xE0) == CCE_COLOR_RGB) ? cceRGBtoHSL(color) : ((color.rgb.type & 0xE0) == CCE_COLOR_HSV) ? cceHSVtoHSL(color) : \
 ((color.rgb.type & 0xE0) == CCE_COLOR_HSL) ? color              : cceHCLtoHSL(color))

#define cceColorToHCL(color) \
(((color.rgb.type & 0xE0) == CCE_COLOR_RGB) ? cceRGBtoHCL(color) : ((color.rgb.type & 0xE0) == CCE_COLOR_HSV) ? cceHSVtoHCL(color) : \
 ((color.rgb.type & 0xE0) == CCE_COLOR_HSL) ? cceHSLtoHCL(color) : color)

// Let's hope compiler is gonna optimize that
#define cceColorToColorType(color, type) \
(((type & 0xE0) == CCE_COLOR_RGB) ? cceColorToRGB(color) : ((type & 0xE0) == CCE_COLOR_HSV) ? cceColorToHSV(color) : \
 ((type & 0xE0) == CCE_COLOR_HSL) ? cceColorToHSL(color) : cceColorToHCL(color))

CCE_PUBLIC_OPTIONS extern uint8_t (*cceEngineShouldTerminate) (void);
CCE_PUBLIC_OPTIONS extern void (*cceSetEngineShouldTerminate) (uint8_t);

#define CCE_BUTTON_A 0x1
#define CCE_BUTTON_B 0x2
#define CCE_BUTTON_X 0x4
#define CCE_BUTTON_Y 0x8
#define CCE_BUTTON_L 0x10
#define CCE_BUTTON_R 0x20
#define CCE_BUTTON_STICK_L 0x40
#define CCE_BUTTON_STICK_R 0x80
#define CCE_BUTTON_BACK 0x100
#define CCE_BUTTON_START 0x200
//#define CCE_BUTTON_GUIDE (Should never be used by any game)
#define CCE_TRIGGER_L 0x400
#define CCE_TRIGGER_R 0x800
#define CCE_BUTTON_DPAD_LEFT 0x1000
#define CCE_BUTTON_DPAD_RIGHT 0x2000
#define CCE_BUTTON_DPAD_DOWN 0x4000
#define CCE_BUTTON_DPAD_UP 0x8000

#define CCE_AXISPAIR_LSTICK 0
#define CCE_AXISPAIR_DPAD 1
#define CCE_AXISPAIR_RSTICK 2
#define CCE_AXISPAIR_TRIGGERS 3

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // ENGINE_COMMON_H
