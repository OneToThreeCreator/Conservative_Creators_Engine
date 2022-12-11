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
#ifndef INIH_LOCAL
#include <setjmp.h>
#endif

#include <ini.h>

#include "../include/cce/engine_common.h"
#include "../include/cce/utils.h"
#include "../include/cce/os_interaction.h"
#include "../include/cce/endianess.h"

#include "engine_common_internal.h"

struct cce_backend_data cce__engineBackend;
uint64_t cce__currentTime, cce__deltaTime;

uint16_t cce__buttonsBitFieldDiff;
uint16_t cce__buttonsBitField;
uint8_t  cce__axesPairChanged;
int8_t   cce__axes[8];
static void (*moveCallbacks[4])(int8_t, int8_t);
static void (*buttonsCallback)(uint16_t, uint16_t);
void (*cce__keyCallback)(cce_enum key, cce_enum state);

CCE_PUBLIC_OPTIONS uint8_t (*cceEngineShouldTerminate) (void);
CCE_PUBLIC_OPTIONS void (*cceSetEngineShouldTerminate) (uint8_t);

struct cce__string
{
   const char *str;
   size_t size;
};

struct cce__callbackData
{
   struct cce__string *names;
   int (*fn)(void*, const char*, const char*);
   int (*init)(void*);
   void *data;
   uint16_t namesQuantity;
   uint8_t flags;
};

CCE_ARRAY(iniCallbacks, struct cce__callbackData, uint16_t);
size_t iniCallbackLongestName = 11;
#ifndef INIH_LOCAL
jmp_buf g_jumpOnIniFailure;
#endif
uint16_t commonIniCallbackID;

CCE_PUBLIC_OPTIONS void cceSetAxisChangeCallback (void (*callback)(int8_t, int8_t), cce_enum axePair)
{
   moveCallbacks[axePair] = callback;
}

CCE_PUBLIC_OPTIONS void cceSetButtonCallback (void (*callback)(uint16_t buttonState, uint16_t diff))
{
   buttonsCallback = callback;
}

CCE_PUBLIC_OPTIONS void cceSetKeyCallback (void (*callback)(cce_enum key, cce_enum state))
{
   cce__keyCallback = callback;
}

CCE_PUBLIC_OPTIONS uint16_t cceRegisterIniCallback (const char **lowercasenames, void *data, int (*callback)(void*, const char*, const char*), int (*init)(void*), uint8_t flags)
{
   CCE_REALLOC_ARRAY(iniCallbacks, iniCallbacksQuantity + 1);
   iniCallbacks[iniCallbacksQuantity].data          = data;
   iniCallbacks[iniCallbacksQuantity].fn            = callback;
   iniCallbacks[iniCallbacksQuantity].namesQuantity = 0;
   for (const char **it = lowercasenames; *it != NULL; ++it, ++iniCallbacks[iniCallbacksQuantity].namesQuantity) {}
   iniCallbacks[iniCallbacksQuantity].names = malloc(iniCallbacks[iniCallbacksQuantity].namesQuantity * sizeof(struct cce__string));
   iniCallbacks[iniCallbacksQuantity].flags = flags | (-(init == NULL) & CCE_INI_CALLBACK_DO_NOT_INIT);
   iniCallbacks[iniCallbacksQuantity].init = init;
   struct cce__string *jit = iniCallbacks[iniCallbacksQuantity].names;
   for (const char **it = lowercasenames, **end = lowercasenames + iniCallbacks[iniCallbacksQuantity].namesQuantity; it < end; ++it, ++jit)
   {
      jit->str = *it;
      jit->size = strlen(*it);
      iniCallbackLongestName = CCE_MAX(jit->size, iniCallbackLongestName);
   }
   return iniCallbacksQuantity++;
}

CCE_PUBLIC_OPTIONS uint64_t cceGetDeltaTime (void)
{
   return cce__deltaTime;
}

CCE_PUBLIC_OPTIONS uint64_t cceGetTime (void)
{
   return cce__currentTime;
}

void calculateInternalDeltaTime (void)
{
   uint64_t currentTime = cce__engineBackend.getTime();
   cce__deltaTime       = currentTime - cce__currentTime;
   cce__currentTime     = currentTime;
}

#define CCE_MEMEQ(x, y) (memcmp(x, y, strlen(y)) == 0)

static int iniCallback (void *data, const char *name, const char *value)
{
   char buf[17];
   strncpy(buf, name, 17);
   cceMemoryToLowercase(buf, 16);
   const char *it = buf + (-CCE_MEMEQ(buf, "cur") & 3) + (-CCE_MEMEQ(buf + 3, "rent") & 4);
   if (CCE_STREQ(it, "path") || CCE_STREQ(it, "dir") || CCE_STREQ(it, "directory"))
   {
      strncpy(buf, value, 17);
      cceMemoryToLowercase(buf, 16);
      it = buf + cceIsCharDelimiter(buf[3]) + cceIsCharDelimiter(buf[4]) + 3;
      if ((CCE_MEMEQ(buf, "ini") && (buf[3] == '\0' || CCE_STREQ(it, "file"))) || (CCE_MEMEQ(buf, "game") && CCE_STREQ(it + 1, "ini")))
      {
         const char *path = data;
         size_t len = strlen(path);
         it = path + len;
         while (it > path)
         {
            --it;
            if (cceIsPathDelimiter(*it))
               break;
            --len;
         }
         char *tmp = malloc((len + 1) * sizeof(char));
         memcpy(tmp, path, len);
         tmp[len] = '\0';
         cceSetCurrentPath(tmp);
         free(tmp);
      }
      else if (!CCE_STREQ(buf, "unchanged"))
      {
         cceSetCurrentPath(value);
      }
   }
   return 0;
}

