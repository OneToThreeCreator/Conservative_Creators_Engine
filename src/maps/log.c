/*
    CoffeeChain - open source engine for making games.
    Copyright (C) 2020-2021 Andrey Givoronsky

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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "map2D_internal.h"
//#include "log.h"
#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif // __APPLE__

void cce__openGLErrorPrint (GLenum error, size_t line, const char *file)
{
   switch (error)
   {
      case GL_NO_ERROR: break;
      case GL_INVALID_ENUM:
      {
         fprintf(stderr, "%s: %ld: OPENGL::INVALID_ENUM:\nan unacceptable value is specified for an enumerated argument\n", file, line);
         break;
      }
      case GL_INVALID_VALUE:
      {
         fprintf(stderr, "%s: %ld: OPENGL::INVALID_VALUE:\na numeric argument is out of range\n", file, line);
         break;
      }
      case GL_INVALID_OPERATION:
      {
         fprintf(stderr, "%s: %ld: OPENGL::INVALID_OPERATION:\nthe specified operation is not allowed in the current state\n", file, line);
         break;
      }
      case GL_INVALID_FRAMEBUFFER_OPERATION:
      {
         fprintf(stderr, "%s: %ld: OPENGL::INVALID_OPERATION::FRAMEBUFFER:\nthe framebuffer object is not complete\n", file, line);
         break;
      }
      case GL_OUT_OF_MEMORY:
      {
         fprintf(stderr, "%s: %ld: OPENGL::OUT_OF_MEMORY:\nthere is not enough memory left to execute the command\n", file, line);
         break;
      }
/*    case GL_STACK_UNDERFLOW:
      {
         fprintf(stderr, "OPENGL::STACK::OVERFLOW:\nan attempt has been made to perform an operation that would cause an internal stack to underflow\n");
         break;
      }
      case GL_STACK_OVERFLOW:
      {
         fprintf(stderr, "OPENGL::STACK::OVERFLOW:\nan attempt has been made to perform an operation that would cause an internal stack to overflow\n");
         break;
      }*/
      default:
      {
         fprintf(stderr, "%s: %ld: OPENGL::UNKNOWN:\n%d\n", file, line, error);
      }
   }
}

void cce__openALErrorPrint (ALenum error)
{
   switch (error)
   {
      case AL_NO_ERROR: break;
      case AL_INVALID_NAME:
      {
         fprintf(stderr, "OPENAL::INVALID_NAME:\na bad name (ID) was passed to an OpenAL function\n");
         break;
      }
      case AL_INVALID_ENUM:
      {
         fprintf(stderr, "OPENAL::INVALID_ENUM:\nan invalid enum value was passed to an OpenAL function\n");
         break;
      }
      case AL_INVALID_VALUE:
      {
         fprintf(stderr, "OPENAL::INVALID_VALUE:\nan invalid value was passed to an OpenAL function\n");
         break;
      }
      case AL_INVALID_OPERATION:
      {
         fprintf(stderr, "OPENAL::INVALID_OPERATION:\nthe requested operation is not valid\n");
         break;
      }
      case AL_OUT_OF_MEMORY:
      {
         fprintf(stderr, "OPENAL::OUT_OF_MEMORY:\nthe requested operation resulted in OpenAL running out of memory\n");
         break;
      }
      default:
      {
         fprintf(stderr, "OPENAL::UNKNOWN:\n%d\n", error);
      }
   }
}

void cce__errorPrint (const char *const msgAndFormat, ...)
{
   va_list args;
   va_start(args, msgAndFormat);
   vfprintf(stderr, msgAndFormat, args);
   fputc('\n', stderr);
   va_end(args);
   return;
}

void cce__infoPrint (const char *const msgAndFormat, ...)
{
   va_list args;
   va_start(args, msgAndFormat);
   vfprintf(stdout, msgAndFormat, args);
   fputc('\n', stdout);
   va_end(args);
   return;
}

void cce__criticalErrorPrint (const char *const msgAndFormat, ...)
{
   va_list args;
   va_start(args, msgAndFormat);
   vfprintf(stderr, msgAndFormat, args);
   fprintf(stderr, "\nCritical!\nShutting down engine...\n");
   va_end(args);
   cce__terminateEngine2D();
   exit(-1);
}
