# Once done these will be defined:
#
# REMOTEDISPLAY_FOUND
# REMOTEDISPLAY_INCLUDE_DIRS
# REMOTEDISPLAY_LIBRARIES
#

if(REMOTEDISPLAY_INCLUDE_DIRS AND REMOTEDISPLAY_LIBRARIES)
  set(REMOTEDISPLAY_FOUND TRUE)
else()
  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(_SODIUM QUIET sodium)
  endif()

  find_path(REMOTEDISPLAY_INCLUDE_DIR
    NAMES remotedisplaywidget.h
    HINTS
      ENV RemoteDisplayPath
      ${_REMOTEDISPLAY_INCLUDE_DIRS}
      ${CMAKE_CURRENT_SOURCE_DIR}/ext_libs/RemoteDisplay/src
      /usr/include
      /usr/local/include
      /opt/local/include      
  )

  find_library(REMOTEDISPLAY_LIB
    NAMES RemoteDisplay libRemoteDisplay
    HINTS   ${CMAKE_CURRENT_SOURCE_DIR}/working_dir/lib
            ${_REMOTEDISPLAY_LIBRARY_DIRS} 
            /usr/lib 
            /usr/local/lib 
            /opt/local/lib
  )
  #MESSAGE ("XXXXX: ${CMAKE_CURRENT_SOURCE_DIR}/working_dir/lib")

  set(REMOTEDISPLAY_INCLUDE_DIRS ${REMOTEDISPLAY_INCLUDE_DIR} CACHE PATH "RemoteDisplay include dir")
  set(REMOTEDISPLAY_LIBRARIES ${REMOTEDISPLAY_LIB} CACHE STRING "RemoteDisplay libraries")

  find_package_handle_standard_args(RemoteDisplay DEFAULT_MSG REMOTEDISPLAY_LIB REMOTEDISPLAY_INCLUDE_DIR)
  mark_as_advanced(REMOTEDISPLAY_INCLUDE_DIR REMOTEDISPLAY_LIB)
endif()
