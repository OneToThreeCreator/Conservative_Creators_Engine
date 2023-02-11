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

#include <assert.h>
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

#define CCE_CALLBACK_ENABLED  0x1
#define CCE_CALLBACK_DISABLED 0x0

struct iniCallbackData
{
   int (*fn)(void*, const char*, const char*);
   int (*init)(void*);
   void *data;
   const char *name;
   uint16_t nameLength;
   uint8_t flags;
};

struct updateCallbackData
{
   void (*fn)(void);
   uint8_t flags;
};

CCE_API const char *cceBackend;
struct iniCallbackData *iniCallbacks = NULL;
cce_termfun *terminationCallbacks = NULL;
// Backend must be registered first, but it can do so ONLY when initEngine() is called. Workaround
uint16_t iniCallbacksQuantity = 1;
uint16_t iniCallbacksAllocated = 0;
uint16_t terminationCallbacksQuantity = 1;
uint16_t terminationCallbacksAllocated = 0;
CCE_ARRAY(updateCallbacks, struct updateCallbackData, uint16_t);

#define CCE_INI_CALLBACK_TO_BE_INITIALIZED 0x4
#define CCE_INI_CALLBACK_NO_TERMINATION_CALLBACK 0x8

struct cce_backend_data cce__engineBackend;
uint64_t cce__currentTime = 0, cce__deltaTime = 0;

struct cce_u16vec2 cce__gameResolution;
uint16_t cce__buttonsBitFieldDiff;
uint16_t cce__buttonsBitField;
uint8_t  cce__axesPairChanged;
int8_t   cce__axes[8];
static void (*moveCallbacks[4])(int8_t, int8_t);
static void (*buttonsCallback)(uint16_t, uint16_t);
void (*cce__keyCallback)(cce_enum key, cce_enum state);

CCE_API uint8_t (*cceEngineShouldTerminate) (void);
CCE_API void (*cceSetEngineShouldTerminate) (uint8_t);
CCE_API void (*cceScreenUpdate) (void);

size_t iniCallbackLongestName = 11;
#ifndef INIH_LOCAL
jmp_buf g_jumpOnIniFailure;
#endif
uint16_t commonIniCallbackID;
uint8_t  ignoreUninitializedPlugins;

CCE_API void cceSetAxisChangeCallback (void (*callback)(int8_t, int8_t), cce_enum axePair)
{
   moveCallbacks[axePair] = callback;
}

CCE_API void cceSetButtonCallback (void (*callback)(uint16_t buttonState, uint16_t diff))
{
   buttonsCallback = callback;
}

CCE_API void cceSetKeyCallback (void (*callback)(cce_enum key, cce_enum state))
{
   cce__keyCallback = callback;
}

CCE_API uint16_t cceRegisterUpdateCallback (void (*callback)(void))
{
   if (updateCallbacksQuantity >= updateCallbacksAllocated)
      CCE_REALLOC_ARRAY(updateCallbacks, updateCallbacksQuantity + 1);
   updateCallbacks[updateCallbacksQuantity].fn = callback;
   updateCallbacks[updateCallbacksQuantity].flags = CCE_CALLBACK_ENABLED;
   return updateCallbacksQuantity++;
}

CCE_API void cceEnableUpdateCallback (uint16_t callbackID)
{
   assert(callbackID < updateCallbacksQuantity);
   updateCallbacks[callbackID].flags ^= CCE_CALLBACK_ENABLED;
}

CCE_API void cceDisableUpdateCallback (uint16_t callbackID)
{
   assert(callbackID < updateCallbacksQuantity);
   updateCallbacks[callbackID].flags ^= CCE_CALLBACK_ENABLED;
}

static int emptyIniCallback (void *st, const char *name, const char *value)
{
   CCE_UNUSED(st);
   CCE_UNUSED(name);
   CCE_UNUSED(value);
   return 0;
}

