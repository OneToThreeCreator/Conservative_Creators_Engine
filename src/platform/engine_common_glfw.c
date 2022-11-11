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

#define CCE_FULLSCREEN 0x10

static struct
{
   GLFWwindow *window;
   const GLFWvidmode *vidMode;
   unsigned int windowWidth;
   unsigned int windowHeight;
   struct cce_u32vec2 monitorAspectRatio;
   int windowPositionX;
   int windowPositionY;
   uint8_t flags;
} g_GLFWstate;

static struct RegisteredKeys
{
   int key;
   int modifiers;
   uint16_t eventType; // 0x0 - boolChange, 0x1 - plotNumber, 0x2 - systemAction
   uint16_t number;
} *cce_keys;
static size_t keysQuantity = 0u;
//static size_t keysQuantityAllocated = CCE_ALLOCATION_STEP;

static uint8_t engineFlags = 0u;
const uint8_t *const cce__flags = &engineFlags;
static uint8_t internalFlags = 0u;

static uint64_t getTime__glfw (void)
{
   uint64_t currentTime = glfwGetTimerValue();
   currentTime *= 1000000.0f / glfwGetTimerFrequency();
   return currentTime;
}

static void engineUpdate__glfw (void)
{
   glfwPollEvents();
   if (glfwWindowShouldClose(g_GLFWstate.window))
   {
      engineFlags |= CCE_ENGINE_STOP;
   }
}

static void framebufferSizeCallback (GLFWwindow *window, int width, int height)
{
   CCE_UNUSED(window);
   glViewport(0, 0, width, height);
}

static void windowResizeCallback (GLFWwindow *window, int width, int height)
{
   CCE_UNUSED(window);
   g_GLFWstate.windowWidth = width;
   g_GLFWstate.windowHeight = height;
}

void swapBuffers__glfw (void)
{
   glfwSwapBuffers(g_GLFWstate.window);
}

static size_t registerKey__glfw (int glfwKey, int glfwKeyModifiers, uint16_t eventType, uint16_t number)
{
   //if (keysQuantity >= keysQuantityAllocated)
   //{
   //   keysQuantityAllocated += CCE_ALLOCATION_STEP;
   //   cce_keys = realloc(cce_keys, keysQuantityAllocated);
   //}
   (cce_keys + keysQuantity)->key = glfwKey;
   (cce_keys + keysQuantity)->modifiers = glfwKeyModifiers;
   (cce_keys + keysQuantity)->eventType = eventType;
   (cce_keys + keysQuantity)->number = number;
   return keysQuantity++;
}

static void toFullscreen__glfw (void)
{
   GLFWmonitor *monitor = glfwGetPrimaryMonitor();
   g_GLFWstate.vidMode = glfwGetVideoMode(monitor);
   uint32_t aspectRatio = 0u, monitorAspectRatio = 0u;
   unsigned int width, height;
   glfwSetWindowSizeCallback(g_GLFWstate.window, NULL);
   width = g_GLFWstate.windowWidth;
   height = g_GLFWstate.windowHeight;
   glfwGetWindowPos(g_GLFWstate.window, &g_GLFWstate.windowPositionX, &g_GLFWstate.windowPositionY);
   glfwSetWindowMonitor(g_GLFWstate.window, monitor, 0, 0, width, height, g_GLFWstate.vidMode->refreshRate);
   glfwSetWindowSizeCallback(g_GLFWstate.window, windowResizeCallback);
   g_GLFWstate.flags |= CCE_FULLSCREEN;
   glfwSwapInterval(1);
}

static void toWindow__glfw (void)
{
   glfwSetWindowMonitor(g_GLFWstate.window, NULL, g_GLFWstate.windowPositionX, g_GLFWstate.windowPositionY, g_GLFWstate.windowWidth, g_GLFWstate.windowHeight, g_GLFWstate.vidMode->refreshRate);
   g_GLFWstate.flags &= ~CCE_FULLSCREEN;
   glfwSwapInterval(1);
}

static void showWindow__glfw (void)
{
   glfwShowWindow(g_GLFWstate.window);
}

