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

#include <stdio.h>
#include <stdlib.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "../../include/cce/engine_common.h"
#include "../../include/cce/utils.h"

#include "../engine_common_internal.h"

#include "engine_common_keyboard.h"

#define CCE_FULLSCREEN 0x10
#define CCE_MOVE_EVENT 0x20

#define CCE_NO_SCALING 0x0
#define CCE_INTEGER_SCALING 0x1
#define CCE_ASPECT_PRESERVING_SCALING 0x2
#define CCE_UNRESTRICTED_SCALING 0x3
#define CCE_SCALING (CCE_NO_SCALING | CCE_INTEGER_SCALING | CCE_ASPECT_PRESERVING_SCALING | CCE_UNRESTRICTED_SCALING)
#define CCE_RESIZABLE 0x4
#define CCE_VERTICAL_SYNC 0x8


GLFWwindow *g_window;
struct cce_i32vec2 g_windowResolution;
struct cce_i32vec2 g_windowBaseResolution;
uint8_t g_flags;

struct glfw_properties
{
   char *windowName;
   struct cce_u16vec2 resolution;
   uint8_t flags;
};

static uint8_t cceKeyFromGLFWkey (int16_t key);
static int16_t cceKeyToGLFWkey (uint8_t key);

static uint64_t getTime__glfw (void)
{
   uint64_t currentTime = glfwGetTimerValue();
   currentTime *= 1000000.0f / glfwGetTimerFrequency();
   return currentTime;
}

static void engineUpdate__glfw (void)
{
   glfwPollEvents();
}

static uint8_t engineShouldTerminate__glfw (void)
{
   return glfwWindowShouldClose(g_window);
}

static void setEngineShouldTerminate__glfw (uint8_t value)
{
   glfwSetWindowShouldClose(g_window, value);
}

static void framebufferSizeCallback_fixed (GLFWwindow *window, int width, int height)
{
   CCE_UNUSED(window);
   if (width == g_windowBaseResolution.x && height == g_windowBaseResolution.y)
      return; // Did not changed - fine
   struct cce_i32vec2 windowSize;
   glfwGetWindowSize(g_window, &windowSize.x, &windowSize.y);
   glfwSetWindowSize(g_window, g_windowBaseResolution.x * windowSize.x / width, g_windowBaseResolution.y * windowSize.y / height); // Change window resolution to match framebuffer resolution
}

static void framebufferSizeCallback_integer (GLFWwindow *window, int width, int height)
{
   CCE_UNUSED(window);
   uint16_t k = CCE_MIN(width / g_windowBaseResolution.x, height / g_windowBaseResolution.y);
   struct cce_u16vec2 resolution = {g_windowBaseResolution.x * k, g_windowBaseResolution.y * k};
   glViewport((width - resolution.x)/2, (height - resolution.y)/2, resolution.x, resolution.y);
}

static void framebufferSizeCallback_unrestricted (GLFWwindow *window, int width, int height)
{
   CCE_UNUSED(window);
   glViewport(0, 0, width, height);
}

void swapBuffers__glfw (void)
{
   glfwSwapBuffers(g_window);
}

static struct cce_u8vec2 getAspectRatio (uint16_t width, uint16_t height)
{
   if (!width || !height)
   {
      return (struct cce_u8vec2) {0u, 0u};
   }
   uint32_t ratio = (width * 100u) / height;
   if (ratio < 100u/1u)
   {
      ratio = (height * 100u) / width;
      if (ratio < 800u/5u)
      {
         if (ratio < 400u/3u)
         {
            if (ratio < 500u/4u)
               return (struct cce_u8vec2) {1u, 1u};
            else
               return (struct cce_u8vec2) {4u, 5u};
         }
         else
         {
            if (ratio < 300u/2u)
            {
               return (struct cce_u8vec2) {3u, 4u};
            }
            else
            {
               if (ratio < 1400u/9u)
                  return (struct cce_u8vec2) {2u, 3u};
               else
                  return (struct cce_u8vec2) {9u, 14u};
            }
         }
      }
      else
      {
         if (ratio < 1600u/9u)
         {
            if (ratio < 500u/3u)
               return (struct cce_u8vec2) {5u, 8u};
            else
               return (struct cce_u8vec2) {3u, 5u};
         }
         else
         {
            if (ratio < 200u/1u)
            {
               return (struct cce_u8vec2) {9u, 16u};
            }
            else
            {
               if (ratio < 700u/3u)
                  return (struct cce_u8vec2) {1u, 2u};
               else
                  return (struct cce_u8vec2) {3u, 7u};
            }
         }
      }
   }
   else
   {
      if (ratio < 800u/5u)
      {
         if (ratio < 400u/3u)
         {
            if (ratio < 500u/4u)
               return (struct cce_u8vec2) {1u, 1u};
            else
               return (struct cce_u8vec2) {5u, 4u};
         }
         else
         {
            if (ratio < 300u/2u)
            {
               return (struct cce_u8vec2) {4u, 3u};
            }
            else
            {
               if (ratio < 1400u/9u)
                  return (struct cce_u8vec2) {3u, 2u};
               else
                  return (struct cce_u8vec2) {14u, 9u};
            }
         }
      }
      else
      {
         if (ratio < 1600u/9u)
         {
            if (ratio < 500u/3u)
               return (struct cce_u8vec2) {8u, 5u};
            else
               return (struct cce_u8vec2) {5u, 3u};
         }
         else
         {
            if (ratio < 200u/1u)
            {
               return (struct cce_u8vec2) {16u, 9u};
            }
            else
            {
               if (ratio < 700u/3u)
                  return (struct cce_u8vec2) {2u, 1u};
               else
                  return (struct cce_u8vec2) {7u, 3u};
            }
         }
      }
   }
}

