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

#include <ctype.h>
#include <string.h>

#include <ini.h>

#include "../../../include/cce/engine_common.h"
#include "../../../include/cce/plugins/map2D/map2D.h"
#include "../../../include/cce/plugins/actions.h"
#include "../../../include/cce/os_interaction.h"
#include "../../../include/cce/plugins/map2D/map2D_text_rendering.h"
#include "../../../include/cce/utils.h"

#define UNK 1

#define MAX(x, y)(((x) > (y)) ? (x) : (y))
#define MIN(x, y)(((x) < (y)) ? (x) : (y))
#define CMP_VEC2(a, op, b)(((a).x op (b).x) && ((a).y op (b).y))

struct FontInfo
{
   char *imageName;
   uint32_t textureID;
   struct cce_u16vec2 charSize;
   struct cce_i16vec2 baseLineOffset;
   struct cce_u16vec2 charGap;
};

struct LetterInfo
{
   struct cce_u16vec2 position;
   struct cce_u16vec2 size;
   struct cce_u16vec2 drawOffset;
   uint32_t fontID;
};

struct LetterInfoNumerated
{
   struct cce_u16vec2 position;
   struct cce_u16vec2 size;
   struct cce_u16vec2 drawOffset;
   uint32_t fontID;
   uint32_t characterNum;
};

struct INIKeyHandlerStruct
{
   char *filePath;
   char prevSection[128];
   union
   {
      struct cce_u16vec2 blockOffset;
      struct cce_u16vec2 charGap;
   };
   struct cce_u16vec2 drawCharSize;
   struct cce_u16vec2 charSize;
   union
   {
      struct cce_u16vec2 drawOffset;
      struct cce_i16vec2 baseLineOffset;
   };
   uint32_t fontID;
   union
   {
      char letters[128];
      char imagename[128];
   };
};

struct LetterInfo unknown;
static union
{
   struct LetterInfo *unnumerated;
   struct LetterInfoNumerated *numerated;
} letters;
static size_t lettersQuantity;
static size_t lettersQuantityAllocated;
#ifndef INIH_LOCAL
#include <setjmp.h>

jmp_buf loadingError;
#endif

CCE_ARRAY(fonts, static struct FontInfo, static uint32_t);
static void (*cce__processStruct)(struct INIKeyHandlerStruct*);
CCE_API uint32_t* (*ccePrintString)(char *string, struct Map2DElementDev *elementTemplate, cce_enum elementType, uint8_t isCurrentPosition);

static int cce__processProperties (struct INIKeyHandlerStruct *st)
{
   uint16_t textureID = 0;
   for (struct FontInfo *iterator = fonts, *end = fonts + fontsQuantity; iterator < end; ++iterator)
   {
      if (strcmp(iterator->imageName, st->imagename) == 0)
      {
         textureID = iterator->textureID;
         if (CMP_VEC2(iterator->charSize, ==, st->charSize) && CMP_VEC2(iterator->baseLineOffset, ==, st->baseLineOffset))
         {
            st->fontID = iterator - fonts;
            return 0;
         }
      }
   }
   st->fontID = fontsQuantity;
   if (textureID == 0)
   {
      size_t fileNamePosition = 0, pathLength = 0;
      char *path = st->filePath;
      for (char *iterator = path; *iterator != '\0'; ++iterator, ++pathLength)
      {
         if (*iterator == '/' || *iterator == '\\')
         {
            fileNamePosition = iterator - path + 1;
         }
      }
      size_t imageNameLength = strlen(st->imagename);
      if (pathLength - fileNamePosition + 8 < imageNameLength)
      {
         st->filePath = realloc(path, fileNamePosition + imageNameLength + 1);
         path = st->filePath;
      }
      memcpy(path + fileNamePosition, st->imagename, imageNameLength + 1);
      textureID = cceLoadTexture(path);
      if (textureID == 0)
      {
         fprintf(stderr, "ENGINE::TEXT_RENDERING::IMAGE_LOADING_ERROR:\n%s - file cannot be loaded\n", path);
         return -1;
      }
   }
   ++fontsQuantity;
   CCE_FIT_ARRAY_TO_SIZE(fonts);
   size_t nameLength = strlen(st->imagename);
   fonts[st->fontID].imageName = malloc(nameLength + 1);
   memcpy(fonts[st->fontID].imageName, st->imagename, nameLength);
   fonts[st->fontID].imageName[nameLength] = '\0';
   fonts[st->fontID].charSize              = st->charSize;
   fonts[st->fontID].baseLineOffset        = st->baseLineOffset;
   fonts[st->fontID].textureID             = textureID;
   fonts[st->fontID].charGap               = st->charGap;
   return 0;
}

