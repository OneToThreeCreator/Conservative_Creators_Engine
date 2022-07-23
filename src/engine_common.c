/*
    CoffeeChain - open source engine for making games.
    Copyright (C) 2020-2022 Andrey Givoronsky

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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "external/stb_image.h"

#include "../include/coffeechain/engine_common.h"
#include "../include/coffeechain/utils.h"
#include "../include/coffeechain/path_getters.h"
#include "../include/coffeechain/endianess.h"

#include "engine_common_internal.h"
#include "shader.h"
#include "platform/engine_common_glfw.h"

struct GlobalVariables
{
   uint16_t          plotNumber;             /* 1 short, 65536 values (chain) */
   uint_fast16_t    *globalBools;            /* 4 or 8 bytes -> globalBoolsQuantity * 8 bytes (8 B - 8 KiB), 8 - 65536 values */
   uint_fast16_t    *temporaryBools;         /* 4 or 8 bytes -> (1024 - globalBoolsQuantity) * 8 bytes (8 KiB - 8 B), 65536 - 8 values */
};

uint16_t      g_globalBoolsQuantity;    /* Quantity of 8-byte variables */
static struct GlobalVariables cce_Gvars;

//static struct alObjects *AL;

CCE_ARRAY(g_temporaryBools, static struct UsedTemporaryBools, static uint16_t);
static uint8_t g_flags;

void (*cce__engineUpdate__api) (void);
void (*cce__terminateEngine__api) (void);
struct cce_u32vec2 (*cce__getCurrentStep) (void);
void (*cce__toFullscreen) (void);
void (*cce__toWindow) (void);
void (*cce__showWindow) (void);
void (*cce__swapBuffers) (void);
CCE_PUBLIC_OPTIONS void (*cceSetWindowParameters) (cce_enum parameter, uint32_t a, uint32_t b);

void cce__callActions (void (**doAction)(void*), uint8_t actionsQuantity, uint32_t *actionIDs, uint32_t *actionArgOffsets, cce_void *actionArgs)
{
   for (cce_ubyte j = 0u; j < actionsQuantity; ++j)
   {
      (**(doAction + (*(actionIDs + j))))((void*) (actionArgs + (*(actionArgOffsets + j))));
   }
}

// Workaround for increasing timers precision. Makes chains of timers independent of frametime
static double maxTimerCheckDelay = 0.0;

CCE_PUBLIC_OPTIONS uint8_t cceIsTimerExpired (struct Timer *timer)
{
   if (timer->initTime < 0.0 || timer->delay == 0.0)
      return 1;
   double timerCheckDelay = *cceCurrentTime - (timer->initTime + timer->delay);
   if (timerCheckDelay >= 0.0)
   {
      if (timerCheckDelay < *cceDeltaTime && (maxTimerCheckDelay == 0 || maxTimerCheckDelay > timerCheckDelay))
      {
         maxTimerCheckDelay = timerCheckDelay;
      }
      return 1;
   }
   return 0;
}

CCE_PUBLIC_OPTIONS void cceStartTimer (struct Timer *timer)
{
   timer->initTime = *cceCurrentTime - maxTimerCheckDelay;
}

CCE_PUBLIC_OPTIONS void resetTimerDelayCompensation (void)
{
   maxTimerCheckDelay = 0.0;
}

CCE_PUBLIC_OPTIONS uint8_t cceGetBool (uint16_t boolID)
{
   uint_fast16_t *boolean;
   if (boolID < g_globalBoolsQuantity)
   {
      boolean = cce_Gvars.globalBools;
   }
   else
   {
      boolean = cce_Gvars.temporaryBools;
      boolID -= g_globalBoolsQuantity;
   }
   boolean += ((boolID) >> (3u + SHIFT_OF_FAST_SIZE));
   return ((*boolean) & (((uint_fast16_t) 0x0001u) << ((boolID) & BITWIZE_AND_OF_FAST_SIZE))) > 0u;
}