static void keyCallback (GLFWwindow *window, int key, int scancode, int action, int modifiers)
{
   CCE_UNUSED(window);
   CCE_UNUSED(scancode);
   struct RegisteredKeys *keyInfo = (struct RegisteredKeys*) cceBinarySearchFirstAscending((((uint8_t*) cce_keys) + offsetof(struct RegisteredKeys, key)), keysQuantity, sizeof(int), sizeof(struct RegisteredKeys), key);
   if (keyInfo > cce_keys + keysQuantity)
   {
      return;
   }
   if (keyInfo->key == key)
   {
      if ((modifiers & keyInfo->modifiers) == keyInfo->modifiers)
      {
         
      }
   }
}

static struct cce_u32vec2 getAspectRatio (uint32_t width, uint32_t height)
{
   if (!width || !height)
   {
      return (struct cce_u32vec2) {0u, 0u};
   }
   uint32_t ratio = (width * 100u) / height;
   if (ratio < 800u/5u)
   {
      if (ratio < 400u/3u)
      {
         if (ratio < 500u/4u)
            return (struct cce_u32vec2) {1u, 1u};
         else
            return (struct cce_u32vec2) {5u, 4u};
      }
      else
      {
         if (ratio < 300u/2u)
         {
            return (struct cce_u32vec2) {4u, 3u};
         }
         else
         {
            if (ratio < 1400u/9u)
               return (struct cce_u32vec2) {3u, 2u};
            else
               return (struct cce_u32vec2) {14u, 9u};
         }
      }
   }
   else
   {
      if (ratio < 1600u/9u)
      {
         if (ratio < 500u/3u)
            return (struct cce_u32vec2) {8u, 5u};
         else
            return (struct cce_u32vec2) {5u, 3u};
      }
      else
      {
         if (ratio < 200u/1u)
         {
            return (struct cce_u32vec2) {16u, 9u};
         }
         else
         {
            if (ratio < 700u/3u)
               return (struct cce_u32vec2) {2u, 1u};
            else
               return (struct cce_u32vec2) {7u, 3u};
         }
      }
   }
}

struct cce_u32vec2 getCurrentStep__glfw (void)
{
   switch (g_GLFWstate.monitorAspectRatio.x)
   {
      case 1u:
      {
         return (struct cce_u32vec2) {12u, 12u};
      }
      case 3u:
      {
         return (struct cce_u32vec2) {15u, 10u};
      }
      case 5u:
      {
         return (struct cce_u32vec2) {15u, 9u};
      }
      case 7u:
      {
         return (struct cce_u32vec2) {19u, 8u};
      }
      case 14u:
      {
         return (struct cce_u32vec2) {14u, 9u};
      }
      default:
      {
         return (struct cce_u32vec2) {16u, (uint32_t) (16.0f / g_GLFWstate.monitorAspectRatio.x * g_GLFWstate.monitorAspectRatio.y)};
      }
   }
}

static void setWindowParameters__glfw (cce_enum parameter, uint32_t a, uint32_t b)
{
   
}

static int compare (const void *_a, const void *_b)
{
   const int a = *((const int*) (((uint8_t*) _a) + offsetof(struct RegisteredKeys, key)));
   const int b = *((const int*) (((uint8_t*) _b) + offsetof(struct RegisteredKeys, key)));
   return (a > b) - (a < b);
}

static void terminateEngine__glfw (void);

int cce__initEngine__glfw (const char *label, struct cce_backend_data *functions)
{
   if (glfwInit() != GLFW_TRUE)
   {
      return -1;
   }
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
   glfwWindowHint(GLFW_AUTO_ICONIFY,   GLFW_TRUE);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   //glfwWindowHint(GLFW_VISIBLE,        GLFW_FALSE);
   glfwWindowHint(GLFW_RESIZABLE,      GL_FALSE);

#ifdef __APPLE__
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
   glfwWindowHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
   glfwWindowHint(GLFW_COCOA_MENUBAR,         GLFW_FALSE);
#endif
#if defined(NDEBUG)
   glfwWindowHint(GLFW_CONTEXT_NO_ERROR,      GLFW_TRUE);
#else
   glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,  GLFW_TRUE);