GLFWmonitor* getClosestMonitor (void)
{
   struct cce_i32vec4 windowProps;
   struct cce_i32vec4 monitorPos;
   glfwGetWindowPos(g_window, &windowProps.x, &windowProps.y);
   glfwGetWindowSize(g_window, &windowProps.z, &windowProps.w);
   int monitorQuantity;
   int32_t overlap = 0, bestOverlap = 0;
   GLFWmonitor *closest = NULL;
   for (GLFWmonitor **it = glfwGetMonitors(&monitorQuantity), **end = it + monitorQuantity; it < end; ++it)
   {
      const GLFWvidmode *mode = glfwGetVideoMode(*it);
      glfwGetMonitorPos(*it, &monitorPos.x, &monitorPos.y);
      overlap = ((int32_t)CCE_MIN(windowProps.x + windowProps.z, monitorPos.x + mode->width) - (int32_t)CCE_MAX(windowProps.x, monitorPos.x)) * 
                ((int32_t)CCE_MIN(windowProps.y + windowProps.w, monitorPos.y + mode->height) - (int32_t)CCE_MAX(windowProps.y, monitorPos.y));
      if (overlap <= bestOverlap)
         continue;
      
      bestOverlap = overlap;
      closest = *it;
      if (overlap == windowProps.z * windowProps.w)
         break;
   }
   return closest;
}

static void toFullscreen__glfw (void)
{
   GLFWmonitor *monitor = getClosestMonitor();
   const GLFWvidmode *vidMode = glfwGetVideoMode(monitor);
   glfwGetWindowSize(g_window, &g_windowResolution.x, &g_windowResolution.y);
   switch (g_flags & CCE_SCALING)
   {
      case CCE_NO_SCALING:
         glfwSetWindowMonitor(g_window, monitor, 0, 0, g_windowResolution.x, g_windowResolution.y, vidMode->refreshRate);
         break;
      case CCE_ASPECT_PRESERVING_SCALING:
         glfwSetFramebufferSizeCallback(g_window, NULL);
         glfwSetWindowMonitor(g_window, monitor, 0, 0, vidMode->width, vidMode->height, vidMode->refreshRate);
         glfwSetFramebufferSizeCallback(g_window, framebufferSizeCallback_unrestricted);
         framebufferSizeCallback_integer(g_window, vidMode->width, vidMode->height); // value in g_windowBaseResolution is actually an aspect ratio if this scaling mode is active
         break;
      case CCE_INTEGER_SCALING:
      case CCE_UNRESTRICTED_SCALING:
         glfwSetWindowMonitor(g_window, monitor, 0, 0, vidMode->width, vidMode->height, vidMode->refreshRate);
         break;
   }
   
   g_flags |= CCE_FULLSCREEN;
}

static void toWindow__glfw (void)
{
   glfwSetWindowMonitor(g_window, NULL, 0, 0, g_windowResolution.x, g_windowResolution.y, GLFW_DONT_CARE);
   g_flags &= ~CCE_FULLSCREEN;
}

static int8_t verticalControlAxis;
static int8_t horizontalControlAxis;