CCE_API void cceRegisterPlugin (const char *lowercaseName, void *data, int (*iniCallback)(void*, const char*, const char*), int (*init)(void*), void (*term)(void), uint8_t flags)
{
   if (cceCheckPlugin(lowercaseName))
      return;
   if (iniCallbacksQuantity >= iniCallbacksAllocated)
      CCE_REALLOC_ARRAY(iniCallbacks, iniCallbacksQuantity + 1);
   iniCallbacks[iniCallbacksQuantity].data          = data;
   iniCallbacks[iniCallbacksQuantity].fn            = (iniCallbacks != NULL) ? iniCallback : emptyIniCallback;
   iniCallbacks[iniCallbacksQuantity].flags = flags | (-(init == NULL) & CCE_INI_CALLBACK_DO_NOT_INIT) | (-(term == NULL) & CCE_INI_CALLBACK_NO_TERMINATION_CALLBACK);
   iniCallbacks[iniCallbacksQuantity].init = init;
   iniCallbacks[iniCallbacksQuantity].name = lowercaseName;
   iniCallbacks[iniCallbacksQuantity].nameLength = strlen(lowercaseName);
   iniCallbackLongestName = CCE_MAX(iniCallbacks[iniCallbacksQuantity].nameLength, iniCallbackLongestName);
   if (term != NULL)
   {
      if (terminationCallbacksQuantity >= terminationCallbacksAllocated)
         CCE_REALLOC_ARRAY(terminationCallbacks, terminationCallbacksQuantity + 1);
      terminationCallbacks[terminationCallbacksQuantity] = term;
      ++terminationCallbacksQuantity;
   }
   ++iniCallbacksQuantity;
}

void cce__registerBackend (const char *lowercasename, void *data, int (*iniCallback)(void*, const char*, const char*), int (*init)(void*), void (*term)(void), uint8_t flags)
{
   if (iniCallbacksQuantity > iniCallbacksAllocated)
      CCE_ALLOC_ARRAY(iniCallbacks, 1);
   iniCallbacks[0].data          = data;
   iniCallbacks[0].fn            = iniCallback;
   iniCallbacks[0].flags = flags | (-(init == NULL) & CCE_INI_CALLBACK_DO_NOT_INIT) | (-(term == NULL) & CCE_INI_CALLBACK_NO_TERMINATION_CALLBACK);
   iniCallbacks[0].init = init;
   iniCallbacks[0].name = "window";
   iniCallbacks[0].nameLength = 6;
   iniCallbackLongestName = CCE_MAX(iniCallbacks[0].nameLength, iniCallbackLongestName);
   if (term != NULL)
   {
      if (terminationCallbacksQuantity > terminationCallbacksAllocated)
         CCE_ALLOC_ARRAY(terminationCallbacks, 1);
      terminationCallbacks[0] = term;
   }
   cceBackend = lowercasename;
}

CCE_API uint64_t cceGetDeltaTime (void)
{
   return cce__deltaTime;
}

CCE_API uint64_t cceGetTime (void)
{
   return cce__currentTime;
}

void calculateInternalDeltaTime (void)
{
   uint64_t currentTime = cce__engineBackend.getTime();
   cce__deltaTime       = currentTime - cce__currentTime;
   cce__currentTime     = currentTime;
}

static void terminateEngineCommon (void)
{
   cceTerminateTemporaryDirectory();
   free(iniCallbacks);
   free(terminationCallbacks);
   iniCallbacksQuantity   = 1;
   iniCallbacksAllocated  = 0;
   terminationCallbacksQuantity = 1;
   terminationCallbacksAllocated = 0;
   iniCallbackLongestName = 11;
   cce__currentTime = 0;
   cce__deltaTime = 0;
}

#define CCE_MEMEQ(x, y) (memcmp(x, y, strlen(y)) == 0)

static int iniCallback (void *data, const char *name, const char *value)
{
   char buf[27];
   strncpy(buf, name, 27);
   cceMemoryToLowercase(buf, 26);
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
      else if (!(CCE_STREQ(buf, "unchanged") || CCE_STREQ(buf, "default")))
      {
         cceSetCurrentPath(value);
      }
   }
   else if (CCE_MEMEQ(buf, "ignore"))
   {
      it = buf + 6;
      it += (-(CCE_MEMEQ(it, "notconfigured") || CCE_MEMEQ(it, "uninitialized")) & 13) | (-CCE_MEMEQ(it, "unset") & 5);
      if (CCE_STREQ(it, "plugins"))
         ignoreUninitializedPlugins = cceStringToBool(value);
   }
   return 0;
}

