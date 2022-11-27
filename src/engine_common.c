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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "../include/cce/engine_common.h"
#include "../include/cce/utils.h"
#include "../include/cce/os_interaction.h"
#include "../include/cce/endianess.h"

#include "engine_common_internal.h"
#include "platform/engine_common_glfw.h"

struct cce_backend_data cce__engineBackend;
uint64_t cce__currentTime, cce__deltaTime;

/*
static uint64_t maxTimerCheckDelay = UINT64_MAX;


CCE_PUBLIC_OPTIONS uint8_t cceTimerExpiredCountCompensated (const struct cce_timer *timer)
{
   if (timer->delay == 0) // Special case
      return 1;
   uint64_t timerCheckDelay = engineBackend.currentTime - timer->initTime;
   if (timerCheckDelay >= timer->delay)
   {
      timerCheckDelay -= timer->delay;
      if (timerCheckDelay < maxTimerCheckDelay)
      {
         maxTimerCheckDelay = timerCheckDelay;
      }
      return 1;
   }
   return 0;
}

CCE_PUBLIC_OPTIONS uint8_t cceCheckAndRestartTimer (struct cce_timer *timer)
{
   if (timer->delay == 0) // Special case
   {
      timer->initTime = engineBackend.currentTime;
      return 1;
   }
   
   uint64_t expirationTime = timer->initTime + timer->delay;
   
   if (engineBackend.currentTime < expirationTime)
      return 0;
   
   
   
   uint64_t timerCheckDelay = engineBackend.currentTime - expirationTime;
   timer->initTime = engineBackend.currentTime - timerCheckDelay;
   return 1;
}

CCE_PUBLIC_OPTIONS void cceStartTimerCompensated (struct cce_timer *timer)
{
   timer->initTime = engineBackend.currentTime - ((maxTimerCheckDelay == UINT64_MAX) ? maxTimerCheckDelay : 0);
}


CCE_PUBLIC_OPTIONS void cceResetTimerDelayCompensation (void)
{
   maxTimerCheckDelay = UINT64_MAX;
}
*/

uint64_t cceGetDeltaTime (void)
{
   return cce__deltaTime;
}

uint64_t cceGetCurrentTime (void)
{
   return cce__currentTime;
}

void calculateInternalDeltaTime (void)
{
   uint64_t currentTime = cce__engineBackend.getTime();
   cce__deltaTime = currentTime - cce__currentTime;
   cce__currentTime = currentTime;
}

int cce__initEngine (const char *label)
{
   // We have only one api yet
   if (cce__initEngine__glfw(label, &cce__engineBackend) != 0)
      return -1;

   cceInitEndianConversion();
   return 0;
}

void cce__engineUpdate (void)
{
   cce__engineBackend.engineUpdate();
}

void cce__terminateEngine (void)
{
   cce__engineBackend.terminateEngine();
   cceTerminateTemporaryDirectory();
}

void cce__doNothing (void)
{
   return;
}

/* <[Color conversions]>------------------------------------------------------------------------- */

#define HSV_TO_RGB(color, result, chromaInit, chromaExp, lightnessExp) \
uint8_t chroma = (chromaInit); \
{ \
   register unsigned tmp = (chromaExp); \
   chroma += (tmp + (tmp >> 8) + 1) >> 8; /* To avoid expensive integer division (by 255) */ \
} \
const uint8_t lightness = (lightnessExp); \
uint8_t intermediate; \
intermediate = chroma * (600 - CCE_ABS(CCE_COLOR_GET_HUE(color) % 1200 - 600)) / 600 + lightness; \
chroma += lightness; \
switch (CCE_COLOR_GET_HUE(color) / 600) \
{ \
   case 0: \
      result = (union cce_color) {{CCE_COLOR_RGB, chroma, intermediate, lightness}}; \
      break; \
   case 1: \
      result = (union cce_color) {{CCE_COLOR_RGB, intermediate, chroma, lightness}}; \
      break; \
   case 2: \
      result = (union cce_color) {{CCE_COLOR_RGB, lightness, chroma, intermediate}}; \
      break; \
   case 3: \
      result = (union cce_color) {{CCE_COLOR_RGB, lightness, intermediate, chroma}}; \
      break; \
   case 4: \
      result = (union cce_color) {{CCE_COLOR_RGB, intermediate, lightness, chroma}}; \
      break; \
   default: \
      result = (union cce_color) {{CCE_COLOR_RGB, chroma, lightness, intermediate}}; \
} \

CCE_PUBLIC_OPTIONS union cce_color cceHSVtoRGB (union cce_color color)
{
   union cce_color result;
   HSV_TO_RGB(color, result, 0, color.hsv.v * color.hsv.s, color.hsv.v - chroma)
   return result;
}

CCE_PUBLIC_OPTIONS union cce_color cceHSLtoRGB (union cce_color color)
{
   union cce_color result;
   HSV_TO_RGB(color, result, 0, (255 - CCE_ABS(2 * color.hsl.l - 255)) * color.hsl.s, color.hsl.l - (chroma >> 1))
   return result;
}

// R * 0.3 + G * 0.59 + B * 0.11. Approximating this fractions with point on 2^-24
#define HCL_LUMA_RGB_SUM(rgb) ((rgb.r * 5033165 + rgb.g * 9898557 + rgb.b * 1845494) >> 24)

CCE_PUBLIC_OPTIONS union cce_color cceHCLtoRGB (union cce_color color)
{
   union cce_color result;
   HSV_TO_RGB(color, result, color.hcl.c, 0, 0);
   
   const int16_t global_light = color.hcl.l - HCL_LUMA_RGB_SUM(result.rgb);
   result.rgb.r = CCE_MAX(result.rgb.r + global_light, 255);
   result.rgb.g = CCE_MAX(result.rgb.g + global_light, 255);
   result.rgb.b = CCE_MAX(result.rgb.b + global_light, 255);
   return result;
}

