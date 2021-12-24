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

#include "log.h"
#include "audio.h"
#define STB_VORBIS_HEADER_ONLY
#include "external/stb_vorbis.h"

#define toALFormat(channels) (((channels) > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16)

void loadAudio (const ALuint *buffer, const char *const filepath)
{
   ALint channels;
   ALsizei length, freq;
   short *data;
   length = stb_vorbis_decode_filename(filepath, &channels, &freq, &data);
   alBufferData(*buffer, toALFormat(channels), data, (length * channels * 2), freq);
   free(data);
   ALCenum error = alGetError();
   if (error != AL_NO_ERROR)
      openALErrorPrint(error);
}

struct alObjects* initAL ()
{
   struct alObjects *ALObjects = (struct alObjects*) malloc(sizeof(struct alObjects));
   ALObjects->device = alcOpenDevice(NULL);
   if (!(ALObjects->device))
   {
      criticalErrorPrint("OPENAL::FAILED_TO_OPEN_DEVICE::NULL", NULL);
   }
   ALObjects->context = alcCreateContext(ALObjects->device, NULL);
   ALCenum alcError = alcGetError(ALObjects->device);
   if (alcError != ALC_NO_ERROR)
   {
      criticalErrorPrint("OPENAL::FAILED_TO_CREATE_CONTEXT", NULL);
   }
   if (!alcMakeContextCurrent(ALObjects->context))
   {
      criticalErrorPrint("OPENAL::FAILED_TO_CREATE_CONTEXT", NULL);
   }
   alGetError();
   ALObjects->sources = (ALuint*) malloc(SOURCES_QUANTITY * sizeof(ALuint));
   alGenSources((ALuint)SOURCES_QUANTITY, ALObjects->sources);
   ALenum error = alGetError();
   if (error != AL_NO_ERROR)
   {
      openALErrorPrint(error);
   }
   for (unsigned char i = 0u; i < SOURCES_QUANTITY; ++i)
   {
      alSourcef(*((ALObjects->sources) + i), AL_PITCH, 1);
      error = alGetError();
      if (error != AL_NO_ERROR)
      {
         openALErrorPrint(error);
         criticalErrorPrint("^OPENAL::ERROR^", NULL);
      }
      alSourcef(*((ALObjects->sources) + i), AL_GAIN, 1);
      error = alGetError();
      if (error != AL_NO_ERROR)
      {
         openALErrorPrint(error);
         criticalErrorPrint("^OPENAL::ERROR^", NULL);
      }
      alSource3f(*((ALObjects->sources) + i), AL_POSITION, 0, 0, 0);
         error = alGetError();
      if (error != AL_NO_ERROR)
      {
         openALErrorPrint(error);
         criticalErrorPrint("^OPENAL::ERROR^", NULL);
      }
      alSource3f(*((ALObjects->sources) + i), AL_VELOCITY, 0, 0, 0);
      error = alGetError();
      if (error != AL_NO_ERROR)
      {
         openALErrorPrint(error);
         criticalErrorPrint("^OPENAL::ERROR^", NULL);
      }
      alSourcei(*((ALObjects->sources) + i), AL_LOOPING, AL_FALSE);
      error = alGetError();
      if (error != AL_NO_ERROR)
      {
         openALErrorPrint(error);
         criticalErrorPrint("^OPENAL::ERROR^", NULL);
      }
   }
   ALObjects->buffersQuantity = 0u;
   ALObjects->buffers = NULL;
   return ALObjects;
}


void stopAL (struct alObjects *al)
{
   alDeleteSources(SOURCES_QUANTITY, al->sources);
   free((al->sources));
   alDeleteBuffers(al->buffersQuantity, al->buffers);
   free((al->buffers));
   alcDestroyContext(al->context);
   alcMakeContextCurrent(NULL);
   alcCloseDevice(al->device);
   free(al);
}