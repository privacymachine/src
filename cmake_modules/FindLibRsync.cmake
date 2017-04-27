# Once done these will be defined:
#
# LIBRSYNC_FOUND
# LIBRSYNC_INCLUDE_DIRS
# LIBRSYNC_LIBRARIES
#

if(LIBRSYNC_INCLUDE_DIRS AND LIBRSYNC_LIBRARIES)
  set(LIBRSYNC_FOUND TRUE)
else()
  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(_LIBRSYNC QUIET librsync)
  endif()

  find_path(LIBRSYNC_INCLUDE_DIR
    NAMES librsync.h
    HINTS
      ENV LibRsyncPath
      ${_LIBRSYNC_INCLUDE_DIRS}
      ${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/librsync/src
      /usr/include
      /usr/local/include
      /opt/local/include      
  )

  find_path(LIBRSYNC_BUILD_DIR
    NAMES librsync-config.h
    HINTS
      ENV LibRsyncBuildPath
      ${CMAKE_CURRENT_SOURCE_DIR}/../build_ext_libs/librsync/src
      ${CMAKE_CURRENT_SOURCE_DIR}/../build-ext_libs-Desktop-Default/librsync/src
  )

  find_library(LIBRSYNC_LIB
    NAMES librsync liblibrsync
    HINTS   ${CMAKE_CURRENT_SOURCE_DIR}/../working_dir/lib
            ${CMAKE_CURRENT_SOURCE_DIR}/../working_dir/build_libs
            ${_LIBRSYNC_LIBRARY_DIRS} 
            /usr/lib 
            /usr/local/lib 
            /opt/local/lib
              )

  set(LIBRSYNC_INCLUDE_DIRS ${LIBRSYNC_INCLUDE_DIR} ${LIBRSYNC_BUILD_DIR} CACHE PATH "LibRsync include dir")
  set(LIBRSYNC_LIBRARIES ${LIBRSYNC_LIB} CACHE STRING "LibRsync libraries")

  find_package_handle_standard_args(LibRsync DEFAULT_MSG LIBRSYNC_LIB LIBRSYNC_INCLUDE_DIR LIBRSYNC_BUILD_DIR)
  mark_as_advanced(LIBRSYNC_INCLUDE_DIR LIBRSYNC_LIB LIBRSYNC_BUILD_DIR)
endif()
