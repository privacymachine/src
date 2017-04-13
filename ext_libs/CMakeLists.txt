cmake_minimum_required (VERSION 2.8.11)

project (PrivacyMachineExtLibs-Project)

# Default to debug build
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE Debug)
endif()
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  # some libs are building with 'd' suffix
  set (PM_DEBUG_D_SUFFIX "d")
endif()


set(BUILD_SHARED_LIBS ON)

# AL: Needed for compiling Fervor without QWebView
# @todo: remove me
set(SKIP_QWEBVIEW "ON") # Valid values: ON/OFF
# We need to support differet QtWebView-Version if we need HTML-Dialogs:
#   https://blog.qt.io/blog/2016/06/16/qt-5-7-released/
#  https://doc.qt.io/qt-5/qtwebenginewidgets-qtwebkitportingguide.html
if (SKIP_QWEBVIEW)
  # add a define useable in cpp source
  add_definitions(-DSKIP_QWEBVIEW)
endif()

# search all Qt-Modules needed for Translation and Installers
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(Qt5Xml REQUIRED)

if(MSVC)
  add_definitions(-D_WIN32_WINNT=0x0601)
  add_definitions(-DWINVER=0x0601)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)

  # used in our source
  add_definitions(-DPM_WINDOWS)

  # enable parallel build
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} /MP")

  # Exception Handling (https://msdn.microsoft.com/en-us/library/1deeycx5.aspx)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
  
  if(NOT DEFINED BUILD_SHARED_LIBS)
    MESSAGE("Build with STATIC Runtime, Build-Type: ${CMAKE_BUILD_TYPE}")
    foreach(flag_var
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_DEBUG_INIT
      CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_DEBUG_INIT
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      if(${flag_var} MATCHES "/MD")
        if ({CMAKE_BUILD_TYPE} MATCHES "DEBUG")
          string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
        else()
          string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif()
      endif()
    endforeach(flag_var)
  else()
    MESSAGE("Building with SHARED Runtime, Build-Type: ${CMAKE_BUILD_TYPE}")
  endif()
else()
  MESSAGE("Build-Type: ${CMAKE_BUILD_TYPE}")
endif()

# Build all executables and libraries to specific directories: very usefull for installation and RPATH-Handling
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../working_dir)
if(MSVC)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../working_dir)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../working_dir/build_libs) #*.lib needed for linking
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../working_dir/lib)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../working_dir/lib)
endif()
# see https://cmake.org/Wiki/CMake_RPATH_handling
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib" /opt/privacymachine/lib)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)


# Needed for QuaZip
set (CMAKE_POSITION_INDEPENDENT_CODE TRUE)

# Disabled by default: set to ON to get quick compile times but a useless PM ;)
set(SKIP_FREERDP_CODE "OFF") # Valid values: ON/OFF
if (SKIP_FREERDP_CODE)
  # add a define useable in cpp source
  add_definitions(-DSKIP_FREERDP_CODE)
endif()

if(MSVC)
  # we use our own zlib on windows, not the qt-version
  set (ZLIB_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/zlib)
  set (ZLIB_LIBRARY zlib)
  include_directories(${ZLIB_ROOT}) # needed for including zlib.h
  include_directories(${CMAKE_CURRENT_BINARY_DIR}/zlib) # needed for including zconf.h
endif()

# Search the Qt-Packages which are needed for translation and the setups
find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5Multimedia)
find_package(Qt5LinguistTools)
find_package(Qt5Test)


# Add CMake submodules to the path
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../cmake_modules ${CMAKE_MODULE_PATH})
include(PmCommon)



MESSAGE("Configuring ext_lib...")

if (NOT SKIP_FREERDP_CODE)
  set (WITH_CLIENT ON)
  set (WITH_SERVER OFF)
  set (CHANNEL_CLIPRDR ON)
  set (CHANNEL_RDPDR OFF)
  set (CHANNEL_SMARTCARD OFF)
  set (CHANNEL_REMDESK OFF)
  set (CHANNEL_RDPSND OFF)
  set (CHANNEL_RDPGFX OFF) 
  set (CHANNEL_DRDYNVC OFF)
  set (CHANNEL_DRIVE OFF)
  set (CHANNEL_PRINTER OFF)
  set (WITH_XTEST ON CACHE BOOL "turned on in all subdirs")
  set (WITH_WAYLAND OFF CACHE BOOL "turned off in all subdirs")
  set (WITH_DIRECTFB OFF)
  if(NOT MSVC)
    # needed for sending keystrokes under linux
    set (WITH_X11 ON) 
    set (WITH_XKBFILE OFF)
  endif()
  set (BUILD_WITH_JPEG ON CACHE BOOL "turned on in all subdirs")
  set (WITH_QTSOUND OFF CACHE BOOL "turned off in all subdirs")
  set (WITH_SSE2 ON)
  #set (WITH_SSE2_TARGET OFF)
  set (WITH_GSTREAMER_1_0 OFF)
  set (WITH_FFMPEG OFF)
  set (WITH_OSS OFF)
  set (WITH_ALSA OFF)
  # enable massive debug log (needed for clipboard-development)
  set (WITH_DEBUG_CLIPRDR ON)
  
  # disabled feature: use libsystemd for systemd journal appender (allows to export wLog to systemd journal)
  set (WITH_LIBSYSTEMD OFF)

  set (WITH_MBEDTLS OFF) # does not work as drop-in-replacement for openssl
  set (WITH_OPENSSL ON)
  
  MESSAGE("Configuring FreeRDP...")
  set (FREERDP_SKIP_WINPR_TOOLS ON)
  set (FREERDP_CUSTOM_INSTALL ON)
  if (MSVC)
    set (CMAKE_INSTALL_LIBDIR .)
    set (WINPR_CMAKE_INSTALL_DIR .)
    set (FREERDP_CLIENT_CMAKE_INSTALL_DIR .)
    set (FREERDP_CMAKE_INSTALL_DIR .)
  else()
    set (CMAKE_INSTALL_LIBDIR privacymachine/)
    set (WINPR_CMAKE_INSTALL_DIR privacymachine/lib)
    set (FREERDP_CLIENT_CMAKE_INSTALL_DIR privacymachine/lib)
    set (FREERDP_CMAKE_INSTALL_DIR privacymachine/)
  endif()
  add_subdirectory(FreeRDP)

  MESSAGE("Configuring RemoteDisplay...")

  # We want that the FindFreeRDP function finds the current source, not the installed binaries 
  if(NOT DEFINED FREERDP_ROOT_DIR)
    set (FREERDP_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/FreeRDP")
  endif()
  if(NOT DEFINED FREERDP_INCLUDE_DIR)
    set (FREERDP_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/FreeRDP/include")
  endif()
  if(NOT DEFINED FREERDP_LIBRARIES)
    set (FREERDP_LIBRARIES freerdp freerdp-client)
  endif()
  set (REMOTEDISPLAY_SKIP_INSTALL_HEADER ON)
  set (REMOTEDISPLAY_SKIP_INSTALL_CLIENT ON)
  set (REMOTEDISPLAY_INSTALL_PREFIX privacymachine/)
  add_subdirectory(RemoteDisplay)
endif()

if (MSVC)
  set (SKIP_INSTALL_ALL ON)
  add_subdirectory(zlib)
endif()

if (MSVC)
  set (LIBRSYNC_INSTALL_DIR .)
else()
  set (LIBRSYNC_INSTALL_DIR privacymachine/lib)
endif()
add_subdirectory(librsync)

set (QUAZIP_SKIP_INSTALL ON)
add_subdirectory(quazip)