#define HANDLER_STRUCT_TO_LETTER(st, l, o) \
(l)->position.x = (st)->blockOffset.x + (o); \
(l)->position.y = (st)->blockOffset.y; \
(l)->size       = (((st)->drawCharSize.x == 0 && (st)->drawCharSize.y == 0) ? (fonts[(st)->fontID].charSize) : ((st)->drawCharSize)); \
(l)->drawOffset = (st)->drawOffset; \
(l)->fontID     = (st)->fontID; \
(o)            += (st)->charSize.x

static void cce__processStructASCII (struct INIKeyHandlerStruct *structure)
{
   struct INIKeyHandlerStruct *st = (struct INIKeyHandlerStruct*) structure;
   struct LetterInfo *currentLetter;
   uint32_t offset = 0;
   if (st->letters[0] == '\0') // letters weren't specified
   {
      if (st->prevSection[1] == '-')
      {
         if (memcmp(st->prevSection + 2, "UNK", 3) == 0)
         {
            HANDLER_STRUCT_TO_LETTER(st, letters.unnumerated + st->prevSection[0] - 33, offset);
            HANDLER_STRUCT_TO_LETTER(st, &unknown, offset);
            return;
         }
         char i, j;
         if (st->prevSection[0] <= st->prevSection[2])
         {
            i = st->prevSection[0];
            j = st->prevSection[2];
         }
         else
         {
            i = st->prevSection[2];
            j = st->prevSection[0];
         }
         while (i <= j)
         {
            currentLetter = letters.unnumerated + i;
            HANDLER_STRUCT_TO_LETTER(st, currentLetter, offset);
            ++i;
         }
      }
      else if (memcmp(st->prevSection, "UNK", 3) == 0)
      {
         HANDLER_STRUCT_TO_LETTER(st, &unknown, offset);
         if (st->prevSection[3] == '-')
         {
            HANDLER_STRUCT_TO_LETTER(st, letters.unnumerated + st->prevSection[4] - 33, offset);
         }
      }
      else
      {
         HANDLER_STRUCT_TO_LETTER(st, letters.unnumerated + st->prevSection[0] - 33, offset);
      }
      return;
   }
   for (char *iterator = st->letters, *end = st->letters + 127; iterator < end && *iterator != '\0'; ++iterator)
   {
      switch (*iterator)
      {
         case UNK:
            currentLetter = &unknown;
            break;
         default:
            currentLetter = letters.unnumerated + *iterator - 33;
            break;
      }
      HANDLER_STRUCT_TO_LETTER(st, currentLetter, offset);
   }
}

void processSymbol (uint32_t s, struct INIKeyHandlerStruct *st, uint32_t *o)
{
   struct LetterInfoNumerated *currentLetter;
   ++lettersQuantity;
   if (lettersQuantity > lettersQuantityAllocated)
   {
      letters.numerated = realloc(letters.numerated, (lettersQuantityAllocated + CCE_ALLOCATION_STEP) * sizeof(struct LetterInfoNumerated));
      memset(letters.numerated + lettersQuantityAllocated, 0, sizeof(struct LetterInfoNumerated) * CCE_ALLOCATION_STEP);
      lettersQuantityAllocated += CCE_ALLOCATION_STEP;
   }
   currentLetter = letters.numerated + cceBinarySearch(&(letters.numerated->characterNum), lettersQuantity - 1, sizeof(uint32_t), sizeof(struct LetterInfoNumerated), s);
   if (currentLetter->characterNum == s)
   {
      --lettersQuantity;
   }
   else if (currentLetter < letters.numerated + lettersQuantity)
   {
      memmove(currentLetter + 1, currentLetter, (lettersQuantity - (currentLetter - letters.numerated) - 1) * sizeof(struct LetterInfoNumerated));
   }
   HANDLER_STRUCT_TO_LETTER(st, currentLetter, *o);
   currentLetter->characterNum = s;
}

