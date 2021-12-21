#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "engine_common_internal.h"
#include "external/glad.h"
#include <GLFW/glfw3.h>
//#include "log.h"
#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif // __APPLE__

void openGLErrorPrint (GLenum error, size_t line, char *file)
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

void openALErrorPrint (ALenum error)
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

void errorPrint (const char *const msgAndFormat, ...)
{
   va_list args;
   va_start(args, msgAndFormat);
   vfprintf(stderr, msgAndFormat, args);
   fputc('\n', stdout);
   va_end(args);
   return;
}

void infoPrint (const char *const msgAndFormat, ...)
{
   va_list args;
   va_start(args, msgAndFormat);
   vfprintf(stdout, msgAndFormat, args);
   fputc('\n', stdout);
   va_end(args);
   return;
}

void criticalErrorPrint (const char *const msgAndFormat, ...)
{
   va_list args;
   va_start(args, msgAndFormat);
   vfprintf(stderr, msgAndFormat, args);
   fprintf(stderr, "\nCritical!\nShutting down engine...\n");
   va_end(args);
   terminateEngine();
   exit(-1);
}
