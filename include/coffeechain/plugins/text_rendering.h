#ifndef TEXT_RENDERING_H
#define TEXT_RENDERING_H

#include "../engine_common.h"
#include "../map2D/map2D.h"

#define CCE_ASCII_EXT_ENCODING 0x1
#define CCE_UTF8_ENCODING      0x2

typedef uint8_t enc_type;

CCE_PUBLIC_OPTIONS int cceInitTextRendering (enc_type encoding);
CCE_PUBLIC_OPTIONS int cceLoadBitmapFont (const char *cceFontName);
CCE_PUBLIC_OPTIONS extern uint32_t* (*ccePrintString)(char *string, struct Map2DElementDev *elementTemplate, cce_enum elementType, uint8_t isCurrentPosition);

#endif // TEXT_RENDERING_H
