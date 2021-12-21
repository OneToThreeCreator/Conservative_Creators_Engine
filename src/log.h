#ifndef LOG_H
#define LOG_H

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif // __APPLE__
#include "external/glad.h"
#include <stdlib.h>

void openGLErrorPrint (GLenum error, size_t line, char *file);
void criticalErrorPrint (const char *const msgAndFormat, ...);
void infoPrint (const char *const msgAndFormat, ...);
void errorPrint (const char *const msgAndFormat, ...);
void openALErrorPrint (ALCenum error);

#endif // LOG_H
