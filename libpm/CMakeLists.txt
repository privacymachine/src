################################################################################
#        Copyright (c) 2013-2017 by the Developers of PrivacyMachine.eu
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

project(PrivacyMachine)
set (PM_STATIC_LIB pm)

# Add CMake submodules to the path
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules ${CMAKE_MODULE_PATH})
include(PmCommon)

# add the build dir to the include path
# because moc generates the files there
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# automatic handle qrc files
set(CMAKE_AUTORCC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(Qt5Xml REQUIRED)


# needed for libsodium
find_package(LibSodium REQUIRED)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/libsodium-1.0.11/src/libsodium/include)
#link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/libsodium-1.0.11/src/libsodium/.libs)

# needed for quazip
find_package(QuaZip REQUIRED)
include_directories(${QUAZIP_INCLUDE_DIRS})
# old:
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/quazip/quazip)
#link_directories(${CMAKE_BINARY_DIR}/../working_dir)
#link_directories(${CMAKE_BINARY_DIR}/../working_dir/lib)
#set (QUAZIP_LIBRARIES quazip5${PM_DEBUG_D_SUFFIX})

# needed for librsync
find_package(LibRsync REQUIRED)
include_directories(${LIBRSYNC_INCLUDE_DIRS})

if(MSVC)
  GET_FILENAME_COMPONENT(Installed64BitVersionFromRegistry
                         "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\OpenSSL (64-bit)_is1;Inno Setup: App Path]" 
                         ABSOLUTE CACHE)

  FIND_PATH(OPENSSL_BINARY_DIR
    NAMES
      ssleay32.dll
    HINTS
      ${Installed64BitVersionFromRegistry}
  )                         
endif()
                         
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Multimedia_INCLUDE_DIRS})
include_directories(${Qt5Xml_INCLUDE_DIRS})


add_definitions(${QT_DEFINITIONS})


set (NoMocSources
                    CpuFeatures.h
                    getMemorySize.cpp
                    getMemorySize.h
                    PmCommand.cpp
                    PmCommand.h
                    PmData.cpp
                    PmData.h
                    VmMaskCurrentConfig.cpp
                    VmMaskCurrentConfig.h
                    VmMaskData.cpp
                    VmMaskData.h
                    VmMaskUserConfig.cpp
                    VmMaskUserConfig.h
                    VmMaskInstance.cpp
                    VmMaskInstance.h
                    VmMaskStaticConfig.cpp
                    VmMaskStaticConfig.h
                    PmLog.cpp
                    PmLog.h
                    PmManager.cpp
                    PmManager.cpp
                    PmManager.h
                    PmManager.h
                    resource.h
                    RunGuard.cpp
                    RunGuard.h
                    SystemConfig.cpp
                    SystemConfig.h
                    UserConfig.cpp
                    UserConfig.cpp
                    UserConfig.h
                    UserConfig.h
                    utils.cpp
                    utils.h
                    PmVpnHelper.cpp
                    PmVpnHelper.h
                    PmVersion.cpp
                    PmVersion.h
                    XmlUpdateParser.cpp
                    XmlUpdateParser.h
                    )

if(MSVC)
  set (NoMocSources ${NoMocSources} CpuFeaturesWindows.cpp)
else()
  set (NoMocSources ${NoMocSources} CpuFeaturesLinux.cpp)
endif()

set (MocReleatedSources
                    images.qrc
                    PmCommandExec.cpp
                    VmInfoIpAddress.cpp
                    WidgetAbout.cpp
                    WidgetCommandExec.cpp
                    WidgetNewTab.cpp
                    WidgetRdpView.cpp
                    WidgetUpdate.cpp
                    WindowMain.cpp
                    UpdateManager.cpp
                    CheckUpdate.cpp
                    VerifiedDownload.cpp
                    WidgetInteractiveUpdate.cpp
                    )

set (MocReleatedHeaders
                    PmCommandExec.h
                    VmInfoIpAddress.h
                    WidgetAbout.h
                    WidgetCommandExec.h
                    WidgetNewTab.h
                    WidgetRdpView.h
                    WidgetUpdate.h
                    WindowMain.h
                    UpdateManager.h
                    CheckUpdate.h
                    VerifiedDownload.h
                    WidgetInteractiveUpdate.h
                    )