static void keyCallback (GLFWwindow *window, int key, int scancode, int action, int modifiers)
{
   CCE_UNUSED(modifiers);
   CCE_UNUSED(window);
   CCE_UNUSED(scancode);
   if (action == GLFW_REPEAT)
      return;
   switch (key)
   {
      case GLFW_KEY_UP:
      case GLFW_KEY_DOWN:
         verticalControlAxis = (key * 2 - GLFW_KEY_DOWN * 2 - 1) * (action == GLFW_PRESS);
         g_flags |= CCE_MOVE_EVENT;
         break;
      case GLFW_KEY_LEFT:
      case GLFW_KEY_RIGHT:
         horizontalControlAxis = (GLFW_KEY_RIGHT * 2 - key * 2 + 1) * (action == GLFW_PRESS);
         g_flags |= CCE_MOVE_EVENT;
         break;
      case GLFW_KEY_ENTER:
      case GLFW_KEY_X:
         break;
      case GLFW_KEY_RIGHT_SHIFT:
      case GLFW_KEY_Z:
         break;
      case GLFW_KEY_RIGHT_CONTROL:
      case GLFW_KEY_C:
         break;
      case GLFW_KEY_DELETE:
      case GLFW_KEY_V:
         break;
      case GLFW_KEY_SPACE:
         break;
      case GLFW_KEY_TAB:
      case GLFW_KEY_D:
         break;
      case GLFW_KEY_F4:
      case GLFW_KEY_F11:
         if (action != GLFW_PRESS)
            break;
         if (g_flags & CCE_FULLSCREEN)
            toWindow__glfw();
         else
            toFullscreen__glfw();
         break;
   }
}

static void terminateEngine__glfw (void);

int initEngine__glfw (void *data)
{
   struct glfw_properties *vals = data;
   if (glfwInit() != GLFW_TRUE)
   {
      return -1;
   }
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
   glfwWindowHint(GLFW_AUTO_ICONIFY,   GLFW_TRUE);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   if ((vals->flags & (CCE_SCALING | CCE_RESIZABLE)) == CCE_RESIZABLE)
   {
      fprintf(stderr, "ENGINE::BACKEND::INCOMPATIBLE_OPTIONS_ERROR:\nResizable window specified without scaling (not supported).\n");
      vals->flags &= ~CCE_RESIZABLE;
   }
   glfwWindowHint(GLFW_RESIZABLE, (vals->flags & CCE_RESIZABLE) > 0);

#ifdef __APPLE__
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
   glfwWindowHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
   glfwWindowHint(GLFW_COCOA_MENUBAR,         GLFW_FALSE);
#endif
   if (vals->flags & CCE_SCALING)
      glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
#if defined(NDEBUG)
   glfwWindowHint(GLFW_CONTEXT_NO_ERROR,      GLFW_TRUE);
#else
   glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,  GLFW_TRUE);
#endif
   
   if (vals->windowName == NULL || *vals->windowName == '\0')
   {
      vals->windowName = CCE_DEFAULT_WINDOW_LABEL;
   }
   
   g_window = glfwCreateWindow(vals->resolution.x, vals->resolution.y, vals->windowName, NULL, NULL);
   if (g_window == NULL)
   {
      // it's perfectly fine to not support this extensions.
#if defined(NDEBUG)
      glfwWindowHint(GLFW_CONTEXT_NO_ERROR,      GLFW_FALSE);
#else
      glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,  GLFW_FALSE);
#endif
      g_window = glfwCreateWindow(vals->resolution.x, vals->resolution.y, vals->windowName, NULL, NULL);
      if (g_window == NULL)
      {
         fprintf(stderr, "GLFW::WINDOW::FAILED_TO_CREATE\n");
         glfwTerminate();
         return -1;
      }
   }
   glfwMakeContextCurrent(g_window);
   glfwSetWindowSizeLimits(g_window, vals->resolution.x, vals->resolution.y, GLFW_DONT_CARE, GLFW_DONT_CARE);
   
   if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
   {
      fprintf(stderr, "GLAD::INITIALIZATION::FAILED:\nOpenGL could not be loaded by GLAD.\n");
      return -1;
   }
   if (!GLAD_GL_VERSION_3_2)
   {
      fprintf(stderr, "GLAD::INITIALIZATION::FAILED:\nOpenGL 3.2 (minimum required) could not be loaded by GLAD.\n");
      return -1;
   }
   
   struct cce_i32vec2 fbSize;
   glfwGetFramebufferSize(g_window, &fbSize.x, &fbSize.y);
   
   switch (vals->flags & CCE_SCALING)
   {
      case CCE_INTEGER_SCALING:
         glfwSetFramebufferSizeCallback(g_window, framebufferSizeCallback_integer);
         g_windowBaseResolution = (struct cce_i32vec2){vals->resolution.x * vals->resolution.x / fbSize.x, vals->resolution.y * vals->resolution.y / fbSize.y};
         glfwSetWindowSize(g_window, g_windowBaseResolution.x, g_windowBaseResolution.y);
         break;
      case CCE_NO_SCALING:
         glfwSetFramebufferSizeCallback(g_window, framebufferSizeCallback_fixed);
         g_windowBaseResolution = (struct cce_i32vec2){vals->resolution.x, vals->resolution.y};
         g_windowResolution = (struct cce_i32vec2){vals->resolution.x * vals->resolution.x / fbSize.x, vals->resolution.y * vals->resolution.y / fbSize.y};
         glfwSetWindowSize(g_window, g_windowResolution.x, g_windowResolution.y);
         break;
      case CCE_ASPECT_PRESERVING_SCALING:
         glfwSetWindowAspectRatio(g_window, vals->resolution.x, vals->resolution.y);
         struct cce_u8vec2 tmp = getAspectRatio(vals->resolution.x, vals->resolution.y);
         g_windowBaseResolution = (struct cce_i32vec2){tmp.x, tmp.y};
         // fallthrough
      case CCE_UNRESTRICTED_SCALING:
         glfwSetFramebufferSizeCallback(g_window, framebufferSizeCallback_unrestricted);
         break;
   }
   glViewport(0, 0, fbSize.x, fbSize.y);
   glfwSetKeyCallback(g_window, keyCallback);
   glfwSwapInterval((vals->flags & CCE_VERTICAL_SYNC) > 0);
   
   cce__engineBackend.screenUpdate = swapBuffers__glfw;
   cce__engineBackend.toWindow = toWindow__glfw;
   cce__engineBackend.toFullscreen = toFullscreen__glfw;
   cce__engineBackend.terminateEngine = terminateEngine__glfw;
   cce__engineBackend.engineUpdate = engineUpdate__glfw;
   cce__engineBackend.getTime = getTime__glfw;
   cceEngineShouldTerminate = engineShouldTerminate__glfw;
   cceSetEngineShouldTerminate = setEngineShouldTerminate__glfw;
   verticalControlAxis = 0;
   horizontalControlAxis = 0;
   return 0;
}

