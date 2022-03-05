#[[
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
]]

cmake_minimum_required(VERSION 3.13)

project(CoffeeChain VERSION 0.2 DESCRIPTION "open 2D engine for creating games - shared library" LANGUAGES C)

option(CoffeeChain_BUILD_SHARED_LIBS     "Build CoffeeChain as shared library" ON)
option(CoffeeChain_BUILD_TESTING         "Build CoffeeChain test program" ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
find_package(glfw3 3.3 REQUIRED)
#find_package(OpenAL    REQUIRED)

add_library(coffeechain
   #src/audio.c
   #src/audio.h
   src/engine_common.c
   include/coffeechain/engine_common.h
   src/engine_common_internal.h
   src/engine_common_glfw.c
   src/engine_common_glfw.h
   src/path_getters.c
   src/path_getters.h
   src/shader.c
   src/shader.h
   src/maps/base_actions.c
   include/coffeechain/map2D/base_actions.h
   src/maps/dynamic_map2D.c
   src/maps/map2D.c
   include/coffeechain/map2D/map2D.h
   src/maps/map2D_internal.h
   src/maps/map2D_file_IO.c
   src/maps/log.c
   src/maps/log.h
   src/external/glad.c
   src/external/glad.h
   src/external/khrplatform.h
   src/external/stb_libs.c
   src/external/stb_image.h
   #src/external/stb_vorbis.h
)

set_target_properties(coffeechain PROPERTIES 
	VERSION ${CoffeeChain_VERSION}
   SOVERSION 2
)



if(CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_BUILD_TYPE MATCHES "DEBUG" OR CMAKE_BUILD_TYPE MATCHES "debug")
   if(NOT MSVC)
      target_compile_options(coffeechain PRIVATE -Wall;-Wextra)
      if(${CMAKE_C_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_C_COMPILER_ID} MATCHES "Clang")
         target_compile_options(coffeechain PRIVATE -Wshadow)
      endif()
   else()
      target_compile_options(coffeechain PRIVATE /W4)
   endif()
endif()


configure_file(src/config.h.in include/coffeechain/config.h)
target_include_directories(coffeechain PUBLIC ${CoffeeChain_BINARY_DIR}/include/coffeechain)
target_link_libraries(coffeechain PRIVATE glfw m)

if(WIN32)
   include(GenerateExportHeader)
   target_link_libraries(coffeechain PRIVATE shell32 kernel32)
   generate_export_header(coffeechain
      EXPORT_MACRO_NAME CCE_EXPORTS
      EXPORT_FILE_NAME ${CoffeeChain_BINARY_DIR}/include/coffeechain/cce_exports.h
      INCLUDE_GUARD_NAME CCE_EXPORTS_H
   )
   install(FILES ${CoffeeChain_BINARY_DIR}/include/coffeechain/cce_exports.h DESTINATION include/coffeechain)
else()
   target_compile_definitions(coffeechain PRIVATE SYSTEM_SHADER_PATH=\"${CMAKE_INSTALL_PREFIX}/share/coffeechain/shaders\")
   # Windows don't have standartized path for storing shared data (or libraries, or anything that isn't self-contained)
endif()

if(CoffeeChain_BUILD_TESTING)
   include(CTest)
   enable_testing()
   add_executable(coffeechain-test1
      test1/main.c
   )
   add_executable(coffeechain-test2
      test2/main.c
      test2/printMap2D.c
   )
   target_include_directories(coffeechain-test1
      PRIVATE include
      PRIVATE ${CoffeeChain_BINARY_DIR}/include
   )
   target_include_directories(coffeechain-test2
      PRIVATE include
      PRIVATE ${CoffeeChain_BINARY_DIR}/include
   )
   target_link_directories(coffeechain-test1 PRIVATE ${CoffeeChain_BINARY_DIR})
   target_link_directories(coffeechain-test2 PRIVATE ${CoffeeChain_BINARY_DIR})
   target_link_libraries(coffeechain-test1 coffeechain)
   target_link_libraries(coffeechain-test2 coffeechain)
   add_test(NAME coffeechain-test1
            COMMAND coffeechain-test1)
   add_test(NAME coffeechain-test2
            COMMAND coffeechain-test2 ${CoffeeChain_SOURCE_DIR})
   
   if(NOT WIN32)
      target_link_options(coffeechain-test1 PRIVATE -Wl,-rpath=${CoffeeChain_BINARY_DIR})
      target_link_options(coffeechain-test2 PRIVATE -Wl,-rpath=${CoffeeChain_BINARY_DIR})
   endif()
endif()

install(TARGETS coffeechain
   LIBRARY DESTINATION lib
)
install(DIRECTORY shaders DESTINATION share/coffeechain)
install(DIRECTORY include/coffeechain DESTINATION include)
install(FILES ${CoffeeChain_BINARY_DIR}/include/coffeechain/config.h DESTINATION include/coffeechain)