CCE_PUBLIC_OPTIONS void cceSetBool (uint16_t boolID, cce_enum action)
{
   uint_fast16_t *boolean;
   if (boolID < g_globalBoolsQuantity)
   {
      boolean = cce_Gvars.globalBools;
   }
   else
   {
      boolean = cce_Gvars.temporaryBools;
      boolID -= g_globalBoolsQuantity;
   }
   boolean += ((boolID) >> (3u + SHIFT_OF_FAST_SIZE));
   register uint_fast16_t mask = (((uint_fast16_t) 0x0001u) << ((boolID) & BITWIZE_AND_OF_FAST_SIZE));
   switch (action)
   {
      case CCE_ENABLE_BOOL:
      {
         (*boolean) |=  mask;
         break;
      }
      case CCE_DISABLE_BOOL:
      {
         (*boolean) &= ~mask;
         break;
      }
      case CCE_SWITCH_BOOL:
      {
         (*boolean) ^=  mask;
         break;
      }
   }
}

CCE_PUBLIC_OPTIONS uint8_t cceCheckPlotNumber (uint16_t value)
{
   return cce_Gvars.plotNumber > value;
}

CCE_PUBLIC_OPTIONS void cceIncreasePlotNumber (uint16_t value)
{
   cce_Gvars.plotNumber += value;
}

CCE_PUBLIC_OPTIONS void cceSetPlotNumber      (uint16_t value)
{
   cce_Gvars.plotNumber = value;
}

static void updateTemporaryBoolsArray (void)
{
   for (struct UsedTemporaryBools *iterator = g_temporaryBools, *end = g_temporaryBools + g_temporaryBoolsQuantity; iterator < end; ++iterator)
   {
      if (iterator->flags & 0x2)
      {
         iterator->flags &= 0x1;
         memset(iterator->temporaryBools, 0, (UINT16_MAX - g_globalBoolsQuantity + 1u) >> 3u);
      }
   }
}

uint16_t cce__getFreeTemporaryBools (void)
{
   g_flags |= CCE_PROCESS_TEMPORARY_BOOLS_ARRAY;
   for (struct UsedTemporaryBools *iterator = g_temporaryBools, *end = g_temporaryBools + g_temporaryBoolsQuantity; iterator < end; ++iterator)
   {
      if (iterator->flags & 0x1)
         continue;
      iterator->flags |= 0x1;
      return (uint16_t) (iterator - g_temporaryBools);
   }
   if (g_temporaryBoolsQuantity >= g_temporaryBoolsQuantityAllocated)
   {
      g_temporaryBoolsQuantityAllocated += CCE_ALLOCATION_STEP;
      g_temporaryBools = realloc(g_temporaryBools, g_temporaryBoolsQuantityAllocated * sizeof(struct UsedTemporaryBools));
      memset(g_temporaryBools + g_temporaryBoolsQuantityAllocated - CCE_ALLOCATION_STEP, 0u, CCE_ALLOCATION_STEP);
   }
   struct UsedTemporaryBools *temporaryBools = g_temporaryBools + g_temporaryBoolsQuantity;
   temporaryBools->flags |= 0x1;
   if (!(temporaryBools->temporaryBools))
   {
      temporaryBools->temporaryBools = (uint_fast16_t *) calloc(((UINT16_MAX + (uint32_t) 1u) >> (SHIFT_OF_FAST_SIZE + 3u)) - (g_globalBoolsQuantity >> (SHIFT_OF_FAST_SIZE + 3u)), sizeof(uint_fast16_t));
   }
   return g_temporaryBoolsQuantity++;
}

void cce__releaseTemporaryBools (uint16_t ID)
{
   (g_temporaryBools + ID)->flags = 0x2;
   return;
}

void cce__releaseUnusedTemporaryBools (uint16_t ID)
{
   (g_temporaryBools + ID)->flags = 0x0;
   return;
}

void cce__setCurrentTemporaryBools (uint16_t temporaryBoolsID)
{
   cce_Gvars.temporaryBools = (g_temporaryBools + temporaryBoolsID)->temporaryBools;
}