set(MocReleatedForms
                    WidgetAbout.ui
                    WidgetCommandExec.ui
                    WidgetNewTab.ui
                    WidgetUpdate.ui
                    WindowMain.ui
                    WidgetInteractiveUpdate.ui
                    )

#set(MocReleatedResources images.qrc)
qt5_add_resources(RESOURCES images.qrc)

qt5_wrap_cpp(MocReleatedHeaders_MOC ${MocReleatedHeaders})
qt5_wrap_ui(MocReleatedForms_MOC ${MocReleatedForms})

add_library(${PM_STATIC_LIB} STATIC
                    ${NoMocSources}
                    ${MocReleatedSources}
                    ${MocReleatedHeaders_MOC}
                    ${MocReleatedForms_MOC}
                    #resources.rc
                    )
            
add_executable(PrivacyMachine
                    main.cpp
                    ${RESOURCES}
                    resources.rc
                    )

               
if(MSVC)
  # find the installed openssl libraries
  GET_FILENAME_COMPONENT(Installed64BitVersionFromRegistry
                         "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\OpenSSL (64-bit)_is1;Inno Setup: App Path]" 
                         ABSOLUTE CACHE)

  FIND_PATH(OPENSSL_BINARY_DIR
    NAMES
      ssleay32.dll
    HINTS
      ${Installed64BitVersionFromRegistry}
  )                         

  # copy main qt libraries to the working dir
  get_target_property(_qtCore Qt5::Core LOCATION)
  get_target_property(_qtGui Qt5::Gui LOCATION)
  get_target_property(_qtWidgets Qt5::Widgets LOCATION)
  get_target_property(_qtNetwork Qt5::Network LOCATION)
  get_target_property(_qtPluginWindows Qt5::QWindowsIntegrationPlugin LOCATION)

  STRING(REGEX REPLACE "\\\\" "/" VS140COMNTOOLS_DIR $ENV{VS140COMNTOOLS}) # Needs installed Visual Studio 2015 (community edition is enough)

  add_custom_command(TARGET PrivacyMachine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${_qtCore}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${_qtGui}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${_qtWidgets}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${_qtNetwork}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}

    # The QT-Plugin 'qwindows.dll' needs to be installed to the subdir 'platforms'
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/platforms
    
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${_qtPluginWindows}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/platforms

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${OPENSSL_BINARY_DIR}/libeay32.dll"
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${OPENSSL_BINARY_DIR}/ssleay32.dll"
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${VS140COMNTOOLS_DIR}../../VC/redist/x64/Microsoft.VC140.CRT/msvcp140.dll"
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${VS140COMNTOOLS_DIR}../../VC/redist/x64/Microsoft.VC140.CRT/vcruntime140.dll"
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${VS140COMNTOOLS_DIR}../../Team Tools/Performance Tools/x64/msvcr120.dll"
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${VS140COMNTOOLS_DIR}../../Team Tools/Performance Tools/x64/msvcp120.dll"
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}

  )
endif()

if (NOT SKIP_FREERDP_CODE)
  if(MSVC)
    set (AdditionalFreeRDPLibraries ${LIBFREERDPCLIENT_LIBRARIES} Mpr.lib)
  else()
    set (AdditionalFreeRDPLibraries ${LIBFREERDPCLIENT_LIBRARIES})
  endif()
  include_directories(${RemoteDisplay_SOURCE_DIR})
else()
  set (AdditionalFreeRDPLibraries "")
endif()

if(MSVC)
  set (MICROSOFT_LIBS wsock32)
endif()

target_link_libraries(${PM_STATIC_LIB}
  ${Qt5Core_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
  ${Qt5Xml_LIBRARIES}
  ${Qt5Test_LIBRARIES}
  Qt5::Multimedia
  ${AdditionalFreeRDPLibraries}
  ${REMOTEDISPLAY_LIBRARIES}
  ${QUAZIP_LIBRARIES}
  ${MICROSOFT_LIBS}
  ${LIBRSYNC_LIB}
  sodium
  stdc++
  )

target_link_libraries(PrivacyMachine
  ${PM_STATIC_LIB}
  )
#MESSAGE("XXXXXX ${LIBRSYNC_LIB}")
#${REMOTEDISPLAY_LIB}

set_target_properties(PrivacyMachine ${PM_STATIC_LIB}
                      PROPERTIES
                      SKIP_BUILD_RPATH FALSE
                      BUILD_WITH_INSTALL_RPATH FALSE
                      )

# Include our extra modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/)
