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
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "../../include/cce/engine_common.h"
#include "../../include/cce/utils.h"
#include "engine_common_keyboard.h"

// If y is string literal, all optimizing compilers will optimize strlen call away
#define CCE_MEMEQ(x,y) (memcmp(x, y, strlen(y)) == 0)

#define CCE_LEFT_RIGHT_CHOOSE(str, l, r) (str[0] == 'l' && (str[1] == '\0' || CCE_STREQ(str + 1, "eft"))  ? l : \
                                          str[0] == 'r' && (str[1] == '\0' || CCE_STREQ(str + 1, "ight")) ? r : CCE_KEY_UNKNOWN)

static const char *const keypad = "keypad";
static const char *const nter   = "nter";

// HUGE mess. But a lot of name variants are supported. And really fast (at least should be)
// buf MUST have size at least 16
static uint8_t cce__keyFromName (char *buf)
{
   char *it;
   uint8_t isRight = 0;
   if (buf[1] == '\0')
   {
      switch (buf[0])
      {
         case '\0':
            return CCE_KEY_NOKEY;
         case '\b':
            return CCE_KEY_BACKSPACE;
         case '\t':
            return CCE_KEY_TAB;
         case '\n':
            return CCE_KEY_ENTER;
         case '\r':
            return CCE_KEY_HOME;
         case 0x1B: // '\e'
            return CCE_KEY_ESCAPE;
         case ' ':
            return CCE_KEY_SPACE;
         case '\'':
            return CCE_KEY_APOSTROPHE;
         case ',':
            return CCE_KEY_COMMA;
         case '-':
            return CCE_KEY_MINUS;
         case '.':
            return CCE_KEY_PERIOD;
         case '/':
            return CCE_KEY_SLASH;
         case '0':
            return CCE_KEY_0;
         case '1':
            return CCE_KEY_1;
         case '2':
            return CCE_KEY_2;
         case '3':
            return CCE_KEY_3;
         case '4':
            return CCE_KEY_4;
         case '5':
            return CCE_KEY_5;
         case '6':
            return CCE_KEY_6;
         case '7':
            return CCE_KEY_7;
         case '8':
            return CCE_KEY_8;
         case '9':
            return CCE_KEY_9;
         case ';':
            return CCE_KEY_SEMICOLON;
         case '=':
            return CCE_KEY_EQUAL;
         case '[':
            return CCE_KEY_LBRACKET;
         case '\\':
            return CCE_KEY_BACKSLASH;
         case ']':
            return CCE_KEY_RBRACKET;
         case '`':
            return CCE_KEY_GRAVE_ACCENT;
         case 'a':
            return CCE_KEY_A;
         case 'b':
            return CCE_KEY_B;
         case 'c':
            return CCE_KEY_C;
         case 'd':
            return CCE_KEY_D;
         case 'e':
            return CCE_KEY_E;
         case 'f':
            return CCE_KEY_F;
         case 'g':
            return CCE_KEY_G;
         case 'h':
            return CCE_KEY_H;
         case 'i':
            return CCE_KEY_I;
         case 'j':
            return CCE_KEY_J;
         case 'k':
            return CCE_KEY_K;
         case 'l':
            return CCE_KEY_L;
         case 'm':
            return CCE_KEY_M;
         case 'n':
            return CCE_KEY_N;
         case 'o':
            return CCE_KEY_O;
         case 'p':
            return CCE_KEY_P;
         case 'q':
            return CCE_KEY_Q;
         case 'r':
            return CCE_KEY_R;
         case 's':
            return CCE_KEY_S;
         case 't':
            return CCE_KEY_T;
         case 'u':
            return CCE_KEY_U;
         case 'v':
            return CCE_KEY_V;
         case 'w':
            return CCE_KEY_W;
         case 'x':
            return CCE_KEY_X;
         case 'y':
            return CCE_KEY_Y;
         case 'z':
            return CCE_KEY_Z;
         default:
            return CCE_KEY_UNKNOWN;
      }
   }
   switch (buf[0])
   {
      case 'a':
         switch (buf[1])
         {
            case 'c':
               if (CCE_MEMEQ(buf + 2, "cent") && (buf[6] == '\0' || (CCE_MEMEQ(it = buf + 6 + cceIsCharWhitespaceLike(buf[6]), "gr") && (it[2] == '\0' || CCE_STREQ(it + 2, "ave"))))) // accent, `
                  return CCE_KEY_GRAVE_ACCENT;
               return CCE_KEY_UNKNOWN;
            case 'd':
               if (buf[2] == 'd' && (CCE_STREQ(it = buf + 3 + cceIsCharWhitespaceLike(buf[3]), "kp") || CCE_STREQ(it, keypad)))
                  return CCE_KEY_KP_PLUS;
               return CCE_KEY_UNKNOWN;
            case 'l':
               if (buf[2] != 't')
                  return CCE_KEY_UNKNOWN;
               // alt
               it = buf + 3 + cceIsCharWhitespaceLike(buf[3]);
               if (CCE_STREQ(it, "gr") || CCE_STREQ(it, "right"))
                  return CCE_KEY_RALT;
               if (CCE_STREQ(it, "left"))
                  return CCE_KEY_LALT;
               return CCE_KEY_UNKNOWN;
            case 'p':
               if (CCE_STREQ(buf + 2, "ostrophe")) // apostrophe, '
                  return CCE_KEY_APOSTROPHE;
               return CCE_KEY_UNKNOWN;
            case 'r':
               if (!CCE_MEMEQ(buf + 2, "row")) // arrow
                  return CCE_KEY_UNKNOWN;
               it = buf + 5 + cceIsCharWhitespaceLike(buf[5]);
               if (CCE_STREQ(it, "up"))
                  return CCE_KEY_UP_ARROW;
               if (CCE_STREQ(it, "down"))
                  return CCE_KEY_DOWN_ARROW;
               return CCE_LEFT_RIGHT_CHOOSE(it, CCE_KEY_LEFT_ARROW, CCE_KEY_RIGHT_ARROW);
            default:
               return CCE_KEY_UNKNOWN;
         }
      case 'b':
         switch (buf[1])
         {
            case 'a':
               if (CCE_STREQ(buf + 2, "ckspace")) // backspace
                  return CCE_KEY_BACKSPACE;
               if (CCE_STREQ(buf + 2, "ckslash")) // backslash
                  return CCE_KEY_BACKSLASH;
               return CCE_KEY_UNKNOWN;
            case 'r':
               if (CCE_STREQ(buf + 2, "eak")) // break
                  return CCE_KEY_PAUSE;
               if (CCE_MEMEQ(buf + 2, "acket")) // bracket
               {
                  it = buf + 7 + cceIsCharWhitespaceLike(buf[7]);
                  return CCE_LEFT_RIGHT_CHOOSE(it, CCE_KEY_LEFT_BRACKET, CCE_KEY_RIGHT_BRACKET);
               }
               return CCE_KEY_UNKNOWN;
            default:
               return CCE_KEY_UNKNOWN;
         }
      case 'c':
         switch (buf[1])
         {
            case 'a':
               if (CCE_MEMEQ(buf + 2, "ps") && (buf[4] == '\0' || CCE_STREQ(buf + 4 + (cceIsCharWhitespaceLike(buf[4])), "lock"))) // capslock
                  return CCE_KEY_CAPSLOCK;
               return CCE_KEY_UNKNOWN;
            case 'm':
               if (buf[3] != 'd')
                  return CCE_KEY_UNKNOWN;
               it = buf + 3 + cceIsCharWhitespaceLike(buf[3]);
               return CCE_LEFT_RIGHT_CHOOSE(it, CCE_KEY_LSUPER, CCE_KEY_RSUPER);
            case 'o':
               switch (buf[2])
               {
                  case 'm':
                     if (!CCE_MEMEQ(buf + 3, "ma"))
                        return CCE_KEY_UNKNOWN;
                     if (CCE_MEMEQ(buf + 5, "nd")) // command
                     {
                        it = buf + 7 + cceIsCharWhitespaceLike(buf[7]);
                        return CCE_LEFT_RIGHT_CHOOSE(it, CCE_KEY_LSUPER, CCE_KEY_RSUPER);
                     }
                     it = buf + 5;
                     if (*it == '\0')
                        return CCE_KEY_COMMA;
                     it += cceIsCharWhitespaceLike(*it);
                     if (CCE_STREQ(it, "kp") || CCE_STREQ(it, keypad))
                        return CCE_KEY_KP_COMMA;
                     return CCE_KEY_UNKNOWN;
                  case 'n':
                     if (!CCE_MEMEQ(buf + 3, "trol")) // control
                        return CCE_KEY_UNKNOWN;
                     it = buf + 7;
                     goto CONTROL_KEY;
                  default:
                     return CCE_KEY_UNKNOWN;
               }
            case 't':
               if (CCE_MEMEQ(buf + 2, "rl")) // ctrl
                  return CCE_KEY_UNKNOWN;
               it = buf + 4;
CONTROL_KEY:
               it += cceIsCharWhitespaceLike(*it);
               return CCE_LEFT_RIGHT_CHOOSE(it, CCE_KEY_LCONTROL, CCE_KEY_RCONTROL);
            default:
               return CCE_KEY_UNKNOWN;
         }
      case 'd':
         switch (buf[1])
         {
            case 'e':
               if (buf[2] == 'l' && (buf[3] == '\0' || CCE_STREQ(buf + 3, "ete"))) // del or delete
                  return CCE_KEY_DELETE;
               return CCE_KEY_UNKNOWN;
            case 'i':
               if (buf[2] != 'v')
                  return CCE_KEY_UNKNOWN;
               it = buf + 3 + CCE_MEMEQ(buf + 3, "ide"); // divide
               if (*it == '\0')
                  return CCE_KEY_SLASH;
               it += cceIsCharWhitespaceLike(*it);
               if (CCE_STREQ(it, "kp") || CCE_STREQ(it, keypad))
                  return CCE_KEY_KP_DIVIDE;
               return CCE_KEY_UNKNOWN;
            case 'o':
               if (CCE_MEMEQ(buf + 2, "wn") && (buf[4] == '\0' || CCE_STREQ(buf + 4 + cceIsCharWhitespaceLike(buf[4]), "arrow"))) // down
                  return CCE_KEY_DOWN_ARROW;
               return CCE_KEY_UNKNOWN;
            default:
               return CCE_KEY_UNKNOWN;
         }
      case 'e':
         switch (buf[1])
         {
            case 'n':
               if (CCE_STREQ(buf + 2, "d")) // end
                  return CCE_KEY_END;
               if (CCE_MEMEQ(buf + 2, nter + 1)) // enter
               {
                  it = buf + 5;
                  switch (*it)
                     case '\0':
                        return CCE_KEY_ENTER;
                     case ' ':
                     case '_':
                     case '-':
                        ++it;
                        if (*it != 'k')
                           return CCE_KEY_UNKNOWN;
                        // fallthrough
                     case 'k':
                        ++it;
                        if (CCE_STREQ(it, "p") || CCE_STREQ(it, keypad + 1))
                           return CCE_KEY_KP_ENTER;
                        return CCE_KEY_UNKNOWN;
               }
               return CCE_KEY_UNKNOWN;
            case 's':
               if (CCE_STREQ(buf, "c") || CCE_STREQ(buf, "cape")) // esc or escape
                  return CCE_KEY_ESCAPE;
               return CCE_KEY_UNKNOWN;
            default:
               return CCE_KEY_UNKNOWN;
         }
      case 'f':
         switch (buf[1])
         {
            case '1':
               if (buf[3] != '\0')
                  return CCE_KEY_UNKNOWN;
               switch (buf[2])
               {
                  case '\0':
                     return CCE_KEY_F1;
                  case '0':
                     return CCE_KEY_F10;
                  case '1':
                     return CCE_KEY_F11;
                  case '2':
                     return CCE_KEY_F12;
                  case '3':
                     return CCE_KEY_F13;
                  case '4':
                     return CCE_KEY_F14;
                  case '5':
                     return CCE_KEY_F15;
                  case '6':
                     return CCE_KEY_F16;
                  case '7':
                     return CCE_KEY_F17;
                  case '8':
                     return CCE_KEY_F18;
                  case '9':
                     return CCE_KEY_F19;
                  default:
                     return CCE_KEY_UNKNOWN;
               }
            case '2':
               if (buf[3] != '\0')
                  return CCE_KEY_UNKNOWN;
               switch (buf[2])
               {
                  case '\0':
                     return CCE_KEY_F2;
                  case '0':
                     return CCE_KEY_F20;
                  case '1':
                     return CCE_KEY_F21;
                  case '2':
                     return CCE_KEY_F22;
                  case '3':
                     return CCE_KEY_F23;
                  case '4':
                     return CCE_KEY_F24;
                  case '5':
                     return CCE_KEY_F25;
                  default:
                     return CCE_KEY_UNKNOWN;
               }
               break;
            case '3':
               if (buf[2] != '\0')
                  return CCE_KEY_UNKNOWN;
               return CCE_KEY_F3;
            case '4':
               if (buf[2] != '\0')
                  return CCE_KEY_UNKNOWN;
               return CCE_KEY_F4;
            case '5':
               if (buf[2] != '\0')
                  return CCE_KEY_UNKNOWN;
               return CCE_KEY_F5;
            case '6':
               if (buf[2] != '\0')
                  return CCE_KEY_UNKNOWN;
               return CCE_KEY_F6;
            case '7':
               if (buf[2] != '\0')
                  return CCE_KEY_UNKNOWN;
               return CCE_KEY_F7;
            case '8':
               if (buf[2] != '\0')
                  return CCE_KEY_UNKNOWN;
               return CCE_KEY_F8;
            case '9':
               if (buf[2] != '\0')
                  return CCE_KEY_UNKNOWN;
               return CCE_KEY_F9;
            default:
               return CCE_KEY_UNKNOWN;
         }
      case 'g':
         if (buf[1] != 'r')
            return CCE_KEY_UNKNOWN;
         it = buf + 2 + (-CCE_MEMEQ(buf + 2, "ave") & 3);
         it += cceIsCharWhitespaceLike(*it);
         if ((it - buf) < 5 && CCE_STREQ(it, "alt"))
            return CCE_KEY_RALT;
         if (CCE_STREQ(it, "accent"))
            return CCE_KEY_GRAVE_ACCENT;
         return CCE_KEY_UNKNOWN;
      case 'h':
         if (CCE_STREQ(buf + 1, "ome")) // home
            return CCE_KEY_HOME;
         if (CCE_STREQ(buf + 1, "yphen")) // hyphen
            return CCE_KEY_MINUS;
         return CCE_KEY_UNKNOWN;
      case 'i':
         if (CCE_MEMEQ(buf + 1, "ns") && (buf[3] == '\0' || CCE_STREQ(buf + 3, "ert"))) // ins or insert
            return CCE_KEY_INSERT;
         return CCE_KEY_UNKNOWN;
      case 'k':
         it = buf + 2;
         switch (buf[1])
         {
            case 'e':
               if (!CCE_MEMEQ(it, keypad + 2)) // keypad
                  return CCE_KEY_UNKNOWN;
               it = buf + 6;
               // fallthrough
            case 'p': // kp
               if (cceIsCharWhitespaceLike(*it))
                  ++it;
               if (it[1] == '\0')
                  switch (*it)
                  {
                     case '+':
                        return CCE_KEY_KP_PLUS;
                     case '-':
                        return CCE_KEY_KP_MINUS;
                     case '*':
                        return CCE_KEY_KP_MULTIPLY;
                     case '/':
                        return CCE_KEY_KP_DIVIDE;
                     case ',':
                        return CCE_KEY_KP_COMMA;
                     case '0':
                        return CCE_KEY_KP_0;
                     case '1':
                        return CCE_KEY_KP_1;
                     case '2':
                        return CCE_KEY_KP_2;
                     case '3':
                        return CCE_KEY_KP_3;
                     case '4':
                        return CCE_KEY_KP_4;
                     case '5':
                        return CCE_KEY_KP_5;
                     case '6':
                        return CCE_KEY_KP_6;
                     case '7':
                        return CCE_KEY_KP_7;
                     case '8':
                        return CCE_KEY_KP_8;
                     case '9':
                        return CCE_KEY_KP_9;
                     default: 
                        return CCE_KEY_UNKNOWN;
                  }
               return CCE_KEY_UNKNOWN;
               switch (*it)
               {
                  case 'a':
                     if (CCE_STREQ(it + 1, "dd")) // add
                        return CCE_KEY_KP_PLUS;
                     if (CCE_STREQ(it + 1, "sterisk")) // asterisk
                        return CCE_KEY_KP_MULTIPLY;
                     return CCE_KEY_UNKNOWN;
                  case 'c':
                     if (CCE_STREQ(it + 1, "omma")) // comma
                        return CCE_KEY_KP_COMMA;
                     return CCE_KEY_UNKNOWN;
                  case 'd':
                     if (CCE_MEMEQ(it + 1, "iv") && (it[3] == '\0' || CCE_STREQ(it + 3, "ide"))) // div or divide
                        return CCE_KEY_KP_DIVIDE;
                     return CCE_KEY_UNKNOWN;
                  case 'e':
                     if (CCE_STREQ(it + 1, nter))
                        return CCE_KEY_KP_ENTER;
                     return CCE_KEY_UNKNOWN;
                  case 'h':
                     if (CCE_STREQ(it + 1, "yphen")) // hyphen
                        return CCE_KEY_KP_MINUS;
                     return CCE_KEY_UNKNOWN;
                  case 'm':
                     switch (it[1])
                     {
                        case 'i':
                           if (CCE_STREQ(it + 2, "nus")) // minus
                              return CCE_KEY_MINUS;
                           return CCE_KEY_UNKNOWN;
                        case 'u':
                           if (it[2] == 'l' && (it[3] == '\0' || CCE_STREQ(it + 4, "tiply"))) // multiply
                              return CCE_KEY_KP_MULTIPLY;
                           return CCE_KEY_UNKNOWN;
                        default:
                           return CCE_KEY_UNKNOWN;
                     }
                  case 'n':
                     if (!CCE_MEMEQ(it + 1, "um"))
                        return CCE_KEY_UNKNOWN;
                     it += 3 + cceIsCharWhitespaceLike(*it);
                     if (CCE_STREQ(it, "lock"))
                        return CCE_KEY_NUMLOCK;
                     return CCE_KEY_UNKNOWN;
                  case 'p':
                     if (CCE_STREQ(it + 1, "lus")) // plus
                        return CCE_KEY_KP_PLUS;
                     return CCE_KEY_UNKNOWN;
                  case 'r':
                     if (CCE_STREQ(it + 1, "eturn")) // return
                        return CCE_KEY_KP_ENTER;
                     return CCE_KEY_UNKNOWN;
                  case 's':
                     if (CCE_STREQ(it + 1, "lash")) // slash
                        return CCE_KEY_KP_DIVIDE;
                     if (CCE_STREQ(it + 1, "ubtract")) // subtract
                        return CCE_KEY_KP_MINUS;
                     return CCE_KEY_UNKNOWN;
                  default:
                     return CCE_KEY_UNKNOWN;
               }
            
            default:
               return CCE_KEY_UNKNOWN;
         }
      // lalt and ralt, lshift and rshift ...
      case 'r':
         isRight = -1;
         it = buf + 1 + CCE_MEMEQ(buf + 1, "ight") * 4 + cceIsCharWhitespaceLike(buf[5]);
         goto SIDE_KEY;
      case 'l':
         it = buf + 1 + CCE_MEMEQ(buf + 1, "eft") * 3 + cceIsCharWhitespaceLike(buf[4]);
SIDE_KEY:
         switch (*it)
         {
            case '\0':
               return CCE_KEY_LEFT_ARROW + (isRight & (CCE_KEY_RIGHT_ARROW - CCE_KEY_LEFT_ARROW));
            case 'a':
               if (CCE_STREQ(it + 1, "lt")) // alt
                  return CCE_KEY_LALT + (isRight & (CCE_KEY_RALT - CCE_KEY_LALT));
               if (CCE_STREQ(it + 1, "rrow")) // arrow
                  return CCE_KEY_LEFT_ARROW + (isRight & (CCE_KEY_RIGHT_ARROW - CCE_KEY_LEFT_ARROW));
               return CCE_KEY_UNKNOWN;
            case 'b':
               if (CCE_STREQ(it + 1, "racket")) // bracket
                  return CCE_KEY_LEFT_BRACKET + (isRight & (CCE_KEY_RIGHT_BRACKET - CCE_KEY_LEFT_BRACKET));
               return CCE_KEY_UNKNOWN;
            case 'c':
               if (CCE_STREQ(it + 1, "trl") || CCE_STREQ(it + 1, "ontrol")) // ctrl or control
                  return CCE_KEY_LCONTROL + (isRight & (CCE_KEY_RCONTROL - CCE_KEY_LCONTROL));
               if (CCE_STREQ(it + 1, "md") || CCE_STREQ(it + 1, "ommand")) // cmd or command
                  return CCE_KEY_LSUPER + (isRight & (CCE_KEY_RSUPER - CCE_KEY_LSUPER));
               return CCE_KEY_UNKNOWN;
            case 'e':
               if (!(isRight != 0 && CCE_MEMEQ(buf + 2, "turn"))) // return
                  return CCE_KEY_UNKNOWN;
               if (buf[6] == '\0')
                  return CCE_KEY_ENTER;
               it = buf + 6 + cceIsCharWhitespaceLike(buf[6]);
               if (CCE_STREQ(it, "kp") || CCE_STREQ(it, keypad))
                  return CCE_KEY_KP_ENTER;
               return CCE_KEY_UNKNOWN;
            case 's':
               if (CCE_STREQ(it + 1, "hift")) // shift
                  return CCE_KEY_LSHIFT + (isRight & (CCE_KEY_RSHIFT - CCE_KEY_LSHIFT));
               if (CCE_STREQ(it + 1, "uper")) // super
                  return CCE_KEY_LSUPER + (isRight & (CCE_KEY_RSUPER - CCE_KEY_LSUPER));
               return CCE_KEY_UNKNOWN;
            case 'w':
               if (CCE_MEMEQ(it + 1, "in") && (it[3] == '\0' || CCE_STREQ(it + 3, "dows"))) // win or windows
                  return CCE_KEY_LSUPER + (isRight & (CCE_KEY_RSUPER - CCE_KEY_LSUPER));
               return CCE_KEY_UNKNOWN;
            default:
               return CCE_KEY_UNKNOWN;
         }
      case 'm':
         switch (buf[1])
         {
            case 'e':
               if (CCE_STREQ(buf + 2, "enu"))
                  return CCE_KEY_MENU;
               return CCE_KEY_UNKNOWN;
            case 'i':
               if (CCE_STREQ(buf + 2, "nus"))
                  return CCE_KEY_MINUS;
               return CCE_KEY_UNKNOWN;
            default:
               return CCE_KEY_UNKNOWN;
         }
      case 'n':
         if (CCE_MEMEQ(buf + 1, "um") && (buf[3] == '\0' || CCE_STREQ(buf + 3 + cceIsCharWhitespaceLike(buf[3]), "lock")))
            return CCE_KEY_NUMLOCK;
         return CCE_KEY_UNKNOWN;
      case 'p':
         switch (buf[1])
         {
            case 'a':
               if (CCE_MEMEQ(buf + 2, "use") && (buf[5] == '\0' || CCE_STREQ(buf + 5 + cceIsCharWhitespaceLike(buf[5]), "break"))) // pause or pausebreak
                  return CCE_KEY_PAUSE;
               if (!CCE_MEMEQ(buf + 2, "ge")) // page
                  return CCE_KEY_UNKNOWN;
               it = buf + 4 + cceIsCharWhitespaceLike(buf[4]);
               if (CCE_STREQ(it, "up"))
                  return CCE_KEY_PAGEUP;
               if (CCE_STREQ(it, "down"))
                  return CCE_KEY_PAGEDOWN;
               return CCE_KEY_UNKNOWN;
            case 'g':
               if (CCE_STREQ(buf + 2, "up")) // PgUp
                  return CCE_KEY_PAGEUP;
               if (CCE_STREQ(buf + 2, "dn")) // PgDn
                  return CCE_KEY_PAGEDOWN;
               return CCE_KEY_UNKNOWN;
            case 'r':
               if (CCE_STREQ(buf + 2, "scn") || (CCE_MEMEQ(buf + 2, "int") && CCE_STREQ(buf + 5 + cceIsCharWhitespaceLike(buf[5]), "screen"))) // prscn or printscreen
                  return CCE_KEY_PRINTSCREEN;
               return CCE_KEY_UNKNOWN;
            default:
               return CCE_KEY_UNKNOWN;
         }
      case 's':
         switch (buf[1])
         {
            case 'c':
               if (!CCE_MEMEQ(buf + 2, "roll")) // scroll
                  return CCE_KEY_UNKNOWN;
               it = buf + 6 + cceIsCharWhitespaceLike(buf[6]);
               if (CCE_STREQ(it, "lock"))
                  return CCE_KEY_SCROLLLOCK;
               return CCE_KEY_UNKNOWN;
            case 'h':
               if (!CCE_MEMEQ(buf + 2, "ift")) // shift
                  return CCE_KEY_UNKNOWN;
               it = buf + 5 + cceIsCharWhitespaceLike(buf[5]);
               return CCE_LEFT_RIGHT_CHOOSE(it, CCE_KEY_LSHIFT, CCE_KEY_RSHIFT);
            case 'l':
               if (CCE_STREQ(buf + 2, "ash")) // slash
                  return CCE_KEY_SLASH;
               return CCE_KEY_UNKNOWN;
            case 'p':
               if (CCE_STREQ(buf + 2, "ace")) // space
                  return CCE_KEY_SPACE;
               return CCE_KEY_UNKNOWN;
            case 'u':
               if (CCE_MEMEQ(buf + 2, "per")) // super
               {
                  it = buf + 5 + cceIsCharWhitespaceLike(buf[5]);
                  return CCE_LEFT_RIGHT_CHOOSE(it, CCE_KEY_LSUPER, CCE_KEY_RSUPER);
               }
               if (CCE_MEMEQ(buf + 2, "btract")) // subtract
               {
                  if (buf[8] == '\0')
                     return CCE_KEY_MINUS;
                  it = buf + 8 + cceIsCharWhitespaceLike(buf[8]);
                  if (CCE_STREQ(it, "kp") || CCE_STREQ(it, keypad))
                     return CCE_KEY_KP_MINUS;
               }
               return CCE_KEY_UNKNOWN;
            case 'y':
               if (buf[3] != 's') // sys
                  return CCE_KEY_UNKNOWN;
               it = buf + 3 + (-CCE_MEMEQ(buf + 3, "tem") & 3);
               it += cceIsCharWhitespaceLike(*it);
               if (CCE_STREQ(it, "rq") || CCE_STREQ(it, "request")) // sysrq or systemrequest
                  return CCE_KEY_PRINTSCREEN;
               return CCE_KEY_UNKNOWN;
            default:
               return CCE_KEY_UNKNOWN;
         }
      case 't':
         if (CCE_STREQ(buf + 1, "ab")) // tab
            return CCE_KEY_TAB;
         return CCE_KEY_UNKNOWN;
      case 'u':
         if (buf[1] == 'p' && (buf[2] == '\0' || CCE_STREQ(buf + 2 + cceIsCharWhitespaceLike(buf[2]), "arrow")))
            return CCE_KEY_UP_ARROW;
         return CCE_KEY_UNKNOWN;
      case 'w':
         if (!CCE_MEMEQ(buf + 1, "in")) // win
            return CCE_KEY_UNKNOWN;
         it = buf + 3 + CCE_MEMEQ(buf + 3, "dows"); // windows
         it += cceIsCharWhitespaceLike(*it);
         return CCE_LEFT_RIGHT_CHOOSE(it, CCE_KEY_LSUPER, CCE_KEY_RSUPER);
      default:
         return CCE_KEY_UNKNOWN;
   }
}