void cce__processLogic (uint32_t logicQuantity, struct ElementLogic *logic, struct Timer *timers, void (**doAction)(void*),
                        cce_ubyte (*fourth_if_func)(uint16_t, va_list), ...)
{
   va_list argp, argcp;
   va_start(argp, fourth_if_func);
   uint_fast32_t boolSum;
   uint16_t boolNumber;
   struct ElementLogic *endLogic = (logic + logicQuantity);
   uint_fast16_t currentOperations;
   cce_byte isLogic;
   while (logic < endLogic)
   {
      resetTimerDelayCompensation();
      boolSum = 0u;
      currentOperations = (*logic->operations);
      for (cce_ubyte j = 0;; ++j)
      {
         boolNumber = (*((logic->logicElements) + j));
         switch (((logic->elementType) >> (j * 2u)) & 0x3)
         {
            case 0x0:
            {
               boolSum += (((uint_fast16_t) cceGetBool(boolNumber)) << (logic->logicElementsQuantity - 1u - j));
               break;
            }
            case 0x1:
            {
               boolSum += (((uint_fast16_t) (cce_Gvars.plotNumber > boolNumber)) << (logic->logicElementsQuantity - 1u - j));
               break;
            }
            case 0x2:
            {
               boolSum += (((uint_fast16_t) cceIsTimerExpired(timers + boolNumber)) << (logic->logicElementsQuantity - 1u - j));
               break;
            }
            default:
            {
               va_copy(argcp, argp);
               boolSum += (((uint_fast16_t) fourth_if_func(boolNumber, argcp)) << (logic->logicElementsQuantity - 1u - j)); // x << j == x * pow(2, j)
               va_end(argcp);
            }
         }
         // Some portable (probably) magic for checking results and exit earlier if only false or only true is possible (and no other options).
         switch (logic->logicElementsQuantity - 1u - j)
         {
            #if UINT_FAST16_MAX < UINT64_MAX
            case 6u:
            #if UINT_FAST16_MAX < UINT32_MAX
            case 5u:
            #endif // UINT_FAST16_MAX < UINT32_MAX
            {
               cce_byte state = 0;
               for (uint_fast16_t *iterator = (logic->operations + (boolSum >> SHIFT_OF_FAST_SIZE)),
               *end = logic->operations + (boolSum >> SHIFT_OF_FAST_SIZE) + (1 << (logic->logicElementsQuantity - j - 1u - 3u))/sizeof(uint_fast16_t); iterator < end; ++iterator)
               {
                  if (!(*iterator))
                  {
                     --state;
                  }
                  else if ((*iterator) == UINT_FAST16_MAX)
                  {
                     ++state;
                  }
               }
               switch (state)
               {
                  case -sizeof(uint_fast16_t):
                  {
                     isLogic = 0u;
                     break;
                  }
                  case sizeof(uint_fast16_t):
                  {
                     isLogic = 1u;
                     break;
                  }
                  default:
                  {
                     continue;
                  }
               }
               break;
            }
            #if UINT_FAST16_MAX == UINT32_MAX
            case 5u:
            #else
            case 4u:
            #endif // UINT_FAST16_MAX == UINT32_MAX
            #else
            case 6u:
            #endif // UINT_FAST16_MAX < UINT64_MAX
            {
               currentOperations = (*(logic->operations + (boolSum >> SHIFT_OF_FAST_SIZE)));
               switch (currentOperations)
               {
                  case 0u:
                  {
                     isLogic = 0u;
                     break;
                  }
                  case UINT_FAST16_MAX:
                  {
                     isLogic = 1u;
                     break;
                  }
                  default:
                  {
                     continue;
                  }
               }
               break;
            }
            #if UINT_FAST16_MAX > UINT32_MAX
            case 5u:
            #endif
            #if UINT_FAST16_MAX > UINT16_MAX
            case 4u:
            #endif
            case 3u:
            case 2u:
            case 1u:
            case 0u:
            {
               uint_fast16_t mask = ((((uint_fast16_t) 1) << (1u << (logic->logicElementsQuantity - 1u - j))) - 1u) << (boolSum & BITWIZE_AND_OF_FAST_SIZE);
               currentOperations &= mask;
               if (!currentOperations)
               {
                  isLogic = 0u;
                  break;
               }
               else if (currentOperations == mask)
               {
                  isLogic = 1u;
                  break;
               }
               continue;
            }
            default:
            {
               continue;
            }
         }
         break;
      }
      if (isLogic)
      {
         cce__callActions(doAction, logic->actionsQuantity, logic->actionIDs, logic->actionsArgOffsets, logic->actionsArg);
      }
      ++logic;
   }
   va_end(argp);
}

