################################################################################
#        Copyright (c) 2013-2014 by the Developers of PrivacyMachine.eu
#                         contact@privacymachine.eu
#     OpenPGP-Fingerprint: 5134 F10C 58ED FCA2 7F25 7483 50F4 40FC 4347 D242
#
# Licensed under the EUPL, Version 1.1 or - as soon they will be approved by the
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
cmake_minimum_required(VERSION 2.6)

set (CMAKE_BUILD_TYPE DEBUG)

set (ProjectName PrivacyMachine)
PROJECT(${ProjectName})

# Add CMake submodules to the path
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules ${CMAKE_MODULE_PATH})

if(WIN32)
  set (PM_WINDOWS TRUE)
  add_definitions(-DPM_WINDOWS)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif()

find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork QtXml REQUIRED)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

# add the build dir to the include path 
# because moc generates the files there
include_directories(${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/rdp)


set (NoMocSources
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
                    rdp/WidgetRdpView.cpp
                    )

set (MocReleatedHeaders
                    WindowMain.h
                    WidgetNewTab.h
                    WidgetUpdate.h
                    WidgetCommandExec.h
                    rdp/WidgetRdpView.h
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

set(MocReleatedResources images.qrc)

QT4_WRAP_CPP(MocReleatedHeaders_MOC ${MocReleatedHeaders})
QT4_WRAP_UI(MocReleatedForms_MOC ${MocReleatedForms})
QT4_ADD_RESOURCES(MocReleatedResources_RCC ${MocReleatedResources})

# Translation see <http://www.cmake.org/Wiki/CMake:How_To_Build_Qt4_Software>
# all Files parsed for Translation
file (GLOB HEADER_FILES *.h *.hpp)
file (GLOB CPP_FILES *.cpp)
file (GLOB UI_FILES *.ui)
set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${CPP_FILES} ${UI_FILES} ${HEADER_FILES})
set (TRANSLATIONS_FILES
     ${CMAKE_SOURCE_DIR}/translations/lang_en.ts
     ${CMAKE_SOURCE_DIR}/translations/lang_de.ts
)

option (UPDATE_TRANSLATIONS "Update source translation translations/*.ts files (WARNING: make clean will delete the source .ts files! Danger!)")
if (UPDATE_TRANSLATIONS)
  message ("Warning: Translation files will be owerwritten:\n ${TRANSLATIONS_FILES}\n Disable the option UPDATE_TRANSLATIONS after one build!")
  qt4_create_translation(QM_FILES
                          ${FILES_TO_TRANSLATE}
                          ${TRANSLATIONS_FILES})
else (UPDATE_TRANSLATIONS)
  qt4_add_translation(QM_FILES ${TRANSLATIONS_FILES})
endif (UPDATE_TRANSLATIONS)

add_custom_target (translations_target DEPENDS ${QM_FILES})

if(WIN32)
  add_definitions(-DWINVER=0x0600)
  add_definitions(-D_WIN32_WINNT=0x0600)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)

  # Build with static Runtime (Release or Debug)
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
endif()

add_executable(${ProjectName}
    ${NoMocSources}
    ${MocReleatedSources}
    ${MocReleatedHeaders_MOC}
    ${MocReleatedForms_MOC}
    ${MocReleatedResources_RCC}
    ${QM_FILES}
    build/PrivacyMachine.ini)

# mark non compilable
set_source_files_properties(build/PrivacyMachine.ini PROPERTIES HEADER_FILE_ONLY TRUE)

if(WIN32)
  target_link_libraries(${ProjectName} ${QT_LIBRARIES} ${LIBFREERDPCLIENT_LIBRARIES} Mpr.lib)
else()
  target_link_libraries(${ProjectName} ${QT_LIBRARIES} ${LIBFREERDPCLIENT_LIBRARIES})
endif()

add_dependencies(${ProjectName} translations_target)
