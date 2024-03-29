/*
   Conservative Creator's Engine - open source engine for making games.
   Copyright © 2020-2023 Andrey Gaivoronskiy

   This file is part of Conservative Creator's Engine.

   Conservative Creator's Engine is free software: you can redistribute it and/or modify it under 
   the terms of the GNU Lesser General Public License as published by the Free Software Foundation,
   either version 2 of the License, or (at your option) any later version.

   Conservative Creator's Engine is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
   PURPOSE. See the GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License along
   with Conservative Creator's Engine. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ENGINE_COMMON_H
#define ENGINE_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdint.h>
#include "config.h"

#include "attributes.h"
#include "cce_exports.h"

typedef uint8_t cce_void;
typedef uint8_t cce_enum;

typedef void (*cce_termfun)(void);

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
struct cce_collider_cir ## components ## D_ ## posbits ## _ ## sizebits {struct cce_i ## posbits ## vec ## components position; uint ## sizebits ## _t diameter;}

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
   uintY_t diameter;
};
*/

#define CCE_POW1(x) (x)
#define CCE_POW2(x) (x)*(x)
#define CCE_POW3(x) (x)*(x)*(x)
#define CCE_POW4(x) (CCE_POW2(x))*(CCE_POW2(x))

CCE_API uint32_t cceGetFrameDeltaTime    (void);
CCE_API uint32_t cceGetFrameCurrentTime  (void);

#define CCE_INI_CALLBACK_FREE_DATA 0x1
#define CCE_INI_CALLBACK_DO_NOT_INIT 0x2

/* Handles overflow of cceGetMonotonicTime. Maximum delay is 24.8 days */
#define cceIsTimeout(time, timeout) ((sizeof(time) == 4) ? (timeout) - (time) - 1 >= 0x7FFFFFFF : timeout <= time)

CCE_API int cceRegisterPlugin (uint32_t uid, void *data, int (*iniCallback)(void*, const char*, const char*), int (*init)(void*), int (*postinit)(void), void (*term)(void), uint8_t flags);
CCE_API uint16_t            cceRegisterUpdateCallback (void (*callback)(void));
CCE_API void                cceEnableUpdateCallback (uint16_t callbackID);
CCE_API void                cceDisableUpdateCallback (uint16_t callbackID);
CCE_API CCE_PURE_FN uint8_t cceCheckPlugin (uint32_t uid);
CCE_API CCE_PURE_FN uint8_t cceIsPluginLoading (uint32_t uid);
CCE_API void                cceSetAxisChangeCallback (void (*callback)(int8_t, int8_t), cce_enum axePair);
CCE_API void                cceSetButtonCallback (void (*callback)(uint16_t buttonState, uint16_t diff));
CCE_API void                cceSetKeyCallback (void (*callback)(cce_enum key, cce_enum state));
CCE_API int                 cceInit (const char *path);
CCE_API void                cceUpdate (void);
CCE_API void                cceTerminate (void);

CCE_API CCE_CONST_FN union cce_color cceHSVtoRGB (union cce_color color);
CCE_API CCE_CONST_FN union cce_color cceHSLtoRGB (union cce_color color);
CCE_API CCE_CONST_FN union cce_color cceHCLtoRGB (union cce_color color);
CCE_API CCE_CONST_FN union cce_color cceHSVtoHCL (union cce_color color);
CCE_API CCE_CONST_FN union cce_color cceHSLtoHCL (union cce_color color);
CCE_API CCE_CONST_FN union cce_color cceHCLtoHSV (union cce_color color);
CCE_API CCE_CONST_FN union cce_color cceHCLtoHSL (union cce_color color);
CCE_API CCE_CONST_FN union cce_color cceHSVtoHSL (union cce_color color);
CCE_API CCE_CONST_FN union cce_color cceHSLtoHSV (union cce_color color);
CCE_API CCE_CONST_FN union cce_color cceRGBtoHSV (union cce_color color);
CCE_API CCE_CONST_FN union cce_color cceRGBtoHSL (union cce_color color);
CCE_API CCE_CONST_FN union cce_color cceRGBtoHCL (union cce_color color);