static int iniHandler (void *data, const char *section, const char *name, const char *value)
{
   char *buf = data;
   if (section == NULL)
      return iniCallback(iniCallbacks[commonIniCallbackID].data, name, value);
   strncpy(buf, section, iniCallbackLongestName + 1);
   cceMemoryToLowercase(buf, iniCallbackLongestName);
   for (struct iniCallbackData *it = iniCallbacks, *end = iniCallbacks + iniCallbacksQuantity; it < end; ++it)
   {
      if (memcmp(buf, it->name, it->nameLength + 1) == 0)
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
            printf("nonzero value returned by %s\n", it->name);
            #ifdef INIH_LOCAL
            return 0;
            #else
            longjmp(g_jumpOnIniFailure, -1);
            #endif
         }
         return 1;
      }
   }
   fprintf(stderr, "ENGINE::INI_PARSE_ERROR:\nUnregistered section %s\n", section);
   return 1;
}

static int parseGameINI (const char *path)
{
   int result = 0;
   cceRegisterPlugin("commonproperties", (void*)path, iniCallback, NULL, NULL, 0);
   commonIniCallbackID = iniCallbacksQuantity - 1;
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
      uint16_t termsIgnored = 0;
      uint16_t termLastIgnored = 0;
      for (uint16_t i = 0, j = 0; i < iniCallbacksQuantity; ++i)
      {
         if ((iniCallbacks[i].flags & (CCE_INI_CALLBACK_DO_NOT_INIT | CCE_INI_CALLBACK_TO_BE_INITIALIZED)) == CCE_INI_CALLBACK_TO_BE_INITIALIZED)
            result = iniCallbacks[i].init(iniCallbacks[i].data);
         else if (!ignoreUninitializedPlugins && ((iniCallbacks[i].flags & (CCE_INI_CALLBACK_DO_NOT_INIT)) == 0))
            result = iniCallbacks[i].init(iniCallbacks[i].data);
         else if (!(iniCallbacks[i].flags & CCE_INI_CALLBACK_NO_TERMINATION_CALLBACK) && !(termsIgnored == 0))
         {
            if (termsIgnored > 0)
            {
               memmove(terminationCallbacks + termLastIgnored + 1 - termsIgnored, terminationCallbacks + termLastIgnored + 1, j - termLastIgnored - 1);
            }
            terminationCallbacks[j] = NULL;
            ++termsIgnored;
            termLastIgnored = j;
         }
         if (iniCallbacks[i].flags & CCE_INI_CALLBACK_FREE_DATA)
            free(iniCallbacks[i].data);
         j += !(iniCallbacks[i].flags & CCE_INI_CALLBACK_NO_TERMINATION_CALLBACK);
         if (result == 0)
            continue;
         fprintf(stderr, "ENGINE::INIT::PLUGIN_INITIALIZATION_FAILURE:\nplugin %s failed to initialize", iniCallbacks[i].name);
         for (; i < iniCallbacksQuantity; ++i)
         {
            if (iniCallbacks[i].flags & CCE_INI_CALLBACK_FREE_DATA)
               free(iniCallbacks[i].data);
         }
         --j;
         while (j > 0)
         {
            --j;
            if (terminationCallbacks[j] != NULL)
               terminationCallbacks[j]();
         }
         terminateEngineCommon();
         return -1;
      }
      if (termsIgnored != 0)
      {
         memmove(terminationCallbacks + termLastIgnored + 1 - termsIgnored, terminationCallbacks + termLastIgnored + 1, terminationCallbacksQuantity - termLastIgnored - 1);
         terminationCallbacksQuantity -= termsIgnored;
         terminationCallbacks = realloc(terminationCallbacks, terminationCallbacksQuantity * sizeof(cce_termfun));
      }
   }
   else
   {
      result = -1;
      fputs("ENGINE::INI::PARSING_FAILURE:\nfunction indicated critical error while parsing", stderr);
      for (struct iniCallbackData *it = iniCallbacks, *end = iniCallbacks + iniCallbacksQuantity; it < end; ++it)
      {
         if (it->flags & CCE_INI_CALLBACK_FREE_DATA)
            free(it->data);
      }
      terminateEngineCommon();
   }
   free(buf);
   fclose(file);
   return result;
}