uint8_t cceKeyFromName (const char *name)
{
   char buf[16];
   strncpy(buf, name, 16);
   cceMemoryToLowercase(buf, 15);
   return cce__keyFromName(buf);
}

#define KEYS_FROM_STRING_N(str, n) \
char buf[16]; \
uint8_t result[n] = {0}; \
for (uint8_t *resptr = result, *end = result + n; resptr < end; ++resptr) \
{ \
   while (isspace(*str)) \
   { \
      ++str; \
   } \
   const char *it = str; \
   for (;; ++it) \
   { \
      if (*it == '\0') \
      { \
         end = resptr + 1; /* break outer loop */ \
         break; \
      } \
      if (cceIsCharDelimiter(*it)) \
      { \
         if (it == str) /* separator is the first symbol in substring */ \
            continue; \
         break; \
      } \
      if (isspace(*it)) \
      { \
         if (((it[1] == 'l' || it[1] == 'L') && (it[2] == '\0' || isspace(it[2]) || cceIsCharDelimiter(*it) || it[2] == 'e' || it[2] == 'E')) || \
             ((it[1] == 'r' || it[1] == 'R') && (it[2] == '\0' || isspace(it[2]) || cceIsCharDelimiter(*it) || it[2] == 'i' || it[2] == 'I'))) /* left (l) or right (r) */ \
            continue; \
         break; \
      } \
   } \
   memset(buf, 0, CCE_MAX(it - str, 16)); \
   memcpy(buf, str, CCE_MAX(it - str, 16)); \
   cceMemoryToLowercase(buf, CCE_MAX(it - str, 16) - 1); \
   if (it - str < 16) \
      buf[(it - str) - 1] = '\0'; \
   *resptr = cce__keyFromName(buf); \
   str = it + 1; \
}

