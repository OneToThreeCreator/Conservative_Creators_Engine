#[[
   Conservative Creator's Engine - open source engine for making games.
   Copyright © 2020-2023 Andrey Gaivoronskiy

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

project(CCE VERSION 0.6.2 DESCRIPTION "open 2D engine for creating games" LANGUAGES C)

if (NOT DEFINED CCE_BUILD_TESTING)
   option(CCE_BUILD_TESTING "Build Conservative Creator's Engine test program" ON)
endif()

if (NOT DEFINED CCE_LIB_TYPE)
   set(CCE_LIB_TYPE SHARED CACHE STRING "Type of library (shared or static) to build Conservative Creator's Engine as")
   set_property(CACHE CCE_LIB_TYPE PROPERTY STRINGS SHARED STATIC)
endif()

if (NOT DEFINED CCE_INSTALL AND NOT ("${CCE_LIB_TYPE}" MATCHES STATIC))
   option(CCE_INSTALL "Install engine" ON) 
endif()

if ("${CCE_LIB_TYPE}" MATCHES STATIC)
   set(CCE_INSTALL OFF) 
endif()

if (NOT DEFINED CCE_BUILD_DEMOS)
   option(CCE_BUILD_DEMOS "Build Conservative Creator's Engine demo programs" ON)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
#find_package(OpenAL    REQUIRED)

add_library(cce ${CCE_LIB_TYPE}
   #src/audio.c
   #src/audio.h
   src/shader.c
   src/shader.h
   src/engine_common.c
   src/engine_common_file_IO.c
   include/cce/engine_common.h
   include/cce/engine_common_internal.h
   src/utils.c
   include/cce/utils.h
   src/platform/engine_common_glfw.c
   src/platform/engine_common_keyboard.c
   include/cce/engine_common_keyboard.h
   src/platform/os_interaction.c
   include/cce/os_interaction.h
   src/platform/platforms.h
   src/platform/endianess.c
   include/cce/endianess.h
   src/plugins/actions.c
   include/cce/plugins/actions.h
   include/cce/plugins/actions_internal.h
   include/cce/plugins/actions_runactions.h
   src/plugins/map2D/map2D_modification.c
   src/plugins/map2D/map2D.c
   src/plugins/map2D/map2D_openGL.c
   src/plugins/map2D/map2D_collision.c
   include/cce/plugins/map2D/map2D.h
   src/plugins/map2D/map2D_internal.h
   src/plugins/map2D/map2D_file_IO.c
   #src/plugins/text_rendering.c
   #src/plugins/open_world.c
   src/external/stb_libs.c
   src/external/stb_image.h
   #src/external/stb_vorbis.h
)

find_package(glfw3 3.3)
if (NOT glfw3_FOUND)
   set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
   set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
   set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
   add_subdirectory(external/glfw)
   target_include_directories(cce PRIVATE external/glfw/include/)
endif()

set_target_properties(cce PROPERTIES 
   VERSION ${CCE_VERSION}
   SOVERSION ${CCE_VERSION_MAJOR}
)

find_package(PkgConfig)
if (PkgConfig_FOUND)
   pkg_check_modules(INIH inih)
else()
   set(INIH_FOUND NO)
endif()

if (NOT INIH_FOUND)
   add_library(inih STATIC
      external/inih/ini.c
      external/inih/ini.h
   )
   target_compile_definitions(inih PRIVATE 
      INI_ALLOW_MULTILINE=1     INI_ALLOW_BOM=1       INI_ALLOW_INLINE_COMMENTS=1       INI_ALLOW_NO_VALUE=0 
      INI_STOP_ON_FIRST_ERROR=1 INI_HANDLER_LINENO=0  INI_CALL_HANDLER_ON_NEW_SECTION=0 INI_USE_STACK=1 
   )
   set(INIH_LIBRARIES inih)
   target_include_directories(cce PRIVATE external/inih)
   target_compile_definitions(cce PRIVATE INIH_LOCAL)
endif()

find_package(ListLib 1.0 QUIET)
if (NOT ListLib_FOUND)
   set(ListLib_LIB_TYPE STATIC)
   add_subdirectory(external/listlib)
   target_include_directories(cce PRIVATE external/listlib/include)
endif()