static void terminateEngine__glfw (void)
{
   glfwDestroyWindow(g_window);
   glfwTerminate();
}

#define CCE_MEMEQ(x, y) (memcmp(x, y, strlen(y)) == 0)

static int iniCallback__glfw (void *data, const char *name, const char *value)
{
   struct glfw_properties *vals = data;
   char buf[24];
   strncpy(buf, name, 24);
   cceMemoryToLowercase(buf, 23);
   char *it;
   if (CCE_STREQ(buf, "gameres") || CCE_STREQ(buf, "res") || CCE_STREQ(buf, "gameresolution") || CCE_STREQ(buf, "resolution"))
   {
      vals->resolution = cceStringToU16Vec2(value);
   }
   else if (CCE_STREQ(buf, "windowname") || CCE_STREQ(buf, "name"))
   {
      size_t len = strlen(value);
      vals->windowName = malloc((len + 1) * sizeof(char));
      memcpy(vals->windowName, value, len + 1);
   }
   else if (CCE_STREQ(buf, "scaling") || CCE_STREQ(buf, "scale") || CCE_STREQ(buf, "scalingtype"))
   {
      vals->flags &= ~CCE_SCALING;
      strncpy(buf, value, 15);
      cceMemoryToLowercase(buf, 14);
      if (CCE_STREQ(buf, "no") || CCE_STREQ(buf, "disabled"))
      {
         vals->flags |= CCE_NO_SCALING;
      }
      else if (CCE_STREQ(buf, "integer") || (CCE_MEMEQ(buf, "pixel") && CCE_STREQ(buf + 5 + cceIsCharWhitespaceLike(buf[5]), "perfect")))
      {
         vals->flags |= CCE_INTEGER_SCALING;
      }
      else if (CCE_STREQ(buf, "unrestricted") || CCE_STREQ(buf, "free"))
      {
         vals->flags |= CCE_UNRESTRICTED_SCALING;
      }
      else
      {
         it = buf + (-CCE_MEMEQ(buf, "keep") & 4) + (-CCE_MEMEQ(buf, "preserve") & 8);
         if (it == buf)
            goto ERROR;
         it += cceIsCharWhitespaceLike(*it);
         if (CCE_MEMEQ(it, "aspect") && (it[6] == '\0' || CCE_STREQ(it + 6 + cceIsCharWhitespaceLike(it[6]), "ratio")))
            vals->flags |= CCE_ASPECT_PRESERVING_SCALING;
         else
            goto ERROR;
      }
   }
   else if (CCE_STREQ(buf, "resize") || CCE_STREQ(buf, "resizable") || CCE_STREQ(buf, "manualresize"))
   {
      vals->flags &= ~CCE_RESIZABLE;
      vals->flags |= -cceStringToBool(value) & CCE_RESIZABLE;
   } // vertical syncronization
   else if (CCE_STREQ(buf, "vsync") || (CCE_MEMEQ(buf, "vertical") && (buf[8] == '\0' || (CCE_MEMEQ(it = buf + 8 + cceIsCharWhitespaceLike(buf[8]), "sync") && (it[4] == '\0' || CCE_STREQ(it + 4, "ronization"))))))
   {
      vals->flags &= ~CCE_VERTICAL_SYNC;
      vals->flags |= -cceStringToBool(value) & CCE_VERTICAL_SYNC;
   }
   else
   {
ERROR:
      fprintf(stderr, "ENGINE::INI::WINDOW_PROPERTIES_PARSE_ERROR:\nUnknown entry %s\n", name);
   }
   return 0;
}