CCE_API uint8_t cceCheckPlugin (const char *name)
{
   size_t nameLength = strlen(name);
   for (struct iniCallbackData *it = iniCallbacks + 1, *end = iniCallbacks + iniCallbacksQuantity; it < end; ++it)
      if (nameLength == it->nameLength && memcmp(name, it->name, nameLength) == 0)
         return 1;
   return 0;
}

CCE_API uint8_t cceIsPluginLoading (const char *name)
{
   size_t nameLength = strlen(name);
   for (struct iniCallbackData *it = iniCallbacks, *end = iniCallbacks + iniCallbacksQuantity; it < end; ++it)
      if (nameLength == it->nameLength && memcmp(name, it->name, nameLength) == 0)
         return ((it->flags & (CCE_INI_CALLBACK_DO_NOT_INIT | CCE_INI_CALLBACK_TO_BE_INITIALIZED)) == CCE_INI_CALLBACK_TO_BE_INITIALIZED) ||
                (!ignoreUninitializedPlugins && ((it->flags & (CCE_INI_CALLBACK_DO_NOT_INIT)) == 0));
   return 0;
}

void loadBackend__glfw (void);

CCE_API int cceInit (const char *gameINIpath)
{
   uint8_t pathFree = 0;
   {
      char *path = getenv("CCE_GAME_PATH");
      if (path != NULL && *path != '\0')
      {
         if (cceIsDirectory(path))
         {
            gameINIpath = cceCreateNewPathFromOldPath(path, "game.ini", 0);
            pathFree = 1;
         }
         gameINIpath = path;
      }
   }
   if (gameINIpath == NULL || *gameINIpath == '\0')
   {
      fputs("ENGINE::INIT::NO_GAME_PATH:\nEngine could not load the game without knowing where it is", stderr);
      return -1;
   }
   moveCallbacks[0] = NULL;
   moveCallbacks[1] = NULL;
   moveCallbacks[2] = NULL;
   moveCallbacks[3] = NULL;
   buttonsCallback  = NULL;
   cce__buttonsBitField = 0;
   cce__buttonsBitFieldDiff = 0;
   ignoreUninitializedPlugins = 0;
   cceInitEndianConversion();
   // We have only one api yet
   loadBackend__glfw();
   
   int status = parseGameINI(gameINIpath);
   if (pathFree)
      free((void*)gameINIpath);
   if (status != 0)
       return status;
   cce__currentTime = cce__engineBackend.getTime();
   cce__deltaTime = 10000; // deltaTime is trash after initialization
   return 0;
}

CCE_API void cceUpdate (void)
{
   cce__engineBackend.engineUpdate();
   calculateInternalDeltaTime();
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
   for (struct updateCallbackData *it = updateCallbacks, *end = updateCallbacks + updateCallbacksQuantity; it < end; ++it)
   {
      if (it->flags & CCE_CALLBACK_ENABLED)
         it->fn();
   }
}