CCE_PUBLIC_OPTIONS cce_ubyte cceCheckCollision (int32_t element1_x, int32_t element1_y, int32_t element1_width, int32_t element1_height,
                                                int32_t element2_x, int32_t element2_y, int32_t element2_width, int32_t element2_height)
{
   if (element1_x < element2_x)
   {
      if (element1_x + element1_width <= element2_x)
      {
         return 0u;
      }
   }
   else if (element1_x >= element2_x + element2_width)
   {
      return 0u;
   }
   
   if (element1_y < element2_y)
   {
      if (element1_y + element1_height <= element2_y)
      {
         return 0u;
      }
   }
   else if (element1_y >= element2_y + element2_height)
   { 
      return 0u;
   }
   return 1u;
}

struct operationsStack
{
   uint_fast16_t *operations;
   struct operationsStack *prev;
   uint_fast16_t operationPriority;
   uint8_t flags; /* 0x1 - logicElement is inverted */
   uint8_t operation;
   uint8_t logicElementID;
};

/* Logic operations */
#define NEG    1u
#define AND    2u
#define NAND   3u
#define OR     4u
#define NOR    5u
#define XOR    6u
#define XNOR   7u
#define GRTR   8u
#define GRTREQ 9u
#define LESS   10u
#define LESSEQ 11u
#define IMPL   LESSEQ

static struct operationsStack* pushToOperationsStack (struct operationsStack *restrict stack, uint_fast32_t priority, uint8_t operation, uint_fast16_t *operations)
{
   struct operationsStack *currentStack = (struct operationsStack*) malloc(sizeof(struct operationsStack));
   currentStack->operationPriority = priority;
   currentStack->operations = operations;
   currentStack->flags = operations != NULL;
   currentStack->operation = operation;
   currentStack->prev = stack;
   return currentStack;
}

/*
struct operationsLoaded
{
   uint_fast16_t *operations;
   uint8_t        isBusy;
};

static struct operationsLoaded *operationsLoaded;
static uint16_t                 operationsLoadedQuantity;

static uint16_t getFreeOperations (uint8_t operationsQuantity)
{
   toBeImplemented (or not, idk)
}
*/

static uint_fast16_t* generateOperationsFromLogicElement (uint8_t ID, uint8_t isInverted, uint8_t logicElementsQuantity)
{
   uint8_t isLogicQuantityHigherThanVariableSize = logicElementsQuantity > (3u + SHIFT_OF_FAST_SIZE);
   size_t operationsQuantity = (0x01 << (logicElementsQuantity - (3u + SHIFT_OF_FAST_SIZE))) * isLogicQuantityHigherThanVariableSize + (!isLogicQuantityHigherThanVariableSize);
   uint_fast16_t *operations = calloc(operationsQuantity, sizeof(uint_fast16_t));
   uint_fast16_t step;
   if ((logicElementsQuantity - ID - 1u) < (3u + SHIFT_OF_FAST_SIZE)) 
   {
      step = 1u << (logicElementsQuantity - ID - 1);
      for (uint_fast16_t mask = (UINT_FAST16_MAX >> (sizeof(uint_fast16_t) * 8u - step)) << (!isInverted * step);; mask <<= step * 2u)
      {
         *operations |= mask;
         /* Checking for last valid value, because shifting on value that is greater then or equal to variable size is *undefined behavior**/
         if ((mask & (((uint_fast16_t) 0x1) << (BITWIZE_AND_OF_FAST_SIZE - step * (isInverted > 0u)))))
            break;
      }
      
      for (uint_fast16_t *iterator = operations + 1u, *end = operations + operationsQuantity; iterator < end; ++iterator)
      {
         *iterator = *operations;
      }
   }
   else
   {
      step = 1u << (logicElementsQuantity - ID - 1 - (3u + SHIFT_OF_FAST_SIZE));
      for (uint_fast16_t *iterator = operations + (!isInverted * step), *end = operations + operationsQuantity; iterator < end; iterator += step * 2u)
      {
         memset(iterator, 1u, step * sizeof(uint_fast16_t));
      }
   }
   return operations;
}

