################################################################################
#        Copyright (c) 2013-2016 by the Developers of PrivacyMachine.eu
#                         contact@privacymachine.eu
#     OpenPGP-Fingerprint: 0C93 F15A 0ECA D404 413B 5B34 C6DE E513 0119 B175
#
#                     Licensed under the EUPL, Version 1.1
#     European Commission - subsequent versions of the EUPL (the "Licence");
#        You may not use this work except in compliance with the Licence.
#                  You may obtain a copy of the Licence at:
#                        http://ec.europa.eu/idabc/eupl
#
# Unless required by applicable law or agreed to in writing, software distributed
#              under the Licence is distributed on an "AS IS" basis,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#      See the Licence for the specific language governing permissions and
#                        limitations under the Licence.
################################################################################
cmake_minimum_required(VERSION 2.8.11)

# Default to debug build
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(ProjectName UpdaterTest)
project(${ProjectName})

# add the build dir to the include path
# because moc generates the files there
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# automatic handle qrc files
set(CMAKE_AUTORCC ON)

find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Test REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Multimedia)
find_package(Qt5Xml)

# needed for quazip
#set (QUAZIP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/quazip/quazip)
#include_directories(${QUAZIP_INCLUDE_DIR})
#link_directories(${CMAKE_BINARY_DIR})
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/quazip/quazip")
#link_directories(${CMAKE_BINARY_DIR}/ext_libs/quazip/quazip)
#set (QUAZIP_LIBRARIES ${ZLIB_LIBRARIES} foo quazip5${PM_DEBUG_D_SUFFIX})

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Multimedia_INCLUDE_DIRS})
include_directories(${Qt5Xml_INCLUDE_DIRS})

set (UPDATER_TEST_USES_QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Multimedia Qt5::Xml)

add_definitions(${QT_DEFINITIONS})

add_definitions(-DFV_GUI)

set (NoMocSources
  main.cpp
                    )

set (MocReleatedSources
                    mainwindow.cpp
  ../libpm/CheckUpdate.cpp
  ../libpm/XmlUpdateParser.cpp
  ../libpm/PmVersion.cpp
  ../libpm/VerifiedDownload.cpp
  ../libpm/UpdateManager.cpp
  ../libpm/WidgetInteraktiveUpdate.cpp
                    )

set (MocReleatedHeaders
                    mainwindow.h
  ../libpm/VerifiedDownload.h
  ../libpm/CheckUpdate.h
  ../libpm/XmlUpdateParser.h
  ../libpm/PmVersion.h
  ../libpm/UpdateManager.h
  ../libpm/WidgetInteraktiveUpdate.h
                    )

if(PM_WINDOWS)
  set (NoMocSources ${NoMocSources}
            )
else()
  set (NoMocSources ${NoMocSources}
            )
endif()


set(MocReleatedForms
                mainwindow.ui
                ../libpm/WidgetInteraktiveUpdate.ui
                )

QT5_WRAP_CPP(MocReleatedHeaders_MOC ${MocReleatedHeaders})
QT5_WRAP_UI(MocReleatedForms_MOC ${MocReleatedForms})


add_executable(${ProjectName}
    ${NoMocSources}
    ${MocReleatedSources}
    ${MocReleatedHeaders_MOC}
    ${MocReleatedForms_MOC}
    )

if(MSVC)
  set (MICROSOFT_LIBS wsock32)
endif()

target_link_libraries(${ProjectName}
  ${UPDATER_TEST_USES_QT_LIBRARIES}
  ${QUAZIP_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
  ${MICROSOFT_LIBS}
  )
