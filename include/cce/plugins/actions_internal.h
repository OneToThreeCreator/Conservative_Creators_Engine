/*
   Conservative Creator's Engine - open source engine for making games.
   Copyright © 2020-2023 Andrey Gaivoronskiy

   This file is part of Conservative Creator's Engine.

   Conservative Creator's Engine is free software: you can redistribute it and/or modify it under 
   the terms of the GNU Lesser General Public License as published by the Free Software Foundation,
   either version 2.1 of the License, or (at your option) any later version.

   Conservative Creator's Engine is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
   PURPOSE. See the GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License along
   with Conservative Creator's Engine. If not, see <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html>.
*/

#include "actions.h"

CCE_API void     ccea__runActions (struct cceaAction *actions, uint32_t totalActionsSize, uint32_t count, struct cce_buffer *state);
// Creates cceaDynamicAction, appends it with data
CCE_API void     ccea__delayDynamicAction (uint32_t ID, uint32_t timeout, void *data, uint32_t dataSize, struct cce_buffer *map);
CCE_API void     ccea__addActionsOnEvent (uint32_t eventUID, uint32_t actionSubsUID, uint32_t totalActionsSize, struct cceaAction *actions, struct cceaActionRunner *wrapper, struct cce_buffer *map);
CCE_API void     ccea__fromHostEndianActionsArray (struct cceaAction *actions, uint32_t totalActionsSize);
CCE_API void     ccea__toHostEndianActionsArray (struct cceaAction *actions, uint32_t totalActionsSize);
CCE_API uint8_t  ccea__doActionsSwapFromHostEndian (void);
