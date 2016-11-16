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
include(CPackComponent)

project (ProblemReporter)

# Add CMake submodules to the path
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules ${CMAKE_MODULE_PATH})

if(MSVC)
  set (PM_WINDOWS TRUE)
  add_definitions(-DPM_WINDOWS)
endif()


# add the build dir to the include path
# because moc generates the files there
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# automatic handle qrc files
set(CMAKE_AUTORCC ON)

find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5Multimedia)
find_package(Qt5LinguistTools)
find_package(Qt5Test)

# needed for quazip
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/quazip/quazip)
link_directories(${CMAKE_BINARY_DIR}/../working_dir)
link_directories(${CMAKE_BINARY_DIR}/../working_dir/lib)
set (QUAZIP_LIBRARIES quazip5${PM_DEBUG_D_SUFFIX})

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Multimedia_INCLUDE_DIRS})

set (PROBLEMREPORTER_USES_QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Multimedia)

add_definitions(${QT_DEFINITIONS})

SET (NoMocSources
                    main.cpp
                    )

if(PM_WINDOWS)
  set (NoMocSources ${NoMocSources} resources.rc)
  set (MICROSOFT_LIBS wsock32)
endif()

set (MocReleatedSources
                    frmProblemReporter.cpp
                    )

set (MocReleatedHeaders
                    frmProblemReporter.h
                    )

set(MocReleatedForms
                frmProblemReporter.ui
                )

set(MocReleatedResources images.qrc)

QT5_WRAP_CPP(MocReleatedHeaders_MOC ${MocReleatedHeaders})
QT5_WRAP_UI(MocReleatedForms_MOC ${MocReleatedForms})

add_executable(ProblemReporter
    ${NoMocSources}
    ${MocReleatedSources}
    ${MocReleatedHeaders_MOC}
    ${MocReleatedForms_MOC}
    ${MocReleatedResources_RCC})


target_link_libraries(ProblemReporter
                      pm
                      ${Qt5Core_LIBRARIES}
                      ${Qt5Widgets_LIBRARIES}
                      ${Qt5Test_LIBRARIES}
                      ${QUAZIP_LIBRARIES}
                      ${MICROSOFT_LIBS}
                      )

set_target_properties(ProblemReporter
                      PROPERTIES
                      SKIP_BUILD_RPATH FALSE
                      BUILD_WITH_INSTALL_RPATH FALSE
                      )