add_subdirectory(external/glad2/cmake)
glad_add_library(glad STATIC API gl:core=3.2 EXTENSIONS GL_NV_copy_image)
set_property(TARGET glad PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(cce PRIVATE list ${INIH_LIBRARIES} glfw glad)

set(CCE_BUILD_TYPE ${CMAKE_BUILD_TYPE})
string(TOLOWER "${CCE_BUILD_TYPE}" CCE_BUILD_TYPE)

if("${CCE_BUILD_TYPE}" MATCHES "debug")
   if(NOT MSVC)
      target_compile_options(cce PRIVATE -Wall)
      if(${CMAKE_C_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_C_COMPILER_ID} MATCHES "Clang")
         target_compile_options(cce PRIVATE -Wextra;-Wshadow;-Woverflow;-pedantic;-Wstrict-aliasing;-Wstrict-overflow;-Wstringop-overflow;-Wpointer-arith)
      endif()
   else()
      target_compile_options(cce PRIVATE /W3)
   endif()
endif()

if("${CCE_BUILD_TYPE}" MATCHES "release" OR "${CCE_BUILD_TYPE}" MATCHES "relwithdebinfo")
   set_property(TARGET cce PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

configure_file(src/config.h.in include/cce/config.h)
configure_file(cce.pc.in cce.pc @ONLY)
include(GenerateExportHeader)
generate_export_header(cce
   EXPORT_MACRO_NAME CCE_API
   DEPRECATED_MACRO_NAME CCE_DEPRECATED
   EXPORT_FILE_NAME ${CCE_BINARY_DIR}/include/cce/cce_exports.h
   INCLUDE_GUARD_NAME CCE_EXPORTS_H
)
set_property(TARGET cce PROPERTY C_VISIBILITY_PRESET hidden)
target_include_directories(cce PUBLIC
                            $<BUILD_INTERFACE:${CCE_SOURCE_DIR}/include>
                            $<INSTALL_INTERFACE:include>)
target_include_directories(cce PUBLIC
                            $<BUILD_INTERFACE:${CCE_BINARY_DIR}/include>)
target_include_directories(cce PUBLIC
                            $<BUILD_INTERFACE:${CCE_BINARY_DIR}/include/cce>)

if(WIN32)
   target_link_libraries(cce PRIVATE shell32 kernel32)
   # Windows don't have standartized path for storing shared data (or libraries, or anything that isn't self-contained)
else()
   target_compile_definitions(cce PRIVATE SYSTEM_RESOURCE_PATH=\"${CMAKE_INSTALL_PREFIX}/share/cce/\")
   target_link_libraries(cce PRIVATE m)
endif()

if(CCE_BUILD_TESTING)
   include(CTest)
   enable_testing()
   add_executable(cce-test1
      test1/main.c
      test1/platformTest.c
      test1/utilsTest.c
   )
   add_executable(cce-test2
      test2/main.c
   )
   target_link_libraries(cce-test1 cce)
   target_link_libraries(cce-test2 cce)
   add_test(NAME cce-test1
      COMMAND cce-test1)
   add_test(NAME cce-test2
      COMMAND cce-test2 "${CCE_SOURCE_DIR}")
endif()

if (CCE_BUILD_DEMOS)
   add_subdirectory(demos)
endif()

if (NOT (CCE_LIB_TYPE MATCHES STATIC) AND CCE_INSTALL)
   install(TARGETS cce
      EXPORT  CCETargets
      LIBRARY DESTINATION lib
   )
   
   include(CMakePackageConfigHelpers)
   
   configure_package_config_file(${CCE_SOURCE_DIR}/Config.cmake.in
      "${CCE_BINARY_DIR}/CCEConfig.cmake"
      INSTALL_DESTINATION "lib/cmake/CCE"
      NO_SET_AND_CHECK_MACRO
      NO_CHECK_REQUIRED_COMPONENTS_MACRO
   )
   
   write_basic_package_version_file(
      "${CCE_BINARY_DIR}/CCEConfigVersion.cmake"
      VERSION "${CCE_VERSION_MAJOR}.${CCE_VERSION_MINOR}"
      COMPATIBILITY AnyNewerVersion
   )
   
   install(DIRECTORY ${CCE_SOURCE_DIR}/shaders DESTINATION share/cce)
   install(DIRECTORY ${CCE_SOURCE_DIR}/include/cce DESTINATION include)
   install(FILES ${CCE_BINARY_DIR}/include/cce/config.h DESTINATION include/cce)
   install(FILES ${CCE_BINARY_DIR}/cce.pc DESTINATION lib/pkgconfig)
   if (WIN32)
      install(FILES ${CCE_BINARY_DIR}/include/cce/cce_exports.h DESTINATION include/cce)
   endif()
   install(EXPORT CCETargets
      FILE CCETargets.cmake
      DESTINATION lib/cmake/CCE
   )
   install(FILES
      ${CCE_BINARY_DIR}/CCEConfig.cmake
      ${CCE_BINARY_DIR}/CCEConfigVersion.cmake
      DESTINATION lib/cmake/CCE
   )
endif()