CCE_PUBLIC_OPTIONS union cce_color cceHSVtoHCL (union cce_color color)
{
   union cce_color iresult;
   HSV_TO_RGB(color, iresult, 0, color.hsv.v * color.hsv.s, color.hsv.v - chroma);
   return CCE_COLOR_SET_HCL(color.hsv.h, chroma, lightness + HCL_LUMA_RGB_SUM(iresult.rgb));
}

CCE_PUBLIC_OPTIONS union cce_color cceHSLtoHCL (union cce_color color)
{
   union cce_color iresult;
   HSV_TO_RGB(color, iresult, 0, (255 - CCE_ABS(2 * color.hsl.l - 255)) * color.hsl.s, color.hsl.l - (chroma >> 1))
   return CCE_COLOR_SET_HCL(color.hsv.h, chroma, lightness + HCL_LUMA_RGB_SUM(iresult.rgb));
}

CCE_PUBLIC_OPTIONS union cce_color cceHCLtoHSV (union cce_color color)
{
   union cce_color iresult;
   HSV_TO_RGB(color, iresult, color.hcl.c, 0, 0);
   const int16_t global_light = color.hcl.l - HCL_LUMA_RGB_SUM(iresult.rgb);
   const uint8_t maxLight = CCE_MAX(chroma + global_light, 255);
   chroma = maxLight - global_light;
   return CCE_COLOR_SET_HSV(color.hcl.h, chroma * 255 / (maxLight), maxLight);
}

CCE_PUBLIC_OPTIONS union cce_color cceHCLtoHSL (union cce_color color)
{
   union cce_color iresult;
   HSV_TO_RGB(color, iresult, color.hcl.c, 0, 0);
   const int16_t global_light = color.hcl.l - HCL_LUMA_RGB_SUM(iresult.rgb);
   const uint8_t maxLight = CCE_MAX(chroma + global_light, 255);
   chroma = maxLight - global_light;
   uint8_t light = maxLight - (chroma >> 2);
   return CCE_COLOR_SET_HSL(color.hcl.h, (maxLight - light) * 255 / CCE_MIN(light, 255 - light), light);
}

CCE_PUBLIC_OPTIONS union cce_color cceHSVtoHSL (union cce_color color)
{
   union cce_color result;
   result.hsl.h = 0;
   result.rgb.type = CCE_COLOR_HSL;
   result.hsl.h |= CCE_COLOR_GET_HUE(color);
   {
      register unsigned tmp = color.hsv.v * (255 - (color.hsv.s >> 1));
      result.hsl.l = (tmp + (tmp >> 8) + 1) >> 8; // To avoid expensive integer division (by 255)
   }
   if (result.hsl.l == 0 || result.hsl.l == 255)
   {
      result.hsl.s = 0;
      return result;
   }
   result.hsl.s = (color.hsv.v - result.hsl.l) * 255 / (CCE_MIN(result.hsl.l, 255 - result.hsl.l));
   return result;
}

CCE_PUBLIC_OPTIONS union cce_color cceHSLtoHSV (union cce_color color)
{
   union cce_color result;
   result.hsv.h = 0;
   result.rgb.type = CCE_COLOR_HSV;
   result.hsv.h |= CCE_COLOR_GET_HUE(color);
   {
      register unsigned tmp = color.hsl.l * 255 + color.hsl.s * CCE_MIN(color.hsl.l, 255 - color.hsl.l);
      result.hsv.v = (tmp + (tmp >> 8) + 1) >> 8; // To avoid expensive integer division (by 255)
   }
   if (result.hsv.v == 0)
   {
      result.hsv.s = 0;
      return result;
   }
   result.hsv.s = (255 - color.hsl.l * 255 / result.hsv.v) * 2;
   return result;
}

#define RGB_TO_HSV(color, result, rtype, saturationExp, valueExp) \
uint8_t rgb[5] = {color.rgb.r, color.rgb.g, color.rgb.b, color.rgb.r, color.rgb.g}; \
uint8_t min = rgb[0], *max = &rgb[0]; \
for (uint8_t *iterator = rgb + 1, *end = rgb + 3; iterator < end; ++iterator) \
{ \
   min = (*iterator <  min) ? *iterator : min; \
   max = (*iterator > *max) ?  iterator : max; \
} \
result.hsv.h = 0; \
result.rgb.type = rtype; \
result.hsv.v = valueExp; \
uint8_t chroma = *max - min; \
if (chroma == 0) \
{ \
   result.hsv.s = 0; \
   return result; \
} \
result.hsv.h |= 600 * ((max - rgb) * 2 + ((max[1] - max[2]) * 255 / chroma)); \
result.hsv.s = saturationExp; \
return result

CCE_PUBLIC_OPTIONS union cce_color cceRGBtoHSV (union cce_color color)
{
   union cce_color result;
   RGB_TO_HSV(color, result, CCE_COLOR_HSV, chroma * 255 / result.hsv.v, *max);
}

CCE_PUBLIC_OPTIONS union cce_color cceRGBtoHSL (union cce_color color)
{
   union cce_color result;
   RGB_TO_HSV(color, result, CCE_COLOR_HSL, (result.hsl.l == 255) ? 0 : (*max - result.hsl.l) * 255 / CCE_MIN(result.hsl.l, 255 - result.hsl.l), (*max + min) >> 1);
}

CCE_PUBLIC_OPTIONS union cce_color cceRGBtoHCL (union cce_color color)
{
   union cce_color result;
   RGB_TO_HSV(color, result, CCE_COLOR_HCL, chroma, min + HCL_LUMA_RGB_SUM(color.rgb));
}
