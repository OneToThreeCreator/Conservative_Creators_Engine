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

project(coffeechain VERSION 0.1 DESCRIPTION "open 2D engine for creating games - shared library" LANGUAGES C)

option(BUILD_SHARED_LIBS "Build CoffeeChain as shared library" ON)
option(BUILD_TESTING     "Build CoffeeChain test program" ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
find_package(glfw3 3.3 REQUIRED)
#find_package(OpenAL    REQUIRED)

add_library(${PROJECT_NAME}
   #src/audio.c
   src/engine_common.c
   src/engine_common_glfw.c
   src/log.c
   src/path_getters.c
   src/shader.c
   src/maps/base_actions.c
   src/maps/dynamic_map2D.c
   src/maps/map2D.c
   src/maps/map2D_file_IO.c
   src/external/glad.c
   src/external/stb_libs.c
)

set_target_properties(${PROJECT_NAME} PROPERTIES 
   VERSION ${PROJECT_VERSION}
   SOVERSION 0
)

if(CMAKE_BUILD_TYPE MATCHES "DEBUG")
   if(NOT MSVC)
      add_compile_options(-Wall -Wextra)
      if(${CMAKE_C_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_C_COMPILER_ID} MATCHES "Clang")
         add_compile_options(-Wshadow -Wpadded)
      endif()
   else()
      add_compile_options(/W4)
   endif()
else ()
   add_compile_definitions(NDEBUG)
endif()


configure_file(src/config.h.in include/coffeechain/config.h)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR}/include/coffeechain)
target_compile_definitions(${PROJECT_NAME} PRIVATE SYSTEM_SHADER_PATH=\"${CMAKE_INSTALL_PREFIX}/share/coffeechain/shaders\")
target_link_libraries(${PROJECT_NAME} PRIVATE glfw m)

if(WIN32)
   target_link_libraries(${PROJECT_NAME} PRIVATE shell32 kernel32)
   generate_exports_header(${PROJECT_NAME}
      EXPORT_MACRO_NAME CCE_EXPORTS
      EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/coffeechain/cce_exports.h
      INCLUDE_GUARD_NAME CCE_EXPORTS_H
      STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC
   )
   install(FILES ${PROJECT_BINARY_DIR}/include/coffeechain/cce_exports.h DESTINATION include/coffeechain)
endif()

if(BUILD_TESTING)
   include(CTest)
   add_executable(coffeechain-test1
      test/main.c
      test/printMap2D.c
   )
   target_include_directories(coffeechain-test1
      PRIVATE include
      PRIVATE ${PROJECT_BINARY_DIR}/include
   )
   target_link_directories(coffeechain-test1 PRIVATE ${PROJECT_BINARY_DIR})
   target_link_libraries(coffeechain-test1 ${PROJECT_NAME})
   add_test(NAME coffeechain-test1
            COMMAND coffeechain-test1 ${PROJECT_SOURCE_DIR})
   
   if(NOT WIN32)
      target_link_options(coffeechain-test1 PRIVATE -Wl,-rpath=${PROJECT_BINARY_DIR})
   endif()
endif()

install(TARGETS ${PROJECT_NAME}
   LIBRARY DESTINATION lib
)
install(DIRECTORY shaders DESTINATION share/coffeechain)
install(DIRECTORY include/coffeechain DESTINATION include)
install(FILES ${PROJECT_BINARY_DIR}/include/coffeechain/config.h DESTINATION include/coffeechain)

