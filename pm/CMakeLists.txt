################################################################################
#        Copyright (c) 2013-2016 by the Developers of PrivacyMachine.eu
#                         contact@privacymachine.eu
#     OpenPGP-Fingerprint: 5134 F10C 58ED FCA2 7F25 7483 50F4 40FC 4347 D242
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

set(ProjectName PrivacyMachine)
project(${ProjectName})

# Add CMake submodules to the path
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules ${CMAKE_MODULE_PATH})

if(MSVC)
  set (PM_WINDOWS TRUE)
  add_definitions(-DPM_WINDOWS)
  #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
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

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Multimedia_INCLUDE_DIRS})

set (PM_USES_QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Multimedia)

add_definitions(${QT_DEFINITIONS})

set (NoMocSources
                    getMemorySize.cpp
                    getMemorySize.h
                    main.cpp
                    utils.h
                    utils.cpp
                    resource.h
                    PMCommand.h
                    PMCommand.cpp
                    PMInstance.h
                    PMInstance.cpp
                    PMInstanceConfiguration.h
                    PMInstanceConfiguration.cpp
                    PMManager.h
                    PMManager.cpp
                    UserConfig.h
                    UserConfig.cpp
                    SystemConfig.h
                    SystemConfig.cpp
                    PMManager.h
                    PMManager.cpp
                    UserConfig.h
                    UserConfig.cpp
                    )

set (MocReleatedSources
                    WindowMain.cpp
                    WidgetNewTab.cpp
                    WidgetUpdate.cpp
                    WidgetCommandExec.cpp
                    WidgetRdpView.cpp
                    )

set (MocReleatedHeaders
                    WindowMain.h
                    WidgetNewTab.h
                    WidgetUpdate.h
                    WidgetCommandExec.h
                    WidgetRdpView.h
                    )

if(PM_WINDOWS)
  set (NoMocSources ${NoMocSources}
            resources.rc
            )
else()
  set (NoMocSources ${NoMocSources}
            )
endif()


set(MocReleatedForms
                WindowMain.ui
                WidgetNewTab.ui
                WidgetUpdate.ui
                WidgetCommandExec.ui
                )

QT5_WRAP_CPP(MocReleatedHeaders_MOC ${MocReleatedHeaders})
QT5_WRAP_UI(MocReleatedForms_MOC ${MocReleatedForms})

# for Translation read <http://doc.qt.io/qt-5/internationalization.html>
#  <http://www.cmake.org/Wiki/CMake:How_To_Build_Qt4_Software>
#  <http://doc.qt.io/qt-5/cmake-manual.html#qt5linguisttools-macros>

# Define all Files parsed for Translations
file (GLOB HEADER_FILES *.h *.hpp)
file (GLOB CPP_FILES *.cpp)
file (GLOB UI_FILES *.ui)
set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${CPP_FILES} ${UI_FILES} ${HEADER_FILES})
set (TRANSLATIONS_FILES
     ${CMAKE_CURRENT_SOURCE_DIR}/translations/lang_de.ts
)

OPTION(REFRESH_TRANSLATIONS "Refresh translations on compile" OFF)

# HOWTO Updating new Translations
# o) To get new text to translateable items in the  pm/translations/lang_de.ts file comment in the line below
# o) Compile the Project ONE TIME and imediately comment it out again, because a 'clean' command will delete the updated lang_de.ts file
# o) Start 'linguist' and open the file pm/translations/lang_de.ts
# o) Do all translations and save the file
# o) On the next build the translations are located in the file build/pm/lang_de.qm
#set (REFRESH_TRANSLATIONS ON)

if (REFRESH_TRANSLATIONS)
  qt5_create_translation(QM_FILES
                          ${FILES_TO_TRANSLATE}
                          ${TRANSLATIONS_FILES})
else (REFRESH_TRANSLATIONS)
  qt5_add_translation(QM_FILES ${TRANSLATIONS_FILES})
endif (REFRESH_TRANSLATIONS)

add_custom_target (translations_target DEPENDS ${QM_FILES})

add_executable(${ProjectName}
    ${NoMocSources}
    ${MocReleatedSources}
    ${MocReleatedHeaders_MOC}
    ${MocReleatedForms_MOC}
    images.qrc
    )

# mark non compilable
#set_source_files_properties(build/PrivacyMachine.ini PROPERTIES HEADER_FILE_ONLY TRUE)

if (NOT SKIP_FREERDP_CODE)
  if(WIN32)
    set (AdditionalFreeRDPLibraries ${LIBFREERDPCLIENT_LIBRARIES} LibRemoteDisplay Mpr.lib)
  else()
    set (AdditionalFreeRDPLibraries ${LIBFREERDPCLIENT_LIBRARIES} LibRemoteDisplay)
  endif()
  include_directories(${LibRemoteDisplay_SOURCE_DIR})
else()
  set (AdditionalFreeRDPLibraries "")
endif()

target_link_libraries(${ProjectName} ${Qt5Core_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${AdditionalFreeRDPLibraries})

#copy the needed files to the build dir
if(MSVC)
  add_custom_command(TARGET ${ProjectName} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_BINARY_DIR}/../ext_libs/FreeRDP/freerdp.dll"
      ${CMAKE_CURRENT_BINARY_DIR}

      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_BINARY_DIR}/../ext_libs/FreeRDP/freerdp-client.dll"
      ${CMAKE_CURRENT_BINARY_DIR}

      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_BINARY_DIR}/../ext_libs/FreeRDP/winpr.dll"
      ${CMAKE_CURRENT_BINARY_DIR}

      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_BINARY_DIR}/../ext_libs/RemoteDisplay/src/LibRemoteDisplay.dll"
      ${CMAKE_CURRENT_BINARY_DIR}

      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/OpenSSL/openssl-1.0.2d-64bit-${CMAKE_BUILD_TYPE}-DLL-VS2013/bin/libeay32.dll"
      ${CMAKE_CURRENT_BINARY_DIR}

      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/OpenSSL/openssl-1.0.2d-64bit-${CMAKE_BUILD_TYPE}-DLL-VS2013/bin/ssleay32.dll"
      ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

if (TARGET translations_target)
  add_dependencies(${ProjectName} translations_target)
endif()