struct cce_u8vec2 cceStringToKeys2 (const char *str)
{
   KEYS_FROM_STRING_N(str, 2)
   return (struct cce_u8vec2) {result[0], result[1]};
}

struct cce_u8vec3 cceStringToKeys3 (const char *str)
{
   KEYS_FROM_STRING_N(str, 3)
   return (struct cce_u8vec3) {result[0], result[1], result[2]};
}

struct cce_u8vec4 cceStringToKeys4 (const char *str)
{
   KEYS_FROM_STRING_N(str, 4)
   return (struct cce_u8vec4) {result[0], result[1], result[2], result[3]};
}

int keyIniCallback(void *data, const char *name, const char *value)
{
   struct cce_keys *vals = data;
   char buf[15];
   strncpy(buf, name, 15);
   cceMemoryToLowercase(buf, 14);
   if (CCE_STREQ(buf, "horizontalmove") || CCE_STREQ(buf, "horizontalaxis") || CCE_STREQ(buf, "horizontal"))
   {
      vals->horizontalAxis = cceStringToKeys2(value);
   }
   else if (CCE_STREQ(buf, "verticalmove") || CCE_STREQ(buf, "verticalaxis") || CCE_STREQ(buf, "vertical"))
   {
      vals->verticalAxis = cceStringToKeys2(value);
   }
   else if (CCE_STREQ(buf, "buttona") || CCE_STREQ(buf, "a"))
   {
      vals->buttonA = cceStringToKeys2(value);
   }
   else if (CCE_STREQ(buf, "buttonb") || CCE_STREQ(buf, "b"))
   {
      vals->buttonB = cceStringToKeys2(value);
   }
   else if (CCE_STREQ(buf, "buttonx") || CCE_STREQ(buf, "x"))
   {
      vals->buttonX = cceStringToKeys2(value);
   }
   else if (CCE_STREQ(buf, "buttony") || CCE_STREQ(buf, "y"))
   {
      vals->buttonY = cceStringToKeys2(value);
   }
   else if (CCE_STREQ(buf, "leftbutton") || CCE_STREQ(buf, "lb"))
   {
      vals->buttonL = cceStringToKeys2(value);
   }
   else if (CCE_STREQ(buf, "rightbutton") || CCE_STREQ(buf, "rb"))
   {
      vals->buttonR = cceStringToKeys2(value);
   }
   else if (CCE_STREQ(buf, "lefttrigger") || CCE_STREQ(buf, "lt"))
   {
      vals->triggerL = cceStringToKeys2(value);
   }
   else if (CCE_STREQ(buf, "righttrigger") || CCE_STREQ(buf, "rt"))
   {
      vals->triggerR = cceStringToKeys2(value);
   }
   else if (CCE_STREQ(buf, "select") || CCE_STREQ(buf, "sel"))
   {
      vals->select = cceStringToKeys2(value);
   }
   else if (CCE_STREQ(buf, "start"))
   {
      vals->start = cceStringToKeys2(value);
   }
   else
   {
      fprintf(stderr, "ENGINE::INI::KEY_PARSER_ERROR:\nUnknown binding %s\n", name);
   }
   return 0;
}
