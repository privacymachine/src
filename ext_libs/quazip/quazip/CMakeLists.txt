# set all include directories for in and out of source builds
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${ZLIB_INCLUDE_DIRS}
)

#file(GLOB SRCS "*.c" "*.cpp")
#file(GLOB PUBLIC_HEADERS "*.h")

set (MocReleatedSources
      qioapi.cpp
      quagzipfile.cpp
      quaziodevice.cpp
      quazipfile.cpp
    )

set (MocReleatedHeaders
      quagzipfile.h
      quaziodevice.h
      quazipfile.h
    )

set (NoMocSources
      unzip.c
      zip.c
      JlCompress.cpp
      quaadler32.cpp
      quacrc32.cpp
      quazip.cpp
      quazipdir.cpp
      quazipfileinfo.cpp
      quazipnewinfo.cpp
    )


set (NoMocReleatedHeaders
      crypt.h
      ioapi.h
      unzip.h
      zip.h
      quazip_global.h
      quaadler32.h
      quacrc32.h
      quachecksum32.h
      quazip.h
      quazipfileinfo.h
      quazipnewinfo.h
      JlCompress.h
      quazipdir.h
    )


# Must be added to enable export macro
ADD_DEFINITIONS(-DQUAZIP_BUILD)

qt5_wrap_cpp(MocReleatedHeaders_MOC ${MocReleatedHeaders})


add_library(${QUAZIP_LIB_TARGET_NAME} SHARED
            ${NoMocSources}
            ${MocReleatedSources}
            ${MocReleatedHeaders_MOC}
            )

add_library(quazip_static STATIC
  ${NoMocSources}
  ${MocReleatedSources}
  ${MocReleatedHeaders_MOC}
  )

# Windows uses .lib extension for both static and shared library
# *nix systems use different extensions for SHARED and STATIC library and by convention both libraries have the same name
if (NOT WIN32)
	set_target_properties(quazip_static PROPERTIES OUTPUT_NAME quazip${QUAZIP_LIB_VERSION_SUFFIX})
endif ()

set_target_properties(${QUAZIP_LIB_TARGET_NAME} quazip_static PROPERTIES VERSION 1.0.0 SOVERSION 1 DEBUG_POSTFIX d)
# Link against ZLIB_LIBRARIES if needed (on Windows this variable is empty)
target_link_libraries(${QUAZIP_LIB_TARGET_NAME} quazip_static ${QT_QTMAIN_LIBRARY} ${QTCORE_LIBRARIES} ${ZLIB_LIBRARIES})

if (NOT QUAZIP_SKIP_INSTALL)
  install(FILES ${PUBLIC_HEADERS} DESTINATION include/quazip${QUAZIP_LIB_VERSION_SUFFIX})
  install(TARGETS ${QUAZIP_LIB_TARGET_NAME} quazip_static LIBRARY DESTINATION ${LIB_DESTINATION} ARCHIVE DESTINATION ${LIB_DESTINATION} RUNTIME DESTINATION ${LIB_DESTINATION})
endif()
