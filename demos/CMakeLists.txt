#[[
   Conservative Creator's Engine - open source engine for making games.
   Copyright © 2023 Andrey Gaivoronskiy

   This file is part of Conservative Creator's Engine.

   Conservative Creator's Engine is free software: you can redistribute it and/or modify it under 
   the terms of the GNU Lesser General Public License as published by the Free Software Foundation,
   either version 2 of the License, or (at your option) any later version.

   Conservative Creator's Engine is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
   PURPOSE. See the GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License along
   with Conservative Creator's Engine. If not, see <https://www.gnu.org/licenses/>.
]]

cmake_minimum_required(VERSION 3.0)

add_executable(cce-2048
   2048/main.c
)
target_link_libraries(cce-2048 cce)