static void cce__processStructUTF8 (struct INIKeyHandlerStruct *structure)
{
   struct INIKeyHandlerStruct *st = (struct INIKeyHandlerStruct*) structure;
   uint32_t offset = 0;
   if (st->letters[0] == '\0') // letters weren't specified
   {
      struct UnicodeCharWithSize chars[3];
      chars[0] = cceGetCharWithSizeUTF8((unsigned char*)st->prevSection);
      if (chars[0].ch == 'U' && (strcmp(st->prevSection, "UNK") == 0))
      {
         chars[0] = (struct UnicodeCharWithSize) {UNK, 3};
      }
      chars[1] = cceGetCharWithSizeUTF8((unsigned char*)st->prevSection + chars[0].size);
      if (chars[1].ch != '-')
      {
         if (chars[0].ch == UNK)
         {
            HANDLER_STRUCT_TO_LETTER(st, &unknown, offset);
         }
         else
         {
            processSymbol(chars[0].ch, st, &offset);
         }
         return;
      }
      chars[2] = cceGetCharWithSizeUTF8((unsigned char*)st->prevSection + chars[0].size + chars[1].size);
      if (chars[2].ch == 'U' && (strcmp(st->prevSection + chars[0].size + chars[1].size, "UNK") == 0))
      {
         processSymbol(chars[0].ch, st, &offset);
         HANDLER_STRUCT_TO_LETTER(st, &unknown, offset);
         return;
      }
      if (chars[0].ch == UNK)
      {
         HANDLER_STRUCT_TO_LETTER(st, &unknown, offset);
         processSymbol(chars[0].ch, st, &offset);
         return;
      }
      uint32_t i, j;
      if (chars[0].ch <= chars[2].ch)
      {
         i = chars[0].ch;
         j = chars[2].ch;
      }
      else
      {
         i = chars[2].ch;
         j = chars[0].ch;
      }
      while (i <= j)
      {
         processSymbol(i, st, &offset);
         ++i;
      }
      return;
   }
   struct UnicodeCharWithSize symbol;
   for (unsigned char *iterator = (unsigned char*) st->letters, *end = (unsigned char*) st->letters + 127; iterator < end && *iterator != '\0'; iterator += symbol.size)
   {
      if (*iterator == UNK)
      {
         HANDLER_STRUCT_TO_LETTER(st, &unknown, offset);
         continue;
      }
      symbol = cceGetCharWithSizeUTF8(iterator);
      processSymbol(symbol.ch, st, &offset);
   }
}

static void cce__valueToLetters (struct INIKeyHandlerStruct *st, const char *string)
{
   char *jiterator = st->letters;
   while (*string != '\0' && *string != '\n')
   {
      if (!iscntrl(*string) && *string != ' ')
      {
         *jiterator = *string;
         if (!iscntrl(*(string + 1)) && *(string + 1) != ' ')
         {
            if (strcmp(string, "UNK") == 0) // UNK - unknown
            {
               *jiterator = UNK;
               ++jiterator;
               string += 3;
               continue;
            }
            ++jiterator;
            *jiterator = *(string + 1);
         }
         ++jiterator;
         ++string;
      }
      ++string;
   }
   *jiterator = '\0';
}