#define cce__checkCollisionRect(element1, element2, comp) \
   (element1.position.comp < (element2.position. comp + element2.size.comp) && (element1.position. comp + element1.size.comp) > element2.position.comp)

#define cceCheckCollisionRect1D(element1, element2) \
   cce__checkCollisionRect(element1, element2, x)
   
#define cceCheckCollisionRect2D(element1, element2) \
   (cce__checkCollisionRect(element1, element2, x) && cce__checkCollisionRect(element1, element2, y))

#define cceCheckCollisionRect3D(element1, element2) \
   (cce__checkCollisionRect(element1, element2, x) && cce__checkCollisionRect(element1, element2, y) && cce__checkCollisionRect(element1, element2, z))

#define cceCheckCollisionRect4D(element1, element2) \
   (cce__checkCollisionRect(element1, element2, x) && cce__checkCollisionRect(element1, element2, y) && cce__checkCollisionRect(element1, element2, z) && cce__checkCollisionRect(element1, element2, w))
   
#define cce__getCirPosDiff(element1, element2, comp) (element1.position.comp - element2.position.comp)
#define cce__getCirPosDiffSqC2(element1, element2) CCE_POW2(cce__getCirPosDiff(element1, element2, x)) + CCE_POW2(cce__getCirPosDiff(element1, element2, y))
#define cce__getCirPosDiffSqC3(element1, element2) cce__getCirPosDiffSqC2(element1, element2) + CCE_POW2(cce__getCirPosDiff(element1, element2, z))
#define cce__getCirPosDiffSqC4(element1, element2) cce__getCirPosDiffSqC3(element1, element2) + CCE_POW2(cce__getCirPosDiff(element1, element2, w))

#define cceCheckCollisionCir1D(element1, element2) (cce__getCirPosDiff(element1, element2, x) *2 < element1.diameter + element2.diameter)
#define cceCheckCollisionCir2D(element1, element2) (cce__getCirPosDiffSqC2(element1, element2)*4 < CCE_POW2(element1.diameter + element2.diameter))
#define cceCheckCollisionCir3D(element1, element2) (cce__getCirPosDiffSqC3(element1, element2)*4 < CCE_POW2(element1.diameter + element2.diameter))
#define cceCheckCollisionCir4D(element1, element2) (cce__getCirPosDiffSqC4(element1, element2)*4 < CCE_POW2(element1.diameter + element2.diameter))

#define cce__getCirCenter(circle, comp) (circle).position.comp*2 + (circle).diameter
#define cce__getCircleSquareDistanceSq(circle, rect, comp) CCE_POW2(cce__getCirCenter(circle, comp) - CCE_CLAMP(cce__getCirCenter(circle, comp), (rect).position.comp*2, ((rect).position.comp + (rect).size.comp)*2))

#define cceCheckCollisionCirRect1D(circle, rect) (circle.position.x < (rect.position.x + rect.size.x) && (circle.position.x + circle.diameter) > rect.position.x)
#define cceCheckCollisionCirRect2D(circle, rect) (cce__getCircleSquareDistanceSq(circle, rect, x) + cce__getCircleSquareDistanceSq(circle, rect, y) < CCE_POW2(circle.diameter))
#define cceCheckCollisionCirRect3D(circle, rect) (cce__getCircleSquareDistanceSq(circle, rect, x) + cce__getCircleSquareDistanceSq(circle, rect, y) + cce__getCircleSquareDistanceSq(circle, rect, z) < CCE_POW2(circle.diameter))
#define cceCheckCollisionCirRect4D(circle, rect) (cce__getCircleSquareDistanceSq(circle, rect, x) + cce__getCircleSquareDistanceSq(circle, rect, y) + \
                                                  cce__getCircleSquareDistanceSq(circle, rect, z) + cce__getCircleSquareDistanceSq(circle, rect, w) < CCE_POW2(circle.diameter))

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

CCE_API extern uint8_t (*cceEngineShouldTerminate) (void);
CCE_API extern void (*cceSetEngineShouldTerminate) (uint8_t);
CCE_API extern void (*cceScreenUpdate) (void);
CCE_API extern const char *cceBackend;

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
