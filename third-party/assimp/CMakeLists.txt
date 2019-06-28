# Open Asset Import Library (assimp)
# ----------------------------------------------------------------------
# Copyright (c) 2006-2017, assimp team
# All rights reserved.
#
# Redistribution and use of this software in source and binary forms,
# with or without modification, are permitted provided that the
# following conditions are met:
#
# * Redistributions of source code must retain the above
#   copyright notice, this list of conditions and the
#   following disclaimer.
#
# * Redistributions in binary form must reproduce the above
#   copyright notice, this list of conditions and the
#   following disclaimer in the documentation and/or other
#   materials provided with the distribution.
#
# * Neither the name of the assimp team, nor the names of its
#   contributors may be used to endorse or promote products
#   derived from this software without specific prior
#   written permission of the assimp team.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#----------------------------------------------------------------------
CMAKE_MINIMUM_REQUIRED( VERSION 3.10 )
PROJECT( Assimp )

# All supported options ###############################################
OPTION( BUILD_SHARED_LIBS
  "Build package with shared libraries."
  ON
)
OPTION( BUILD_FRAMEWORK
  "Build package as Mac OS X Framework bundle."
  OFF
)
OPTION( ASSIMP_DOUBLE_PRECISION
  "Set to ON to enable double precision processing"
  OFF
)
OPTION( ASSIMP_OPT_BUILD_PACKAGES
  "Set to ON to generate CPack configuration files and packaging targets"
  OFF
)
OPTION( ASSIMP_ANDROID_JNIIOSYSTEM
  "Android JNI IOSystem support is active"
  OFF
)
OPTION( ASSIMP_NO_EXPORT
  "Disable Assimp's export functionality."
  OFF
)
OPTION( ASSIMP_BUILD_ZLIB
  "Build your own zlib"
  OFF
)
OPTION( ASSIMP_BUILD_ASSIMP_TOOLS
  "If the supplementary tools for Assimp are built in addition to the library."
  ON
)
OPTION ( ASSIMP_BUILD_SAMPLES
  "If the official samples are built as well (needs Glut)."
  OFF
)
OPTION ( ASSIMP_BUILD_TESTS
  "If the test suite for Assimp is built in addition to the library."
  OFF
)
OPTION ( ASSIMP_COVERALLS
  "Enable this to measure test coverage."
  OFF
)
OPTION ( ASSIMP_WERROR
  "Treat warnings as errors."
  OFF
)
OPTION ( ASSIMP_ASAN
  "Enable AddressSanitizer."
  OFF
)
OPTION ( ASSIMP_UBSAN
  "Enable Undefined Behavior sanitizer."
  OFF
)
OPTION ( SYSTEM_IRRXML
  "Use system installed Irrlicht/IrrXML library."
  OFF
)
OPTION ( BUILD_DOCS
  "Build documentation using Doxygen."
  OFF
)

if (WIN32)
  ADD_DEFINITIONS( -DWIN32_LEAN_AND_MEAN )
endif()

IF(MSVC)
  SET (CMAKE_PREFIX_PATH "D:\\libs\\devil")
  OPTION( ASSIMP_INSTALL_PDB
    "Install MSVC debug files."
    ON
  )
ENDIF(MSVC)

IF (BUILD_FRAMEWORK)
  SET (BUILD_SHARED_LIBS ON)
  MESSAGE(STATUS "Framework bundle building enabled")
ENDIF(BUILD_FRAMEWORK)

IF(NOT BUILD_SHARED_LIBS)
  MESSAGE(STATUS "Shared libraries disabled")
  SET(LINK_SEARCH_START_STATIC TRUE)
ELSE()
  MESSAGE(STATUS "Shared libraries enabled")
ENDIF(NOT BUILD_SHARED_LIBS)

# Define here the needed parameters
SET (ASSIMP_VERSION_MAJOR 4)
SET (ASSIMP_VERSION_MINOR 1)
SET (ASSIMP_VERSION_PATCH 0)
SET (ASSIMP_VERSION ${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}.${ASSIMP_VERSION_PATCH})
SET (ASSIMP_SOVERSION 4)
SET (PROJECT_VERSION "${ASSIMP_VERSION}")

SET( ASSIMP_PACKAGE_VERSION "0" CACHE STRING "the package-specific version used for uploading the sources" )

# Needed for openddl_parser config, no use of c++11 at this moment
ADD_DEFINITIONS( -DOPENDDL_NO_USE_CPP11 )
set_property( GLOBAL PROPERTY CXX_STANDARD 11 )

