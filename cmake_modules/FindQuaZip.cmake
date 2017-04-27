# Once done these will be defined:
#
# QUAZIP_FOUND
# QUAZIP_INCLUDE_DIRS
# QUAZIP_LIBRARIES
#

if(QUAZIP_INCLUDE_DIRS AND QUAZIP_LIBRARIES)
  set(QUAZIP_FOUND TRUE)
else()
  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(_QUAZIP QUIET quazip)
  endif()

  find_path(QUAZIP_INCLUDE_DIR
    NAMES quazip.h
    HINTS
      ENV QuaZipPath
      ${_QUAZIP_INCLUDE_DIRS}
      ${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/quazip/quazip
      /usr/include
      /usr/local/include
      /opt/local/include      
  )

  find_library(QUAZIP_LIB
    NAMES QuaZip quazip5 quazip5d 
    HINTS   ${CMAKE_CURRENT_SOURCE_DIR}/../working_dir/lib
            ${CMAKE_CURRENT_SOURCE_DIR}/../working_dir/build_libs
            ${_QUAZIP_LIBRARY_DIRS} 
            /usr/lib 
            /usr/local/lib 
            /opt/local/lib
  )

  set(QUAZIP_INCLUDE_DIRS ${QUAZIP_INCLUDE_DIR} CACHE PATH "QuaZip include dir")
  set(QUAZIP_LIBRARIES ${QUAZIP_LIB} CACHE STRING "QuaZip libraries")

  find_package_handle_standard_args(QuaZip DEFAULT_MSG QUAZIP_LIB QUAZIP_INCLUDE_DIR)
  mark_as_advanced(QUAZIP_INCLUDE_DIR QUAZIP_LIB)
endif()
