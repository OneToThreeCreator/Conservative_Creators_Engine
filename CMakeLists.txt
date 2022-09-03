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

project(CoffeeChain VERSION 0.6.0 DESCRIPTION "open 2D engine for creating games - shared library" LANGUAGES C)

if (NOT DEFINED CoffeeChain_BUILD_TESTING)
   option(CoffeeChain_BUILD_TESTING "Build CoffeeChain test program" ON)
endif()

if (NOT DEFINED CoffeeChain_LIB_TYPE)
   set(CoffeeChain_LIB_TYPE SHARED CACHE STRING "Type of library (shared or static) to build CoffeeChain as")
   set_property(CACHE CoffeeChain_LIB_TYPE PROPERTY STRINGS SHARED STATIC)
endif()

if (NOT DEFINED CoffeeChain_INSTALL AND NOT ("${CoffeeChain_LIB_TYPE}" MATCHES STATIC))
   option(CoffeeChain_INSTALL "Install CoffeeChain" ON) 
endif()

if ("${CoffeeChain_LIB_TYPE}" MATCHES STATIC)
   set(CoffeeChain_INSTALL OFF) 
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
find_package(glfw3 3.3 REQUIRED)
#find_package(OpenAL    REQUIRED)

add_library(coffeechain ${CoffeeChain_LIB_TYPE}
   #src/audio.c
   #src/audio.h
   src/shader.c
   src/shader.h
   src/engine_common.c
   src/engine_common_file_IO.c
   include/coffeechain/engine_common.h
   src/engine_common_internal.h
   src/utils.c
   include/coffeechain/utils.h
   src/platform/engine_common_glfw.c
   src/platform/engine_common_glfw.h
   src/platform/os_interaction.c
   include/coffeechain/os_interaction.h
   src/platform/platforms.h
   src/platform/endianess.c
   include/coffeechain/endianess.h
   src/map2D/base_actions.c
   include/coffeechain/map2D/base_actions.h
   src/map2D/dynamic_map2D.c
   src/map2D/map2D.c
   include/coffeechain/map2D/map2D.h
   src/map2D/map2D_internal.h
   src/map2D/map2D_file_IO.c
   src/map2D/log.c
   src/map2D/log.h
   src/plugins/text_rendering.c
   src/external/stb_libs.c
   src/external/stb_image.h
   #src/external/stb_vorbis.h
)

set_target_properties(coffeechain PROPERTIES 
   VERSION ${CoffeeChain_VERSION}
   SOVERSION ${CoffeeChain_VERSION_MAJOR}
)

find_package(PkgConfig)
if (PkgConfig_FOUND)
   pkg_check_modules(INIH inih)
else()
   set(INIH_FOUND NO)
endif()

if (NOT INIH_FOUND)
   if ("${CMAKE_BUILD_TYPE}" MATCHES "")
      set(INIH_BUILD_TYPE plain)
   else()
      set(INIH_BUILD_TYPE CMAKE_BUILD_TYPE)
      string(TOLOWER ${INIH_BUILD_TYPE} INIH_BUILD_TYPE)
   endif()
   include(ExternalProject)
   ExternalProject_Add(inih_external
      PREFIX "${CoffeeChain_BINARY_DIR}/external/inih"
      SOURCE_DIR "${CoffeeChain_SOURCE_DIR}/external/inih" 
      BINARY_DIR "${CoffeeChain_BINARY_DIR}/external/inih"
      CONFIGURE_COMMAND meson setup -Ddefault_library=static -Ddistro_install=false -Dwith_INIReader=false -DINI_STOP_ON_FIRST_ERROR=1 --buildtype ${INIH_BUILD_TYPE} ${CoffeeChain_SOURCE_DIR}/external/inih
      BUILD_COMMAND meson compile
      INSTALL_COMMAND "" 
   )
   add_library(inih STATIC IMPORTED)
   add_dependencies(inih inih_external)
   set_target_properties(inih PROPERTIES IMPORTED_LOCATION ${CoffeeChain_BINARY_DIR}/external/inih/libinih.a)
   set(INIH_LIBRARIES inih)
   target_include_directories(coffeechain PRIVATE external/inih)
   target_compile_definitions(coffeechain PRIVATE INIH_LOCAL)
endif()

find_package(ListLib 1.0 QUIET)
if (NOT ListLib_FOUND)
   set(ListLib_LIB_TYPE STATIC)
   add_subdirectory(external/listlib)
   target_include_directories(coffeechain PRIVATE external/listlib/include)
endif()

add_subdirectory(external/glad2/cmake)
glad_add_library(glad STATIC API gl:core=3.2 EXTENSIONS NONE)

target_link_libraries(coffeechain PRIVATE list ${INIH_LIBRARIES} glfw glad)

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
target_include_directories(coffeechain PUBLIC
                            $<BUILD_INTERFACE:${CoffeeChain_SOURCE_DIR}/include>
                            $<INSTALL_INTERFACE:include>)
target_include_directories(coffeechain PUBLIC
                            $<BUILD_INTERFACE:${CoffeeChain_BINARY_DIR}/include>)
target_include_directories(coffeechain PUBLIC
                            $<BUILD_INTERFACE:${CoffeeChain_BINARY_DIR}/include/coffeechain>)

if(WIN32)
   include(GenerateExportHeader)
   generate_export_header(coffeechain
      EXPORT_MACRO_NAME CCE_EXPORTS
      EXPORT_FILE_NAME ${CoffeeChain_BINARY_DIR}/include/coffeechain/cce_exports.h
      INCLUDE_GUARD_NAME CCE_EXPORTS_H
   )
   target_link_libraries(coffeechain PRIVATE shell32 kernel32)
   # Windows don't have standartized path for storing shared data (or libraries, or anything that isn't self-contained)
else()
   target_compile_definitions(coffeechain PRIVATE SYSTEM_RESOURCE_PATH=\"${CMAKE_INSTALL_PREFIX}/share/coffeechain/\")
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
   )
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
   if (WIN32)
      install(FILES ${CoffeeChain_BINARY_DIR}/include/coffeechain/cce_exports.h DESTINATION include/coffeechain)
   endif()
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
