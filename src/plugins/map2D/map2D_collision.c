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
#include <stdint.h>

#include "map2D_internal.h"

#define CCE_COLLIDER (CCE_RECTANGLE_COLLIDER | CCE_CIRCLE_COLLIDER)

#define COLLIDERS_ADD_ELEMENT_POSITION(collider1, collider2, element1, element2) \
collider1.data.rectangle.position.x += element1.position.x; \
collider1.data.rectangle.position.y += element1.position.y; \
collider2.data.rectangle.position.x += element2.position.x; \
collider2.data.rectangle.position.y += element2.position.y

static inline uint8_t checkCollisionMap2DnoFlips (struct cce_collidermap2D a, struct cce_collidermap2D b, struct cce_element _a, struct cce_element _b)
{
   COLLIDERS_ADD_ELEMENT_POSITION(a, b, _a, _b);
   switch (a.type | (b.type << 4))
   {
      case CCE_RECTANGLE_COLLIDER | (CCE_RECTANGLE_COLLIDER << 4):
         return cceCheckCollisionRect2D(a.data.rectangle, b.data.rectangle);
      case CCE_CIRCLE_COLLIDER    | (CCE_RECTANGLE_COLLIDER << 4):
         return cceCheckCollisionCirRect2D(a.data.circle, b.data.rectangle);
      case CCE_RECTANGLE_COLLIDER | (CCE_CIRCLE_COLLIDER << 4):
         return cceCheckCollisionCirRect2D(b.data.circle, a.data.rectangle);
      case CCE_CIRCLE_COLLIDER    | (CCE_CIRCLE_COLLIDER << 4):
         return cceCheckCollisionCir2D(a.data.circle, b.data.circle);
      default:
         return 0;
   }
}

#define RECTANGLE_COLLIDER_FLIP_SYNC(collider, element) \
collider.data.rectangle.position.x = (element.flags & CCE_ELEMENT_FLIP_HORIZONTALLY) ? -collider.data.rectangle.position.x - collider.data.rectangle.size.x : collider.data.rectangle.position.x; \
collider.data.rectangle.position.y = (element.flags & CCE_ELEMENT_FLIP_VERTICALLY)   ? -collider.data.rectangle.position.y - collider.data.rectangle.size.y : collider.data.rectangle.position.y;

#define CIRCLE_COLLIDER_FLIP_SYNC(collider, element) \
collider.data.circle.position.x = (element.flags & CCE_ELEMENT_FLIP_HORIZONTALLY) ? -collider.data.circle.position.x - collider.data.circle.diameter : collider.data.circle.position.x; \
collider.data.circle.position.y = (element.flags & CCE_ELEMENT_FLIP_VERTICALLY)   ? -collider.data.circle.position.y - collider.data.circle.diameter : collider.data.circle.position.y

static inline uint8_t checkCollisionMap2DnoRotation (struct cce_collidermap2D a, struct cce_collidermap2D b, struct cce_element _a, struct cce_element _b)
{
   switch (a.type | (b.type << 4))
   {
      case CCE_RECTANGLE_COLLIDER | (CCE_RECTANGLE_COLLIDER << 4):
         RECTANGLE_COLLIDER_FLIP_SYNC(a, _a);
         RECTANGLE_COLLIDER_FLIP_SYNC(b, _b);
         COLLIDERS_ADD_ELEMENT_POSITION(a, b, _a, _b);
         return cceCheckCollisionRect2D(a.data.rectangle, b.data.rectangle);
      case CCE_CIRCLE_COLLIDER    | (CCE_RECTANGLE_COLLIDER << 4):
         CIRCLE_COLLIDER_FLIP_SYNC(a, _a);
         RECTANGLE_COLLIDER_FLIP_SYNC(b, _b);
         COLLIDERS_ADD_ELEMENT_POSITION(a, b, _a, _b);
         return cceCheckCollisionCirRect2D(a.data.circle, b.data.rectangle);
      case CCE_RECTANGLE_COLLIDER | (CCE_CIRCLE_COLLIDER << 4):
         RECTANGLE_COLLIDER_FLIP_SYNC(a, _a);
         CIRCLE_COLLIDER_FLIP_SYNC(b, _b);
         COLLIDERS_ADD_ELEMENT_POSITION(a, b, _a, _b);
         return cceCheckCollisionCirRect2D(b.data.circle, a.data.rectangle);
      case CCE_CIRCLE_COLLIDER    | (CCE_CIRCLE_COLLIDER << 4):
         CIRCLE_COLLIDER_FLIP_SYNC(a, _a);
         CIRCLE_COLLIDER_FLIP_SYNC(b, _b);
         COLLIDERS_ADD_ELEMENT_POSITION(a, b, _a, _b);
         return cceCheckCollisionCir2D(a.data.circle, b.data.circle);
      default:
         return 0;
   }
}