static int cce__iniKeyHandler (void *structure, const char *section, const char *name,
                               const char *value)
{
   struct INIKeyHandlerStruct *st = (struct INIKeyHandlerStruct*) structure;
   if (section == NULL || strcmp(section, "Properties") == 0)
   {
      if (memcmp(st->prevSection, "Properties", 11) != 0)
      {
         memcpy(st->prevSection, "Properties", 11);
      }
      if (strcmp(name, "imgname") == 0)
      {
         size_t nameLength = strlen(value);
         if (value[0] == '"' || value[0] == '\'')
         {
            ++value;
            --nameLength;
            if (value[nameLength - 1] == '"' || value[nameLength - 1] == '\'')
               --nameLength;
         }
         memcpy(st->imagename, value, MIN(nameLength + 1, 128));
      }
      else if (strcmp(name, "charsize") == 0)
      {
         st->charSize = cceStringToU16Vec2(value);
      }
      else if (strcmp(name, "baselineoffset") == 0)
      {
         st->baseLineOffset = cceStringToI16Vec2(value);
      }
      else if (strcmp(name, "chargap") == 0)
      {
         st->charGap = cceStringToU16Vec2(value);
      }
      else
      {
         fprintf(stderr, "ENGINE::TEXT_RENDERING::INI_PARSER_ERROR:\nUnknown field with name %s appeared in section %s\n", name, section);
      }
      return 1;
   }
   if (strcmp(st->prevSection, section) != 0)
   {
      if (memcmp(st->prevSection, "Properties", 11) == 0)
      {
         if (cce__processProperties(st) != 0)
            #if defined(INIH_LOCAL)
            return 0;
            #else
            longjmp(loadingError, 1);
            #endif
      }
      else
      {
         if (fontsQuantity <= st->fontID)
         {
            fprintf(stderr, "ENGINE::TEXT_RENDERING::INI_PARSER_ERROR:\nProperties section must appear first\n");
            #if defined(INIH_LOCAL)
            return 0;
            #else
            longjmp(loadingError, 1);
            #endif
         }
         cce__processStruct(st);
      }
      strncpy(st->prevSection, section, 127);
      st->prevSection[127] = '\0';
      st->letters[0] = '\0';
      st->drawCharSize = (struct cce_u16vec2) {0, 0};
      st->charSize = fonts[st->fontID].charSize;
      st->drawOffset = (struct cce_u16vec2) {0, 0};
   }
   if (strcmp(name, "blockoffset") == 0)
   {
      st->blockOffset = cceStringToU16Vec2(value);
   }
   else if (strcmp(name, "chars") == 0)
   {
      cce__valueToLetters(st, value);
   }
   else if (strcmp(name, "drawcharsize") == 0)
   {
      st->drawCharSize = cceStringToU16Vec2(value);
   }
   else if (strcmp(name, "charsize") == 0)
   {
      st->charSize = cceStringToU16Vec2(value);
      if (st->drawCharSize.x == 0 && st->drawCharSize.y == 0)
         st->drawCharSize = st->charSize;
   }
   else if (strcmp(name, "drawoffset") == 0)
   {
      st->drawOffset = cceStringToU16Vec2(value);
   }
   else
   {
      fprintf(stderr, "ENGINE::TEXT_RENDERING::INI_PARSER_ERROR:\nUnknown field with name %s appeared in section %s\n", name, section);
   }
   return 1;
}