#define CCE_INI_CALLBACK_TO_BE_INITIALIZED 0x4

static int iniHandler (void *data, const char *section, const char *name, const char *value)
{
   char *buf = data;
   if (section == NULL)
      return iniCallback(iniCallbacks[commonIniCallbackID].data, name, value);
   strncpy(buf, section, iniCallbackLongestName + 1);
   cceMemoryToLowercase(buf, iniCallbackLongestName);
   for (struct cce__callbackData *it = iniCallbacks, *end = iniCallbacks + iniCallbacksQuantity; it < end; ++it)
   {
      for (struct cce__string *jit = it->names, *jend = it->names + it->namesQuantity; jit < jend; ++jit)
      {
         if (memcmp(buf, jit->str, jit->size + 1) == 0)
         {
            strncpy(buf, name, 11);
            cceMemoryToLowercase(buf, 10);
            if (CCE_STREQ(buf, "init") || CCE_STREQ(buf, "initialize"))
            {
               it->flags &= ~CCE_INI_CALLBACK_DO_NOT_INIT;
               it->flags |= ((cceStringToBool(value) && it->init != NULL) - 1) & CCE_INI_CALLBACK_DO_NOT_INIT;
               return 1;
            }
            it->flags |= CCE_INI_CALLBACK_TO_BE_INITIALIZED;
            if (it->fn(it->data, name, value) != 0)
            {
               printf("nonzero value returned by %s\n", it->names[0].str);
               #ifdef INIH_LOCAL
               return 0;
               #else
               longjmp(g_jumpOnIniFailure, -1);
               #endif
            }
            return 1;
         }
      }
   }
   fprintf(stderr, "ENGINE::INI_PARSE_ERROR:\nUnregistered section %s\n", section);
   return 0;
}

static int parseGameINI (const char *path)
{
   int result = 0;
   const char *names[3] = {"commonproperties", "baseproperties", NULL};
   commonIniCallbackID = cceRegisterIniCallback((void*)names, (void*)path, iniCallback, NULL, 0);
   FILE* file = fopen(path, "r");
   if (file == NULL)
   {
      fprintf(stderr, "ENGINE::INI::FILE_NOT_FOUND:\n%s - file can't be opened with fopen\n", path);
      return -1;
   }
   char* buf = malloc(iniCallbackLongestName + 1);
#ifndef INIH_LOCAL
   if (setjmp(g_jumpOnIniFailure) == 0 && ini_parse_file(file, iniHandler, buf) == 0)
   {
#else
   if (ini_parse_file(file, iniHandler, buf) == 0)
   {
#endif
      for (struct cce__callbackData *it = iniCallbacks, *end = iniCallbacks + iniCallbacksQuantity; it < end; ++it)
      {
         if ((it->flags & (CCE_INI_CALLBACK_DO_NOT_INIT | CCE_INI_CALLBACK_TO_BE_INITIALIZED)) == CCE_INI_CALLBACK_TO_BE_INITIALIZED)
            it->init(it->data);
         if (it->flags & CCE_INI_CALLBACK_FREE_DATA)
            free(it->data);
      }
   }
   else
   {
      result  = -1;
      fprintf(stderr, "ENGINE::INI::PARSING_FAILURE: function indicated critical error while parsing\n");
      for (struct cce__callbackData *it = iniCallbacks, *end = iniCallbacks + iniCallbacksQuantity; it < end; ++it)
      {
         if (it->flags & CCE_INI_CALLBACK_FREE_DATA)
            free(it->data);
      }
   }
   free(buf);
   fclose(file);
   return result;
}

void loadBackend__glfw (void);

int cce__initEngine (const char *path)
{
   moveCallbacks[0] = NULL;
   moveCallbacks[1] = NULL;
   moveCallbacks[2] = NULL;
   moveCallbacks[3] = NULL;
   buttonsCallback = NULL;
   cce__buttonsBitField = 0;
   cce__buttonsBitFieldDiff = 0;
   // We have only one api yet
   cceInitEndianConversion();
   loadBackend__glfw();
   
   if (parseGameINI(path) != 0)
      return -1;

   return 0;
}

void cce__engineUpdate (void)
{
   cce__engineBackend.engineUpdate();
   if (cce__buttonsBitFieldDiff != 0)
   {
      cce__buttonsBitField ^= cce__buttonsBitFieldDiff;
      if (buttonsCallback != NULL)
         buttonsCallback(cce__buttonsBitField, cce__buttonsBitFieldDiff);
      cce__buttonsBitFieldDiff = 0;
   }
   if (cce__axesPairChanged != 0)
   {
      void (**moveCallbackIt)(int8_t, int8_t) = moveCallbacks;
      for (int8_t *it = cce__axes, *end = cce__axes + 8; it < end; it += 2, ++moveCallbackIt, cce__axesPairChanged >>= 1)
         if ((cce__axesPairChanged & 1) && *moveCallbackIt != NULL)
            (*moveCallbackIt)(it[0], it[1]);
   }
}

void cce__terminateEngine (void)
{
   cce__engineBackend.terminateEngine();
   cceTerminateTemporaryDirectory();
   free(iniCallbacks);
   iniCallbacksQuantity   = 0;
   iniCallbacksAllocated  = 0;
   iniCallbackLongestName = 11;
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