static struct operationsStack* computeStackDownToPriority (uint_fast32_t priority, struct operationsStack *stack, uint8_t logicQuantity)
{
   uint8_t isLogicQuantityHigherThanVariableSize = logicQuantity > (3u + SHIFT_OF_FAST_SIZE);
   size_t operationsQuantity = (((size_t) 0x01) << (logicQuantity - (3u + SHIFT_OF_FAST_SIZE))) * isLogicQuantityHigherThanVariableSize + (!isLogicQuantityHigherThanVariableSize);
   uint_fast16_t *operations, *prevOperations, *end;
   struct operationsStack *iterator = stack;
   
   if (iterator->operation == 0u)
   {
      if (!iterator->operations)
      {
         iterator->operations = generateOperationsFromLogicElement(iterator->logicElementID, iterator->flags & 1u, logicQuantity);
      }
      return iterator;
   }
   
   for (struct operationsStack *prev; iterator->operation != 0u;)
   {
      if (iterator->operationPriority < priority)
      {
         return iterator;
      }
      prev = iterator->prev;
      
      if (!iterator->operations)
      {
         iterator->operations = generateOperationsFromLogicElement(iterator->logicElementID, iterator->flags & 1u, logicQuantity);
      }
      operations = iterator->operations;
      
      end = operations + operationsQuantity;
      if (iterator->operation != NEG)
      {
         if (!prev->operations)
         {
            prev->operations = generateOperationsFromLogicElement(prev->logicElementID, prev->flags & 1u, logicQuantity);
         }
         prevOperations = prev->operations;
      }
      
      switch (iterator->operation)
      {
         case NEG:
         {
            if (prev->operations)
               free(prev->operations);
            prev->operations = operations;
            while (operations < end)
            {
               (*operations) = ~(*operations);
               ++operations;
            }
            free(iterator);
            iterator = prev;
            continue;
         }
         case AND:
         {
            while (operations < end)
            {
               (*prevOperations) &= (*operations);
               ++operations;
               ++prevOperations;
            }
            break;
         }
         case NAND:
         {
            while (operations < end)
            {
               (*prevOperations) = ~((*prevOperations) & (*operations));
               ++operations;
               ++prevOperations;
            }
            break;
         }
         case OR:
         {
            while (operations < end)
            {
               (*prevOperations) |= (*operations);
               ++operations;
               ++prevOperations;
            }
            break;
         }
         case NOR:
         {
            while (operations < end)
            {
               (*prevOperations) = ~((*prevOperations) | (*operations));
               ++operations;
               ++prevOperations;
            }
            break;
         }
         case XOR:
         {
            while (operations < end)
            {
               (*prevOperations) ^= (*operations);
               ++operations;
               ++prevOperations;
            }
            break;
         }
         case XNOR:
         {
            while (operations < end)
            {
               (*prevOperations) = ~((*prevOperations) ^ (*operations));
               ++operations;
               ++prevOperations;
            }
            break;
         }
         case GRTR:
         {
            while (operations < end)
            {
               (*prevOperations) &= ~(*operations);
               ++operations;
               ++prevOperations;
            }
            break;
         }
         case GRTREQ:
         {
            while (operations < end)
            {
               (*prevOperations) |= (~(*operations));
               ++operations;
               ++prevOperations;
            }
            break;
         }
         case LESS:
         {
            while (operations < end)
            {
               (*prevOperations) = (~(*prevOperations)) & (*operations);
               ++operations;
               ++prevOperations;
            }
            break;
         }
         case LESSEQ:
//       case IMPL:
         {
            while (operations < end)
            {
               (*prevOperations) = (~(*prevOperations)) | (*operations);
               ++operations;
               ++prevOperations;
            }
            break;
         }
      }
      free(iterator->operations);
      free(iterator);
      iterator = prev;
   }
   return iterator;
}