CCE_API int cceLoadBitmapFont (const char *cceFontName)
{
   size_t nameLength = strlen(cceFontName);
   const char *resourcePath = cceGetResourcePath();
   size_t resourcePathSize = strlen(resourcePath);
   size_t fullResourcePathSize = resourcePathSize + (resourcePath[resourcePathSize - 1] != '/') + 6; /*+ "fonts/"*/;
   FILE *file = NULL;
   char *path;
   
   #ifdef SYSTEM_RESOURCE_PATH
   
   size_t systemPathSize = strlen(SYSTEM_RESOURCE_PATH "fonts/");
   path = malloc(MAX(fullResourcePathSize, systemPathSize) + nameLength + 4 + 8); /* + ".ini" +8 in case imagename is longer than ini file name */
   memcpy(path, SYSTEM_RESOURCE_PATH "fonts/", systemPathSize);
   memcpy(path + systemPathSize, cceFontName, nameLength); 
   memcpy(path + systemPathSize + nameLength, ".ini", 4 + 1/* '\0' */);
   file = fopen(path, "r");
   if (file == NULL)
      
   #else
   
   path = malloc(fullResourcePathSize + nameLength + 4 + 8);
   
   #endif // SYSTEM_RESOURCE_PATH
   {
      memcpy(path, resourcePath, resourcePathSize);
      if (fullResourcePathSize - resourcePathSize > 6)
      {
         path[resourcePathSize] = '/';
         ++resourcePathSize;
      }
      memcpy(path + resourcePathSize, "fonts/", 6);
      memcpy(path + fullResourcePathSize, cceFontName, nameLength);
      memcpy(path + fullResourcePathSize + nameLength, ".ini", 4 + 1/* '\0' */);
      file = fopen(path, "r");
      if (file == NULL)
      {
         fprintf(stderr, "ENGINE::TEXT_RENDERING::FONT_LOADING_FAILURE:\nFont %s was not found\npath: %s\n", cceFontName, path);
         free(path);
         return -1;
      }
   }
   struct INIKeyHandlerStruct st = {0};
   st.filePath = path;
   st.fontID = UINT16_MAX;
   #if defined(INIH_LOCAL)
   if (ini_parse_file(file, cce__iniKeyHandler, &st) != 0)
   #else
   if (setjmp(loadingError) != 0 || ini_parse_file(file, cce__iniKeyHandler, &st) != 0)
   #endif
   {
      free(st.filePath);
      fprintf(stderr, "Error occured while parsing info of font %s\n", cceFontName);
      fclose(file);
      return -1;
   }
   cce__processStruct(&st);
   free(st.filePath);
   fclose(file);
   return 0;
}

static uint32_t* ccePrintStringASCII (char *string, struct Map2DElementDev *elementTemplate, cce_enum elementType, uint8_t flags)
{
   size_t length = strlen(string);
   struct Map2DElementDev *elements = malloc(length * sizeof(struct Map2DElementDev));
   struct Map2DElementDev *currentElement = elements;
   struct cce_u32vec2 index = {0, 0};
   for (char *end = string + length; string < end; ++string)
   {
      if (iscntrl(*string))
      {
         switch (*string)
         {
            case '\b':
               index.x -= 1;
               break;
            case '\t':
               index.x &= ~0x7;
               index.x += 8;
               break;
            case '\n':
               index.x = 0;
               index.y += 1;
               break;
            case '\v':
               index.y = ((index.y / 6) + 1) * 6;
               break;
            case '\r':
               index.x = 0;
               break;
         }
         continue;
      }
      if (*string == ' ')
      {
         ++index.x;
         continue;
      }
      memcpy(currentElement, elementTemplate, sizeof(struct Map2DElementDev));
      struct LetterInfo *letter = letters.unnumerated + *string - 33;
      if (letter->size.x == 0 && letter->size.y == 0)
      {
         letter = &unknown;
      }
      struct FontInfo *font = fonts + letter->fontID;
      currentElement->position.x += index.x * (font->charSize.x + font->charGap.x) + letter->drawOffset.x;
      currentElement->position.y -= index.y * (font->charSize.y + font->charGap.y) + letter->size.y + letter->drawOffset.y;
      currentElement->size = letter->size;
      currentElement->textureInfo.position = letter->position;
      currentElement->textureInfo.size = letter->size;
      currentElement->textureInfo.ID = font->textureID;
      ++currentElement;
      ++index.x;
   }
   size_t elementsProcessed = currentElement - elements;
   elements = realloc(elements, elementsProcessed * sizeof(struct Map2DElementDev));
   uint32_t* result = cceCreateMap2DElementsDynamicMap2D(elements, elementsProcessed, elementType, flags | CCE_TEXTUREID_IS_NOT_IMAGEID);
   free(elements);
   return result;
}