void loadBackend__glfw (void)
{
   struct glfw_properties *props = malloc(sizeof(struct glfw_properties));
   props->windowName = NULL;
   props->resolution = (struct cce_u16vec2){640, 480};
   props->flags = 0;
   const char *names[4] = {"window", "windowprops", "windowproperties", NULL};
   cceRegisterIniCallback(names, props, iniCallback__glfw, initEngine__glfw, CCE_INI_CALLBACK_FREE_DATA);
}

static int16_t cceKeyToGLFWkey (uint8_t key)
{
   switch (key)
   {
      case CCE_KEY_NOKEY:
         return 0;
      case CCE_KEY_PAGEUP:
         return GLFW_KEY_PAGE_UP;
      case CCE_KEY_PAGEDOWN:
         return GLFW_KEY_PAGE_DOWN;
      case CCE_KEY_HOME:
         return GLFW_KEY_HOME;
      case CCE_KEY_END:
         return GLFW_KEY_END;
      case CCE_KEY_INSERT:
         return GLFW_KEY_INSERT;
      case CCE_KEY_DELETE:
         return GLFW_KEY_DELETE;
      case CCE_KEY_SPACE:
         return GLFW_KEY_SPACE;
      case CCE_KEY_BACKSPACE:
         return GLFW_KEY_BACKSPACE;
      case CCE_KEY_ENTER:
         return GLFW_KEY_ENTER;
      case CCE_KEY_APOSTROPHE:
         return GLFW_KEY_APOSTROPHE;
      case CCE_KEY_COMMA:
         return GLFW_KEY_COMMA;
      case CCE_KEY_MINUS:
         return GLFW_KEY_MINUS;
      case CCE_KEY_PERIOD:
         return GLFW_KEY_PERIOD;
      case CCE_KEY_SLASH:
         return GLFW_KEY_SLASH;
      case CCE_KEY_0:
         return GLFW_KEY_0;
      case CCE_KEY_1:
         return GLFW_KEY_1;
      case CCE_KEY_2:
         return GLFW_KEY_2;
      case CCE_KEY_3:
         return GLFW_KEY_3;
      case CCE_KEY_4:
         return GLFW_KEY_4;
      case CCE_KEY_5:
         return GLFW_KEY_5;
      case CCE_KEY_6:
         return GLFW_KEY_6;
      case CCE_KEY_7:
         return GLFW_KEY_7;
      case CCE_KEY_8:
         return GLFW_KEY_8;
      case CCE_KEY_9:
         return GLFW_KEY_9;
      case CCE_KEY_SEMICOLON:
         return GLFW_KEY_SEMICOLON;
      case CCE_KEY_EQUAL:
         return GLFW_KEY_EQUAL;
      case CCE_KEY_A:
         return GLFW_KEY_A;
      case CCE_KEY_B:
         return GLFW_KEY_B;
      case CCE_KEY_C:
         return GLFW_KEY_C;
      case CCE_KEY_D:
         return GLFW_KEY_D;
      case CCE_KEY_E:
         return GLFW_KEY_E;
      case CCE_KEY_F:
         return GLFW_KEY_F;
      case CCE_KEY_G:
         return GLFW_KEY_G;
      case CCE_KEY_H:
         return GLFW_KEY_H;
      case CCE_KEY_I:
         return GLFW_KEY_I;
      case CCE_KEY_J:
         return GLFW_KEY_J;
      case CCE_KEY_K:
         return GLFW_KEY_K;
      case CCE_KEY_L:
         return GLFW_KEY_L;
      case CCE_KEY_M:
         return GLFW_KEY_M;
      case CCE_KEY_N:
         return GLFW_KEY_N;
      case CCE_KEY_O:
         return GLFW_KEY_O;
      case CCE_KEY_P:
         return GLFW_KEY_P;
      case CCE_KEY_Q:
         return GLFW_KEY_Q;
      case CCE_KEY_R:
         return GLFW_KEY_R;
      case CCE_KEY_S:
         return GLFW_KEY_S;
      case CCE_KEY_T:
         return GLFW_KEY_T;
      case CCE_KEY_U:
         return GLFW_KEY_U;
      case CCE_KEY_V:
         return GLFW_KEY_V;
      case CCE_KEY_W:
         return GLFW_KEY_W;
      case CCE_KEY_X:
         return GLFW_KEY_X;
      case CCE_KEY_Y:
         return GLFW_KEY_Y;
      case CCE_KEY_Z:
         return GLFW_KEY_Z;
      case CCE_KEY_LEFT_BRACKET:
         return GLFW_KEY_LEFT_BRACKET;
      case CCE_KEY_RIGHT_BRACKET:
         return GLFW_KEY_RIGHT_BRACKET;
      case CCE_KEY_BACKSLASH:
         return GLFW_KEY_BACKSLASH;
      case CCE_KEY_GRAVE_ACCENT:
         return GLFW_KEY_GRAVE_ACCENT;
      case CCE_KEY_LEFT_ARROW:
         return GLFW_KEY_LEFT;
      case CCE_KEY_RIGHT_ARROW:
         return GLFW_KEY_RIGHT;
      case CCE_KEY_UP_ARROW:
         return GLFW_KEY_UP;
      case CCE_KEY_DOWN_ARROW:
         return GLFW_KEY_DOWN;
      case CCE_KEY_LEFT_SHIFT:
         return GLFW_KEY_LEFT_SHIFT;
      case CCE_KEY_RIGHT_SHIFT:
         return GLFW_KEY_RIGHT_SHIFT;
      case CCE_KEY_LEFT_ALT:
         return GLFW_KEY_LEFT_ALT;
      case CCE_KEY_RIGHT_ALT:
         return GLFW_KEY_RIGHT_ALT;
      case CCE_KEY_LEFT_SUPER:
         return GLFW_KEY_LEFT_SUPER;
      case CCE_KEY_RIGHT_SUPER:
         return GLFW_KEY_RIGHT_SUPER;
      case CCE_KEY_NUMLOCK:
         return GLFW_KEY_NUM_LOCK;
      case CCE_KEY_CAPSLOCK:
         return GLFW_KEY_CAPS_LOCK;
      case CCE_KEY_SCROLLLOCK:
         return GLFW_KEY_SCROLL_LOCK;
      case CCE_KEY_PRINTSCREEN:
         return GLFW_KEY_PRINT_SCREEN;
      case CCE_KEY_KP_ENTER:
         return GLFW_KEY_KP_ENTER;
      case CCE_KEY_KP_DIVIDE:
         return GLFW_KEY_KP_DIVIDE;
      case CCE_KEY_KP_MULTIPLY:
         return GLFW_KEY_KP_MULTIPLY;
      case CCE_KEY_KP_MINUS:
         return GLFW_KEY_KP_SUBTRACT;
      case CCE_KEY_KP_PLUS:
         return GLFW_KEY_KP_ADD;
      case CCE_KEY_KP_0:
         return GLFW_KEY_KP_0;
      case CCE_KEY_KP_1:
         return GLFW_KEY_KP_1;
      case CCE_KEY_KP_2:
         return GLFW_KEY_KP_2;
      case CCE_KEY_KP_3:
         return GLFW_KEY_KP_3;
      case CCE_KEY_KP_4:
         return GLFW_KEY_KP_4;
      case CCE_KEY_KP_5:
         return GLFW_KEY_KP_5;
      case CCE_KEY_KP_6:
         return GLFW_KEY_KP_6;
      case CCE_KEY_KP_7:
         return GLFW_KEY_KP_7;
      case CCE_KEY_KP_8:
         return GLFW_KEY_KP_8;
      case CCE_KEY_KP_9:
         return GLFW_KEY_KP_9;
      case CCE_KEY_F1:
         return GLFW_KEY_F1;
      case CCE_KEY_F2:
         return GLFW_KEY_F2;
      case CCE_KEY_F3:
         return GLFW_KEY_F3;
      case CCE_KEY_F4:
         return GLFW_KEY_F4;
      case CCE_KEY_F5:
         return GLFW_KEY_F5;
      case CCE_KEY_F6:
         return GLFW_KEY_F6;
      case CCE_KEY_F7:
         return GLFW_KEY_F7;
      case CCE_KEY_F8:
         return GLFW_KEY_F8;
      case CCE_KEY_F9:
         return GLFW_KEY_F9;
      case CCE_KEY_F10:
         return GLFW_KEY_F10;
      case CCE_KEY_F11:
         return GLFW_KEY_F11;
      case CCE_KEY_F12:
         return GLFW_KEY_F12;
      case CCE_KEY_F13:
         return GLFW_KEY_F13;
      case CCE_KEY_F14:
         return GLFW_KEY_F14;
      case CCE_KEY_F15:
         return GLFW_KEY_F15;
      case CCE_KEY_F16:
         return GLFW_KEY_F16;
      case CCE_KEY_F17:
         return GLFW_KEY_F17;
      case CCE_KEY_F18:
         return GLFW_KEY_F18;
      case CCE_KEY_F19:
         return GLFW_KEY_F19;
      case CCE_KEY_F20:
         return GLFW_KEY_F20;
      case CCE_KEY_F21:
         return GLFW_KEY_F21;
      case CCE_KEY_F22:
         return GLFW_KEY_F22;
      case CCE_KEY_F23:
         return GLFW_KEY_F23;
      case CCE_KEY_F24:
         return GLFW_KEY_F24;
      case CCE_KEY_F25:
         return GLFW_KEY_F25;
      default:
         return GLFW_KEY_UNKNOWN;
   }
}

