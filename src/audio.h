#ifndef AUDIO_H
#define AUDIO_H
#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#define SOURCES_QUANTITY 2

struct alObjects
{
   ALCdevice *device;
   ALCcontext *context;
   unsigned short buffersQuantity;
   ALuint *buffers;
   ALuint *sources;
};

void loadAudio (const ALuint *buffer, const char *const filepath);
struct alObjects* initAL ();
void stopAL (struct alObjects *al);

#endif // AUDIO_H