#endif
   
   if (label == NULL || *label == '\0')
   {
      label = CCE_DEFAULT_WINDOW_LABEL;
   }
   
   GLFWmonitor *monitor = glfwGetPrimaryMonitor();
   g_GLFWstate.vidMode = glfwGetVideoMode(monitor);
   g_GLFWstate.window = glfwCreateWindow(CCE_DEFAULT_WINDOW_WIDTH, CCE_DEFAULT_WINDOW_HEIGHT, label, NULL, NULL);
   if (!(g_GLFWstate.window))
   {
      fprintf(stderr, "GLFW::WINDOW::FAILED_TO_CREATE\n");
      glfwTerminate();
      return -1;
   }
   glfwMakeContextCurrent(g_GLFWstate.window);
   glfwSetFramebufferSizeCallback(g_GLFWstate.window, framebufferSizeCallback);
   glfwSetWindowSizeCallback(g_GLFWstate.window, windowResizeCallback);
   //glfwSetKeyCallback(g_GLFWstate.window, keyCallback);
   if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
   {
      fprintf(stderr, "GLAD::INITIALIZATION::FAILED:\nOpenGL could not be loaded by GLAD.\n");
      glfwTerminate();
      return -1;
   }
   if (!GLAD_GL_VERSION_3_1)
   {
      fprintf(stderr, "GLAD::INITIALIZATION::FAILED:\nOpenGL 3.1 (minimum required) could not be loaded by GLAD.\n");
      glfwTerminate();
      return -1;
   }
   cce_keys = malloc(14u * sizeof(struct RegisteredKeys));
   
   /*
   registerKey__glfw(GLFW_KEY_UP,          GLFW_FALSE, 0x0, globalBoolsQuantity - 12);
   registerKey__glfw(GLFW_KEY_DOWN,        GLFW_FALSE, 0x0, globalBoolsQuantity - 11);
   registerKey__glfw(GLFW_KEY_LEFT,        GLFW_FALSE, 0x0, globalBoolsQuantity - 10);
   registerKey__glfw(GLFW_KEY_RIGHT,       GLFW_FALSE, 0x0, globalBoolsQuantity -  9);
   registerKey__glfw(GLFW_KEY_A,           GLFW_FALSE, 0x0, globalBoolsQuantity -  8);
   registerKey__glfw(GLFW_KEY_S,           GLFW_FALSE, 0x0, globalBoolsQuantity -  7);
   registerKey__glfw(GLFW_KEY_D,           GLFW_FALSE, 0x0, globalBoolsQuantity -  6);
   registerKey__glfw(GLFW_KEY_W,           GLFW_FALSE, 0x0, globalBoolsQuantity -  5);
   registerKey__glfw(GLFW_KEY_LEFT_SHIFT,  GLFW_FALSE, 0x0, globalBoolsQuantity -  4);
   registerKey__glfw(GLFW_KEY_RIGHT_SHIFT, GLFW_FALSE, 0x0, globalBoolsQuantity -  4);
   registerKey__glfw(GLFW_KEY_ENTER,       GLFW_FALSE, 0x0, globalBoolsQuantity -  3);
   registerKey__glfw(GLFW_KEY_Q,           GLFW_FALSE, 0x0, globalBoolsQuantity -  2);
   registerKey__glfw(GLFW_KEY_E,           GLFW_FALSE, 0x0, globalBoolsQuantity -  1);
   
   registerKey__glfw(GLFW_KEY_F11, GLFW_FALSE, 0x2, 0x0);
   qsort(cce_keys, keysQuantity, sizeof(struct RegisteredKeys), compare);
   */
   glfwSwapInterval(1);
   
   functions->screenUpdate = swapBuffers__glfw;
   functions->toWindow = toWindow__glfw;
   functions->toFullscreen = toFullscreen__glfw;
   functions->terminateEngine = terminateEngine__glfw;
   functions->engineUpdate = engineUpdate__glfw;
   functions->getTime = getTime__glfw;
   return 0;
}

static void terminateEngine__glfw (void)
{
   free(cce_keys);
   glfwDestroyWindow(g_GLFWstate.window);
   glfwTerminate();
}