CCE_API void cceTerminate (void)
{
   cce_termfun *it = terminationCallbacks + terminationCallbacksQuantity; // from last to first
   do
   {
      --it;
      (*it)();
   }
   while (it > terminationCallbacks);
   terminateEngineCommon();
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

CCE_API union cce_color cceHSVtoRGB (union cce_color color)
{
   union cce_color result;
   HSV_TO_RGB(color, result, 0, color.hsv.v * color.hsv.s, color.hsv.v - chroma)
   return result;
}

CCE_API union cce_color cceHSLtoRGB (union cce_color color)
{
   union cce_color result;
   HSV_TO_RGB(color, result, 0, (255 - CCE_ABS(2 * color.hsl.l - 255)) * color.hsl.s, color.hsl.l - (chroma >> 1))
   return result;
}

// R * 0.3 + G * 0.59 + B * 0.11. Approximating this fractions with point on 2^-24
#define HCL_LUMA_RGB_SUM(rgb) ((rgb.r * 5033165 + rgb.g * 9898557 + rgb.b * 1845494) >> 24)

CCE_API union cce_color cceHCLtoRGB (union cce_color color)
{
   union cce_color result;
   HSV_TO_RGB(color, result, color.hcl.c, 0, 0);
   
   const int16_t global_light = color.hcl.l - HCL_LUMA_RGB_SUM(result.rgb);
   result.rgb.r = CCE_MAX(result.rgb.r + global_light, 255);
   result.rgb.g = CCE_MAX(result.rgb.g + global_light, 255);
   result.rgb.b = CCE_MAX(result.rgb.b + global_light, 255);
   return result;
}

CCE_API union cce_color cceHSVtoHCL (union cce_color color)
{
   union cce_color iresult;
   HSV_TO_RGB(color, iresult, 0, color.hsv.v * color.hsv.s, color.hsv.v - chroma);
   return CCE_COLOR_SET_HCL(color.hsv.h, chroma, lightness + HCL_LUMA_RGB_SUM(iresult.rgb));
}

CCE_API union cce_color cceHSLtoHCL (union cce_color color)
{
   union cce_color iresult;
   HSV_TO_RGB(color, iresult, 0, (255 - CCE_ABS(2 * color.hsl.l - 255)) * color.hsl.s, color.hsl.l - (chroma >> 1))
   return CCE_COLOR_SET_HCL(color.hsv.h, chroma, lightness + HCL_LUMA_RGB_SUM(iresult.rgb));
}

CCE_API union cce_color cceHCLtoHSV (union cce_color color)
{
   union cce_color iresult;
   HSV_TO_RGB(color, iresult, color.hcl.c, 0, 0);
   const int16_t global_light = color.hcl.l - HCL_LUMA_RGB_SUM(iresult.rgb);
   const uint8_t maxLight = CCE_MAX(chroma + global_light, 255);
   chroma = maxLight - global_light;
   return CCE_COLOR_SET_HSV(color.hcl.h, chroma * 255 / (maxLight), maxLight);
}

CCE_API union cce_color cceHCLtoHSL (union cce_color color)
{
   union cce_color iresult;
   HSV_TO_RGB(color, iresult, color.hcl.c, 0, 0);
   const int16_t global_light = color.hcl.l - HCL_LUMA_RGB_SUM(iresult.rgb);
   const uint8_t maxLight = CCE_MAX(chroma + global_light, 255);
   chroma = maxLight - global_light;
   uint8_t light = maxLight - (chroma >> 2);
   return CCE_COLOR_SET_HSL(color.hcl.h, (maxLight - light) * 255 / CCE_MIN(light, 255 - light), light);
}

CCE_API union cce_color cceHSVtoHSL (union cce_color color)
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

CCE_API union cce_color cceHSLtoHSV (union cce_color color)
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

CCE_API union cce_color cceRGBtoHSV (union cce_color color)
{
   union cce_color result;
   RGB_TO_HSV(color, result, CCE_COLOR_HSV, chroma * 255 / result.hsv.v, *max);
}

CCE_API union cce_color cceRGBtoHSL (union cce_color color)
{
   union cce_color result;
   RGB_TO_HSV(color, result, CCE_COLOR_HSL, (result.hsl.l == 255) ? 0 : (*max - result.hsl.l) * 255 / CCE_MIN(result.hsl.l, 255 - result.hsl.l), (*max + min) >> 1);
}

CCE_API union cce_color cceRGBtoHCL (union cce_color color)
{
   union cce_color result;
   RGB_TO_HSV(color, result, CCE_COLOR_HCL, chroma, min + HCL_LUMA_RGB_SUM(color.rgb));
}