static uint32_t* ccePrintStringUTF8 (char *string, struct Map2DElementDev *elementTemplate, cce_enum elementType, uint8_t flags)
{
   size_t length = strlen(string);
   struct Map2DElementDev *elements = malloc(length * sizeof(struct Map2DElementDev));
   struct Map2DElementDev *currentElement = elements;
   struct cce_u32vec2 index = {0, 0};
   struct UnicodeCharWithSize c;
   struct LetterInfo *letter;
   for (char *end = string + length; string < end; string += c.size)
   {
      if (iscntrl(*string))
      {
         c.size = 1;
         switch (*string)
         {
            case '\b':
               index.x -= 1;
               break;
            case '\t':
               index.x &= ~0x7;
               index.x += 8;
               break;
            case '\n':
               index.x = 0;
               index.y += 1;
               break;
            case '\v':
               index.y = ((index.y / 6) + 1) * 6;
               break;
            case '\r':
               index.x = 0;
               break;
         }
         continue;
      }
      if (*string == ' ')
      {
         c.size = 1;
         ++index.x;
         continue;
      }
      c = cceGetCharWithSizeUTF8((unsigned char*) string);
      memcpy(currentElement, elementTemplate, sizeof(struct Map2DElementDev));
      size_t position = cceBinarySearch(&(letters.numerated->characterNum), lettersQuantity, sizeof(uint32_t), sizeof(struct LetterInfoNumerated), c.ch);
      if (position < lettersQuantity)
      {
         struct LetterInfoNumerated *let = letters.numerated + position;
         if (let->characterNum == c.ch)
         {
            letter = (struct LetterInfo*) let;
         }
         else
         {
            letter = &unknown;
         }
      }
      else
      {
         letter = &unknown;
      }
      if (letter->size.x == 0 && letter->size.y == 0)
      {
         ++index.x;
         continue;
      }
      struct FontInfo *font = fonts + letter->fontID;
      currentElement->position.x += index.x * (font->charSize.x + font->charGap.x) + letter->drawOffset.x;
      currentElement->position.y -= index.y * (font->charSize.y + font->charGap.y) + letter->size.y + letter->drawOffset.y;
      currentElement->size = letter->size;
      currentElement->textureInfo.position = letter->position;
      currentElement->textureInfo.size = letter->size;
      currentElement->textureInfo.ID = font->textureID;
      ++currentElement;
      ++index.x;
   }
   size_t elementsProcessed = currentElement - elements;
   elements = realloc(elements, elementsProcessed * sizeof(struct Map2DElementDev));
   uint32_t* result = cceCreateMap2DElementsDynamicMap2D(elements, elementsProcessed, elementType, flags | CCE_TEXTUREID_IS_NOT_IMAGEID);
   free(elements);
   return result;
}

CCE_API int cceInitTextRendering (enc_type encoding)
{
   switch (encoding)
   {
      case CCE_ASCII_EXT_ENCODING:
      {
         letters.unnumerated = calloc(222, sizeof(struct LetterInfo));
         lettersQuantity = 222;
         lettersQuantityAllocated = 222;
         cce__processStruct = cce__processStructASCII;
         ccePrintString = ccePrintStringASCII;
         break;
      }
      case CCE_UTF8_ENCODING:
      {
         letters.numerated = calloc(CCE_ALLOCATION_STEP, sizeof(struct LetterInfoNumerated));
         lettersQuantity = 0;
         lettersQuantityAllocated = CCE_ALLOCATION_STEP;
         cce__processStruct = cce__processStructUTF8;
         ccePrintString = ccePrintStringUTF8;
         break;
      }
      default:
      {
         return -1;
      }
   }
   CCE_ALLOC_ARRAY(fonts);
   fontsQuantity = 0;
   unknown.position = (struct cce_u16vec2) {0, 0};
   unknown.size = (struct cce_u16vec2) {0, 0};
   return 0;
}
