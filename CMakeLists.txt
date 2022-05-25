#[[
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
]]

cmake_minimum_required(VERSION 3.0)

project(CoffeeChain VERSION 0.4.3 DESCRIPTION "open 2D engine for creating games - shared library" LANGUAGES C)

if (NOT DEFINED CoffeeChain_BUILD_TESTING)
   option(CoffeeChain_BUILD_TESTING "Build CoffeeChain test program" ON)
endif()

if (NOT DEFINED CoffeeChain_LIB_TYPE)
   set(CoffeeChain_LIB_TYPE SHARED CACHE STRING "Type of library (shared or static) to build CoffeeChain as")
   set_property(CACHE CoffeeChain_LIB_TYPE PROPERTY STRINGS SHARED STATIC)
endif()

if (NOT DEFINED CoffeeChain_INSTALL AND NOT (CoffeeChain_LIB_TYPE MATCHES STATIC))
   option(CoffeeChain_INSTALL "Install CoffeeChain" ON) 
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
find_package(glfw3 3.3 REQUIRED)

find_package(ListLib 1.0)
if (NOT ListLib_FOUND)
   set(ListLib_LIB_TYPE STATIC)
   add_subdirectory(external/listlib)
endif()
#find_package(OpenAL    REQUIRED)

add_library(coffeechain ${CoffeeChain_LIB_TYPE}
   #src/audio.c
   #src/audio.h
   src/shader.c
   src/shader.h
   src/engine_common.c
   src/engine_common_file_IO.c
   src/engine_common.h
   include/coffeechain/engine_common.h
   src/engine_common_internal.h
   src/tools.c
   src/tools.h
   include/coffeechain/tools.h
   src/platform/engine_common_glfw.c
   src/platform/engine_common_glfw.h
   src/platform/path_getters.c
   src/platform/path_getters.h
   include/coffeechain/path_getters.h
   src/platform/platforms.h
   src/platform/endianess.c
   src/platform/endianess.h
   include/coffeechain/endianess.h
   src/maps/base_actions.c
   src/maps/base_actions.h
   include/coffeechain/map2D/base_actions.h
   src/maps/dynamic_map2D.c
   src/maps/map2D.c
   src/maps/map2D.h
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
   SOVERSION ${CoffeeChain_VERSION_MAJOR}
)

if (NOT LibList_FOUND)
   target_include_directories(coffeechain PRIVATE external/listlib/include)
endif()
target_link_libraries(coffeechain PRIVATE list glfw)

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
configure_file(coffeechain.pc.in coffeechain.pc @ONLY)
target_include_directories(coffeechain INTERFACE
                            $<BUILD_INTERFACE:${CoffeeChain_SOURCE_DIR}/include>
                            $<INSTALL_INTERFACE:include>)
target_include_directories(coffeechain PUBLIC
                            $<BUILD_INTERFACE:${CoffeeChain_BINARY_DIR}/include/coffeechain>)

if(WIN32)
   include(GenerateExportHeader)
   generate_export_header(coffeechain
      EXPORT_MACRO_NAME CCE_EXPORTS
      EXPORT_FILE_NAME ${CoffeeChain_BINARY_DIR}/include/coffeechain/cce_exports.h
      INCLUDE_GUARD_NAME CCE_EXPORTS_H
   )
   install(FILES ${CoffeeChain_BINARY_DIR}/include/coffeechain/cce_exports.h DESTINATION include/coffeechain)
   target_link_libraries(coffeechain PRIVATE shell32 kernel32)
   # Windows don't have standartized path for storing shared data (or libraries, or anything that isn't self-contained)
else()
   target_compile_definitions(coffeechain PRIVATE SYSTEM_SHADER_PATH=\"${CMAKE_INSTALL_PREFIX}/share/coffeechain/shaders\")
   target_link_libraries(coffeechain PRIVATE m)
endif()

if(CoffeeChain_BUILD_TESTING)
   include(CTest)
   enable_testing()
   add_executable(coffeechain-test1
      test1/main.c
   )
   add_executable(coffeechain-test2
      test2/main.c
   )
   add_executable(coffeechain-test3
      test3/main.c
      test3/printMap2D.c
      test3/printMap2D.h
   )
   target_include_directories(coffeechain-test1
      PRIVATE include
      PRIVATE ${CoffeeChain_BINARY_DIR}/include
   )
   target_include_directories(coffeechain-test2
      PRIVATE include
      PRIVATE ${CoffeeChain_BINARY_DIR}/include
   )
   target_include_directories(coffeechain-test3
      PRIVATE include
      PRIVATE ${CoffeeChain_BINARY_DIR}/include
   )
   target_link_directories(coffeechain-test1 PRIVATE ${CoffeeChain_BINARY_DIR})
   target_link_directories(coffeechain-test2 PRIVATE ${CoffeeChain_BINARY_DIR})
   target_link_directories(coffeechain-test3 PRIVATE ${CoffeeChain_BINARY_DIR})
   target_link_libraries(coffeechain-test1 coffeechain)
   target_link_libraries(coffeechain-test2 coffeechain)
   target_link_libraries(coffeechain-test3 coffeechain)
   add_test(NAME coffeechain-test1
      COMMAND coffeechain-test1)
   add_test(NAME coffeechain-test2
      COMMAND coffeechain-test2)
   add_test(NAME coffeechain-test3
      COMMAND coffeechain-test3 ${CoffeeChain_SOURCE_DIR})
endif()

if (NOT (CoffeeChain_LIB_TYPE MATCHES STATIC) AND CoffeeChain_INSTALL)
   install(TARGETS coffeechain
      EXPORT  CoffeeChainTargets
      LIBRARY DESTINATION lib
   )
   
   include(CMakePackageConfigHelpers)
   
   configure_package_config_file(${CoffeeChain_SOURCE_DIR}/Config.cmake.in
      "${CoffeeChain_BINARY_DIR}/CoffeeChainConfig.cmake"
      INSTALL_DESTINATION "lib/cmake/CoffeeChain"
      NO_SET_AND_CHECK_MACRO
      NO_CHECK_REQUIRED_COMPONENTS_MACRO
   )
   
   write_basic_package_version_file(
      "${CoffeeChain_BINARY_DIR}/CoffeeChainConfigVersion.cmake"
      VERSION "${CoffeeChain_VERSION_MAJOR}.${CoffeeChain_VERSION_MINOR}"
      COMPATIBILITY AnyNewerVersion
   )
   
   install(DIRECTORY ${CoffeeChain_SOURCE_DIR}/shaders DESTINATION share/coffeechain)
   install(DIRECTORY ${CoffeeChain_SOURCE_DIR}/include/coffeechain DESTINATION include)
   install(FILES ${CoffeeChain_BINARY_DIR}/include/coffeechain/config.h DESTINATION include/coffeechain)
   install(FILES ${CoffeeChain_BINARY_DIR}/coffeechain.pc DESTINATION lib/pkgconfig)
   install(EXPORT CoffeeChainTargets
      FILE CoffeeChainTargets.cmake
      DESTINATION lib/cmake/CoffeeChain
   )
   install(FILES
      ${CoffeeChain_BINARY_DIR}/CoffeeChainConfig.cmake
      ${CoffeeChain_BINARY_DIR}/CoffeeChainConfigVersion.cmake
      DESTINATION lib/cmake/CoffeeChain
   )
endif()