static uint8_t cceKeyFromGLFWkey (int16_t key)
{
   switch (key)
   {
      case 0:
         return CCE_KEY_NOKEY;
      case GLFW_KEY_PAGE_UP:
         return CCE_KEY_PAGEUP;
      case GLFW_KEY_PAGE_DOWN:
         return CCE_KEY_PAGEDOWN;
      case GLFW_KEY_HOME:
         return CCE_KEY_HOME;
      case GLFW_KEY_END:
         return CCE_KEY_END;
      case GLFW_KEY_INSERT:
         return CCE_KEY_INSERT;
      case GLFW_KEY_DELETE:
         return CCE_KEY_DELETE;
      case GLFW_KEY_SPACE:
         return CCE_KEY_SPACE;
      case GLFW_KEY_BACKSPACE:
         return CCE_KEY_BACKSPACE;
      case GLFW_KEY_ENTER:
         return CCE_KEY_ENTER;
      case GLFW_KEY_APOSTROPHE:
         return CCE_KEY_APOSTROPHE;
      case GLFW_KEY_COMMA:
         return CCE_KEY_COMMA;
      case GLFW_KEY_MINUS:
         return CCE_KEY_MINUS;
      case GLFW_KEY_PERIOD:
         return CCE_KEY_PERIOD;
      case GLFW_KEY_SLASH:
         return CCE_KEY_SLASH;
      case GLFW_KEY_0:
         return CCE_KEY_0;
      case GLFW_KEY_1:
         return CCE_KEY_1;
      case GLFW_KEY_2:
         return CCE_KEY_2;
      case GLFW_KEY_3:
         return CCE_KEY_3;
      case GLFW_KEY_4:
         return CCE_KEY_4;
      case GLFW_KEY_5:
         return CCE_KEY_5;
      case GLFW_KEY_6:
         return CCE_KEY_6;
      case GLFW_KEY_7:
         return CCE_KEY_7;
      case GLFW_KEY_8:
         return CCE_KEY_8;
      case GLFW_KEY_9:
         return CCE_KEY_9;
      case GLFW_KEY_SEMICOLON:
         return CCE_KEY_SEMICOLON;
      case GLFW_KEY_EQUAL:
         return CCE_KEY_EQUAL;
      case GLFW_KEY_A:
         return CCE_KEY_A;
      case GLFW_KEY_B:
         return CCE_KEY_B;
      case GLFW_KEY_C:
         return CCE_KEY_C;
      case GLFW_KEY_D:
         return CCE_KEY_D;
      case GLFW_KEY_E:
         return CCE_KEY_E;
      case GLFW_KEY_F:
         return CCE_KEY_F;
      case GLFW_KEY_G:
         return CCE_KEY_G;
      case GLFW_KEY_H:
         return CCE_KEY_H;
      case GLFW_KEY_I:
         return CCE_KEY_I;
      case GLFW_KEY_J:
         return CCE_KEY_J;
      case GLFW_KEY_K:
         return CCE_KEY_K;
      case GLFW_KEY_L:
         return CCE_KEY_L;
      case GLFW_KEY_M:
         return CCE_KEY_M;
      case GLFW_KEY_N:
         return CCE_KEY_N;
      case GLFW_KEY_O:
         return CCE_KEY_O;
      case GLFW_KEY_P:
         return CCE_KEY_P;
      case GLFW_KEY_Q:
         return CCE_KEY_Q;
      case GLFW_KEY_R:
         return CCE_KEY_R;
      case GLFW_KEY_S:
         return CCE_KEY_S;
      case GLFW_KEY_T:
         return CCE_KEY_T;
      case GLFW_KEY_U:
         return CCE_KEY_U;
      case GLFW_KEY_V:
         return CCE_KEY_V;
      case GLFW_KEY_W:
         return CCE_KEY_W;
      case GLFW_KEY_X:
         return CCE_KEY_X;
      case GLFW_KEY_Y:
         return CCE_KEY_Y;
      case GLFW_KEY_Z:
         return CCE_KEY_Z;
      case GLFW_KEY_LEFT_BRACKET:
         return CCE_KEY_LEFT_BRACKET;
      case GLFW_KEY_RIGHT_BRACKET:
         return CCE_KEY_RIGHT_BRACKET;
      case GLFW_KEY_BACKSLASH:
         return CCE_KEY_BACKSLASH;
      case GLFW_KEY_GRAVE_ACCENT:
         return CCE_KEY_GRAVE_ACCENT;
      case GLFW_KEY_LEFT:
         return CCE_KEY_LEFT_ARROW;
      case GLFW_KEY_RIGHT:
         return CCE_KEY_RIGHT_ARROW;
      case GLFW_KEY_UP:
         return CCE_KEY_UP_ARROW;
      case GLFW_KEY_DOWN:
         return CCE_KEY_DOWN_ARROW;
      case GLFW_KEY_LEFT_SHIFT:
         return CCE_KEY_LEFT_SHIFT;
      case GLFW_KEY_RIGHT_SHIFT:
         return CCE_KEY_RIGHT_SHIFT;
      case GLFW_KEY_LEFT_ALT:
         return CCE_KEY_LEFT_ALT;
      case GLFW_KEY_RIGHT_ALT:
         return CCE_KEY_RIGHT_ALT;
      case GLFW_KEY_LEFT_SUPER:
         return CCE_KEY_LEFT_SUPER;
      case GLFW_KEY_RIGHT_SUPER:
         return CCE_KEY_RIGHT_SUPER;
      case GLFW_KEY_NUM_LOCK:
         return CCE_KEY_NUMLOCK;
      case GLFW_KEY_CAPS_LOCK:
         return CCE_KEY_CAPSLOCK;
      case GLFW_KEY_SCROLL_LOCK:
         return CCE_KEY_SCROLLLOCK;
      case GLFW_KEY_PRINT_SCREEN:
         return CCE_KEY_PRINTSCREEN;
      case GLFW_KEY_KP_ENTER:
         return CCE_KEY_KP_ENTER;
      case GLFW_KEY_KP_DIVIDE:
         return CCE_KEY_KP_DIVIDE;
      case GLFW_KEY_KP_MULTIPLY:
         return CCE_KEY_KP_MULTIPLY;
      case GLFW_KEY_KP_SUBTRACT:
         return CCE_KEY_KP_MINUS;
      case GLFW_KEY_KP_ADD:
         return CCE_KEY_KP_PLUS;
      case GLFW_KEY_KP_0:
         return CCE_KEY_KP_0;
      case GLFW_KEY_KP_1:
         return CCE_KEY_KP_1;
      case GLFW_KEY_KP_2:
         return CCE_KEY_KP_2;
      case GLFW_KEY_KP_3:
         return CCE_KEY_KP_3;
      case GLFW_KEY_KP_4:
         return CCE_KEY_KP_4;
      case GLFW_KEY_KP_5:
         return CCE_KEY_KP_5;
      case GLFW_KEY_KP_6:
         return CCE_KEY_KP_6;
      case GLFW_KEY_KP_7:
         return CCE_KEY_KP_7;
      case GLFW_KEY_KP_8:
         return CCE_KEY_KP_8;
      case GLFW_KEY_KP_9:
         return CCE_KEY_KP_9;
      case GLFW_KEY_F1:
         return CCE_KEY_F1;
      case GLFW_KEY_F2:
         return CCE_KEY_F2;
      case GLFW_KEY_F3:
         return CCE_KEY_F3;
      case GLFW_KEY_F4:
         return CCE_KEY_F4;
      case GLFW_KEY_F5:
         return CCE_KEY_F5;
      case GLFW_KEY_F6:
         return CCE_KEY_F6;
      case GLFW_KEY_F7:
         return CCE_KEY_F7;
      case GLFW_KEY_F8:
         return CCE_KEY_F8;
      case GLFW_KEY_F9:
         return CCE_KEY_F9;
      case GLFW_KEY_F10:
         return CCE_KEY_F10;
      case GLFW_KEY_F11:
         return CCE_KEY_F11;
      case GLFW_KEY_F12:
         return CCE_KEY_F12;
      case GLFW_KEY_F13:
         return CCE_KEY_F13;
      case GLFW_KEY_F14:
         return CCE_KEY_F14;
      case GLFW_KEY_F15:
         return CCE_KEY_F15;
      case GLFW_KEY_F16:
         return CCE_KEY_F16;
      case GLFW_KEY_F17:
         return CCE_KEY_F17;
      case GLFW_KEY_F18:
         return CCE_KEY_F18;
      case GLFW_KEY_F19:
         return CCE_KEY_F19;
      case GLFW_KEY_F20:
         return CCE_KEY_F20;
      case GLFW_KEY_F21:
         return CCE_KEY_F21;
      case GLFW_KEY_F22:
         return CCE_KEY_F22;
      case GLFW_KEY_F23:
         return CCE_KEY_F23;
      case GLFW_KEY_F24:
         return CCE_KEY_F24;
      case GLFW_KEY_F25:
         return CCE_KEY_F25;
      default:
         return CCE_KEY_UNKNOWN;
   }
}
