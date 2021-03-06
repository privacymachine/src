project (QuaZip)
cmake_minimum_required(VERSION 2.6)

# Add CMake submodules to the path
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules ${CMAKE_MODULE_PATH})
include(PmCommon)

option(BUILD_WITH_QT4 "Build QuaZip with Qt4 no matter if Qt5 was found" OFF)

if( NOT BUILD_WITH_QT4 )
    # try Qt5 first, and prefer that if found
    find_package(Qt5Core QUIET)
endif()

if (Qt5Core_FOUND)
    set(QTCORE_LIBRARIES ${Qt5Core_LIBRARIES})
    set(QUAZIP_LIB_VERSION_SUFFIX 5)
    # if there is no QT_ROOT, try to deduce it from Qt QtCore include
    if ("${QT_ROOT}" STREQUAL "")
        set(QT_ROOT ${QT_QTCORE_INCLUDE_DIR}/../..)
    endif()
    include_directories(${Qt5Core_INCLUDE_DIRS})

    macro(qt_wrap_cpp)
        qt5_wrap_cpp(${ARGN})
    endmacro()
else()
    set(qt_min_version "4.5.0")
    find_package(Qt4 REQUIRED)
    set(QT_USE_QTGUI false)
    include(${QT_USE_FILE})
    include_directories(${QT_INCLUDES})
    set(QTCORE_LIBRARIES ${QT_QTCORE_LIBRARY})

    macro(qt_wrap_cpp)
        qt4_wrap_cpp(${ARGN})
    endmacro()
endif()

find_package(ZLIB REQUIRED)

# All build libraries are moved to this directory
#SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)")
set(LIB_DESTINATION "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE STRING "Library directory name" FORCE)
set(QUAZIP_LIB_TARGET_NAME quazip${QUAZIP_LIB_VERSION_SUFFIX} CACHE
    INTERNAL "Target name of libquazip" FORCE)

add_subdirectory(quazip)

if (NOT QUAZIP_SKIP_INSTALL)
  install(FILES FindQuaZip.cmake RENAME FindQuaZip${QUAZIP_LIB_VERSION_SUFFIX}.cmake DESTINATION ${CMAKE_ROOT}/Modules)
endif()