static int compare (const void *a, const void *b)
{
   const char char_a = *((char*) a);
   const char char_b = *((char*) b);
   return (char_a > char_b) - (char_a < char_b);
}

/* Parse string to truth table. Has hardcoced limit - 32 elements (already 512MiB size), bigger quantities might not fit into memory while parsing */
CCE_PUBLIC_OPTIONS uint_fast16_t* cceParseStringToLogicOperations (const char *const string, uint_fast8_t *const logicQuantity)
{
   char dictionary[33] = ""; //last is '\0'
   uint8_t dictionarySize = 0u;
   size_t stringSize = 0u;
   for (const char *iterator = string; *iterator != '\0'; ++iterator, ++stringSize)
   {
      if ((*iterator >= '0' && *iterator <= '9') || (*iterator >= 'A' && *iterator <= 'Z') || (*iterator >= 'a' && *iterator <= 'z'))
      {
         for (char *jiterator = dictionary; *iterator != *jiterator; ++jiterator)
         {
            if (*jiterator == '\0')
            {
               if ((dictionary + 33) == jiterator) return NULL;
               *jiterator = *iterator;
               *(jiterator + 1u) = '\0';
               ++dictionarySize;
               break;
            }
         }
      }
   }
   if (dictionarySize > 32)
   {
      return NULL;
   }
   qsort(dictionary, dictionarySize, sizeof(char), compare);
   
   struct operationsStack *stack = (struct operationsStack*) malloc(sizeof(struct operationsStack));
   stack->operation = 0u;
   stack->flags = 0u;
   stack->operations = NULL;
   stack->operationPriority = 0u;
   stack->prev = NULL;
   uint_fast32_t currentPriority, lastPriority = 0u; /* 8 - (), 4 - !, 3 - &, 2 - |, 1 - ^, 1 - >, 1 - <, 1 - = */
   uint_fast16_t brackets = 0u;
   uint8_t isInverted = 0u;
   for (const char *iterator = string; *iterator != '\0'; ++iterator)
   {
      switch (*iterator)
      {
         case '(':
         {
            if (isInverted)
            {
               currentPriority = 4u + brackets * 8;
               stack = pushToOperationsStack(stack, currentPriority, NEG, NULL);
               lastPriority = currentPriority;
               isInverted = 0u;
            }
            ++brackets;
            break;
         }
         case ')':
         {
            --brackets;
            break;
         }
         case '!':
         case '~':
         {
            isInverted ^= 1u;
            break;
         }
         case '&':
         case '*':
         {
            if (*(iterator - 1u) == *iterator)
               continue;
            currentPriority = 3u + brackets * 8;
            if (lastPriority >= currentPriority)
            {
               stack = computeStackDownToPriority(currentPriority, stack, dictionarySize);
            }
            stack = pushToOperationsStack(stack, currentPriority, AND + isInverted, NULL);
            isInverted = 0u;
            lastPriority = currentPriority;
            break;
         }
         case '|':
         case '+':
         {
            if (*(iterator - 1u) == *iterator)
               continue;
            currentPriority = 2u + brackets * 8;
            if (lastPriority >= currentPriority)
            {
               stack = computeStackDownToPriority(currentPriority, stack, dictionarySize);
            }
            stack = pushToOperationsStack(stack, currentPriority, OR + isInverted, NULL);
            isInverted = 0u;
            lastPriority = currentPriority;
            break;
         }
         case '^':
         {
            currentPriority = 1u + brackets * 8;
            if (lastPriority >= currentPriority)
            {
               stack = computeStackDownToPriority(currentPriority, stack, dictionarySize);
            }
            stack = pushToOperationsStack(stack, currentPriority, XOR + isInverted, NULL);
            isInverted = 0u;
            lastPriority = currentPriority;
            break;
         }
         case '>':
         {
            if (*(iterator - 1u) == '-' || *(iterator - 1u) == '=')
            {
               stack->operation = IMPL;
               continue;
            }
            currentPriority = 1u + brackets * 8;
            if (lastPriority >= currentPriority)
            {
               stack = computeStackDownToPriority(currentPriority, stack, dictionarySize);
            }
            stack = pushToOperationsStack(stack, currentPriority, GRTR, NULL);
            lastPriority = currentPriority;
            break;
         }
         case '<':
         {
            currentPriority = 1u + brackets * 8;
            if (lastPriority >= currentPriority)
            {
               stack = computeStackDownToPriority(currentPriority, stack, dictionarySize);
            }
            stack = pushToOperationsStack(stack, currentPriority, LESS, NULL);
            lastPriority = currentPriority;
            break;
         }
         case '=':
         {
            if (*(iterator - 1u) == *iterator)
               continue;
            
            if (*(iterator - 1u) == '>' || *(iterator - 1u) == '<')
            {
               ++(stack->operation);
               continue;
            }
            currentPriority = 1u + brackets * 8;
            if (lastPriority >= currentPriority)
            {
               stack = computeStackDownToPriority(currentPriority, stack, dictionarySize);
            }
            stack = pushToOperationsStack(stack, currentPriority, XNOR - isInverted, NULL);
            isInverted = 0u;
            lastPriority = currentPriority;
            break;
         }
         default:
         {
            if ((*iterator >= '0' && *iterator <= '9') || (*iterator >= 'A' && *iterator <= 'Z') || (*iterator >= 'a' && *iterator <= 'z'))
            {
               stack->logicElementID = cceBinarySearch(dictionary, dictionarySize, sizeof(char), sizeof(char), (*iterator));
               stack->flags |= isInverted;
               isInverted = 0u;
            }
         }
      }
   }
   stack = computeStackDownToPriority(0u, stack, dictionarySize);
   uint_fast16_t *operations = stack->operations;
   free(stack);
   if (logicQuantity)
   {
      *logicQuantity = dictionarySize;
   }
   return operations;
}