# Get the current working branch
EXECUTE_PROCESS(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

# Get the latest abbreviated commit hash of the working branch
EXECUTE_PROCESS(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

IF(NOT GIT_COMMIT_HASH)
    SET(GIT_COMMIT_HASH 0)
ENDIF(NOT GIT_COMMIT_HASH)

IF(ASSIMP_DOUBLE_PRECISION)
    ADD_DEFINITIONS(-DASSIMP_DOUBLE_PRECISION)
ENDIF(ASSIMP_DOUBLE_PRECISION)

CONFIGURE_FILE(
  ${CMAKE_CURRENT_LIST_DIR}/revision.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/revision.h
)

CONFIGURE_FILE(
  ${CMAKE_CURRENT_LIST_DIR}/include/assimp/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/assimp/config.h
)

INCLUDE_DIRECTORIES(
    ./
	include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules" )
SET(LIBASSIMP_COMPONENT     "libassimp${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}.${ASSIMP_VERSION_PATCH}" )
SET(LIBASSIMP-DEV_COMPONENT "libassimp${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}.${ASSIMP_VERSION_PATCH}-dev" )
SET(CPACK_COMPONENTS_ALL assimp-bin ${LIBASSIMP_COMPONENT} ${LIBASSIMP-DEV_COMPONENT} assimp-dev)
SET(ASSIMP_LIBRARY_SUFFIX "" CACHE STRING "Suffix to append to library names")

IF( UNIX )
  # Ensure that we do not run into issues like http://www.tcm.phy.cam.ac.uk/sw/inodes64.html on 32 bit linux
  IF( ${OPERATING_SYSTEM} MATCHES "Android")
  ELSE()
    IF ( CMAKE_SIZEOF_VOID_P EQUAL 4) # only necessary for 32-bit linux
      #ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64 )
    ENDIF()
  ENDIF()

  # Use GNUInstallDirs for Unix predefined directories
  INCLUDE(GNUInstallDirs)
ENDIF( UNIX )


# Grouped compiler settings
IF ((CMAKE_C_COMPILER_ID MATCHES "GNU") AND NOT CMAKE_COMPILER_IS_MINGW)
  # hide all not-exported symbols
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fvisibility=hidden -fPIC -Wall -std=c++0x")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  SET(LIBSTDC++_LIBRARIES -lstdc++)
ELSEIF(MSVC)
  # enable multi-core compilation with MSVC
  add_compile_options(/MP)

  # disable "elements of array '' will be default initialized" warning on MSVC2013
  IF(MSVC12)
    add_compile_options(/wd4351)
  ENDIF()
ELSEIF ( "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" )
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fvisibility=hidden -fPIC -Wall -Wno-long-long -std=c++11" )
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
ELSEIF( CMAKE_COMPILER_IS_MINGW )
  SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -Wall -Wno-long-long -std=c++11" )
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  ADD_DEFINITIONS( -U__STRICT_ANSI__ )
ENDIF()

if (ASSIMP_COVERALLS)
    MESSAGE(STATUS "Coveralls enabled")
    INCLUDE(Coveralls)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
endif()

if (ASSIMP_WERROR)
  MESSAGE(STATUS "Treating warnings as errors")
  IF (MSVC)
    add_compile_options(/WX)
  ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
  ENDIF()
endif()

if (ASSIMP_ASAN)
    MESSAGE(STATUS "AddressSanitizer enabled")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
endif()

if (ASSIMP_UBSAN)
    MESSAGE(STATUS "Undefined Behavior sanitizer enabled")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -fno-sanitize-recover=all")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined -fno-sanitize-recover=all")
endif()

#INCLUDE (FindPkgMacros)
#INCLUDE (PrecompiledHeader)

# If this is an in-source build (CMAKE_SOURCE_DIR == CMAKE_BINARY_DIR),
# write the library/executable files to the respective directories in the
# source tree. During an out-of-source build, however, do not litter this
# directory, since that is probably what the user wanted to avoid.
IF ( CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR )
  SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/lib" )
  SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/lib" )
  SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/bin" )
ENDIF ( CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR )

# Cache these to allow the user to override them manually.
SET( ASSIMP_LIB_INSTALL_DIR "lib" CACHE STRING
  "Path the built library files are installed to." )
SET( ASSIMP_INCLUDE_INSTALL_DIR "include" CACHE STRING
  "Path the header files are installed to." )
SET( ASSIMP_BIN_INSTALL_DIR "bin" CACHE STRING
  "Path the tool executables are installed to." )

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
  SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Debug Postfix for lib, samples and tools")
ELSE()
  SET(CMAKE_DEBUG_POSTFIX "" CACHE STRING "Debug Postfix for lib, samples and tools")
ENDIF()

# Only generate this target if no higher-level project already has
#IF (NOT TARGET uninstall)
#  # add make uninstall capability
#  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
#  add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
#ENDIF()

# cmake configuration files
#CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/assimp-config.cmake.in"         "${CMAKE_CURRENT_BINARY_DIR}/assimp-config.cmake" @ONLY IMMEDIATE)
#CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/assimp-config-version.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/assimp-config-version.cmake" @ONLY IMMEDIATE)
#install(FILES "${CMAKE_CURRENT_BINARY_DIR}/assimp-config.cmake"             "${CMAKE_CURRENT_BINARY_DIR}/assimp-config-version.cmake" DESTINATION "${ASSIMP_LIB_INSTALL_DIR}/cmake/assimp-${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}" COMPONENT ${LIBASSIMP-DEV_COMPONENT})
IF( BUILD_DOCS )
    add_subdirectory(doc)
ENDIF( BUILD_DOCS )

# Look for system installed irrXML
IF ( SYSTEM_IRRXML )
    find_package( IrrXML REQUIRED )
ENDIF( SYSTEM_IRRXML )

# Search for external dependencies, and build them from source if not found
# Search for zlib
IF ( NOT ASSIMP_BUILD_ZLIB )
    find_package(ZLIB)
ENDIF( NOT ASSIMP_BUILD_ZLIB )

IF( NOT ZLIB_FOUND )
  message(STATUS "compiling zlib from souces")
  INCLUDE(CheckIncludeFile)
  INCLUDE(CheckTypeSize)
  INCLUDE(CheckFunctionExists)
  # compile from sources
  add_subdirectory(contrib/zlib)
  SET(ZLIB_FOUND 1)
  SET(ZLIB_LIBRARIES zlibstatic)
  SET(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/contrib/zlib ${CMAKE_CURRENT_BINARY_DIR}/contrib/zlib)
else(NOT ZLIB_FOUND)
  ADD_DEFINITIONS(-DASSIMP_BUILD_NO_OWN_ZLIB)
  SET(ZLIB_LIBRARIES_LINKED -lz)
ENDIF(NOT ZLIB_FOUND)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

# Search for unzip
#use_pkgconfig(UNZIP minizip)

IF ( ASSIMP_NO_EXPORT )
  ADD_DEFINITIONS( -DASSIMP_BUILD_NO_EXPORT)
  MESSAGE( STATUS "Build an import-only version of Assimp." )
ENDIF( ASSIMP_NO_EXPORT )

SET ( ASSIMP_BUILD_ARCHITECTURE "" CACHE STRING
  "describe the current architecture."
)
IF    ( ASSIMP_BUILD_ARCHITECTURE STREQUAL "")
ELSE  ( ASSIMP_BUILD_ARCHITECTURE STREQUAL "")
  ADD_DEFINITIONS ( -D'ASSIMP_BUILD_ARCHITECTURE="${ASSIMP_BUILD_ARCHITECTURE}"' )
ENDIF ( ASSIMP_BUILD_ARCHITECTURE STREQUAL "")

# ${CMAKE_GENERATOR}
SET ( ASSIMP_BUILD_COMPILER "" CACHE STRING
  "describe the current compiler."
)
IF    ( ASSIMP_BUILD_COMPILER STREQUAL "")
ELSE  ( ASSIMP_BUILD_COMPILER STREQUAL "")
  ADD_DEFINITIONS ( -D'ASSIMP_BUILD_COMPILER="${ASSIMP_BUILD_COMPILER}"' )
ENDIF ( ASSIMP_BUILD_COMPILER STREQUAL "")

MARK_AS_ADVANCED ( ASSIMP_BUILD_ARCHITECTURE ASSIMP_BUILD_COMPILER )

SET ( ASSIMP_BUILD_NONFREE_C4D_IMPORTER OFF CACHE BOOL
  "Build the C4D importer, which relies on the non-free Melange SDK."
)

IF (ASSIMP_BUILD_NONFREE_C4D_IMPORTER)
  IF ( MSVC )
    SET(C4D_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Melange/includes")

    # pick the correct prebuilt library
    IF(MSVC14)
      SET(C4D_LIB_POSTFIX "_2015")
    ELSEIF(MSVC12)
      SET(C4D_LIB_POSTFIX "_2013")
    ELSEIF(MSVC11)
      SET(C4D_LIB_POSTFIX "_2012")
    ELSEIF(MSVC10)
      SET(C4D_LIB_POSTFIX "_2010")
    ELSE()
      MESSAGE( FATAL_ERROR
        "C4D is currently only supported with MSVC 10, 11, 12, 14"
      )
    ENDIF()

    SET(C4D_LIB_BASE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Melange/libraries/win")

    SET(C4D_DEBUG_LIBRARIES
      "${C4D_LIB_BASE_PATH}/melangelib${C4D_LIB_POSTFIX}/melangelib_debug.lib"
      "${C4D_LIB_BASE_PATH}/jpeglib${C4D_LIB_POSTFIX}/jpeglib_debug.lib"
    )
    SET(C4D_RELEASE_LIBRARIES
      "${C4D_LIB_BASE_PATH}/melangelib${C4D_LIB_POSTFIX}/melangelib_release.lib"
      "${C4D_LIB_BASE_PATH}/jpeglib${C4D_LIB_POSTFIX}/jpeglib_release.lib"
    )

    # winsock and winmm are necessary dependencies of melange (this is undocumented, but true.)
    SET(C4D_EXTRA_LIBRARIES WSock32.lib Winmm.lib)
  ELSE ()
    MESSAGE( FATAL_ERROR
      "C4D is currently only available on Windows with melange SDK installed in contrib/Melange"
    )
  ENDIF ( MSVC )
ELSE (ASSIMP_BUILD_NONFREE_C4D_IMPORTER)
  ADD_DEFINITIONS( -DASSIMP_BUILD_NO_C4D_IMPORTER )
ENDIF (ASSIMP_BUILD_NONFREE_C4D_IMPORTER)

ADD_SUBDIRECTORY(contrib)
ADD_SUBDIRECTORY( code/ )
