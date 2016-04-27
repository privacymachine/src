cmake_minimum_required (VERSION 2.8)

project (PrivacyMachine)

# Default to release build
if ("${CMAKE_BUILD_TYPE}" MATCHES "")
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(BUILD_SHARED_LIBS ON)


if(MSVC)
  add_definitions(-D_WIN32_WINNT=0x0601)
  add_definitions(-DWINVER=0x0601)
  add_definitions(-DWIN32_LEAN_AND_MEAN)

  add_definitions(-D_CRT_SECURE_NO_WARNINGS)

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

set(SKIP_FREERDP_CODE "OFF")

if (NOT SKIP_FREERDP_CODE)
  # Build all external libraries (FreeRDP,...)
  add_subdirectory(ext_libs)
else()
  # add a define useable in cpp source
  add_definitions(-DSKIP_FREERDP_CODE)
endif()

# Disabled by default: Copy the firefox profile to the VM at each VM-Mask start
# Valid values: ON/OFF
set(USE_OUTSIDE_FIREFOX_PROFILE "OFF")

if (USE_OUTSIDE_FIREFOX_PROFILE)
  add_definitions(-DUSE_OUTSIDE_FIREFOX_PROFILE)
endif()


# Add the source of the PrivacyMachine
add_subdirectory(pm)