int cce__initEngine (const char *label, uint16_t globalBoolsQuantity)
{
   // We have only one api yet
   if (cce__initEngine__glfw(label, globalBoolsQuantity) != 0)
      return -1;
      
   g_temporaryBools = (struct UsedTemporaryBools*) calloc(CCE_ALLOCATION_STEP, sizeof(struct UsedTemporaryBools));
   g_temporaryBoolsQuantity = 0u;
   g_temporaryBoolsQuantityAllocated = CCE_ALLOCATION_STEP;
   g_globalBoolsQuantity = globalBoolsQuantity;
   cce_Gvars.globalBools  = (uint_fast16_t*) calloc((globalBoolsQuantity >> SHIFT_OF_FAST_SIZE) + ((globalBoolsQuantity & BITWIZE_AND_OF_FAST_SIZE) > 0u), sizeof(uint_fast16_t));
   cceInitEndianConversion();
   return 0;
}

void cce__engineUpdate (void)
{
   cce__engineUpdate__api();
   if (g_flags & CCE_PROCESS_TEMPORARY_BOOLS_ARRAY)
   {
      updateTemporaryBoolsArray();
   }
}

void cce__terminateEngine (void)
{
   //stopAL(AL);
   for (struct UsedTemporaryBools *iterator = g_temporaryBools, *end = g_temporaryBools + g_temporaryBoolsQuantity; iterator < end; ++iterator)
   {
      free(iterator->temporaryBools);
   }
   free(g_temporaryBools);
   cce__terminateEngine__api();
   cceTerminateTemporaryDirectory();
}

void cce__doNothing (void)
{
   return;
}
