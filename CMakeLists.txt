cmake_minimum_required (VERSION 2.8.11)

project (PrivacyMachine-Project)

# Add CMake submodules to the path
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules ${CMAKE_MODULE_PATH})

enable_testing()

# Default to debug build
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE Debug)
endif()
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  # some libs are building with 'd' suffix
  set (PM_DEBUG_D_SUFFIX "d")
endif()


set(BUILD_SHARED_LIBS ON)

# AL: Needed for compiling Fervor without QWebView
# TODO: remove me
set(SKIP_QWEBVIEW "ON") # Valid values: ON/OFF
# We need to support differet QtWebView-Version if we need HTML-Dialogs:
#   https://blog.qt.io/blog/2016/06/16/qt-5-7-released/
#  https://doc.qt.io/qt-5/qtwebenginewidgets-qtwebkitportingguide.html
if (SKIP_QWEBVIEW)
  # add a define useable in cpp source
  add_definitions(-DSKIP_QWEBVIEW)
endif()

# It is needed that the Project 'ExtLib' is build first
# search an library here explicit to provide nicer error messages to the user
find_library(SomeExtLib_LIB
  NAMES RemoteDisplay libRemoteDisplay
  HINTS   ${CMAKE_CURRENT_SOURCE_DIR}/working_dir/lib
)
if ("${SomeExtLib_LIB}" STREQUAL "SomeExtLib_LIB-NOTFOUND")
  MESSAGE(FATAL_ERROR "ERROR: Please build the required Project 'ext_libs' first (via ./buildExtLibs.sh or opening the file ext_libs/CMakeLists.txt in Qt-Creator")
endif()


# search all Qt-Modules needed for Translation and Installers
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(RemoteDisplay REQUIRED)
include_directories(${REMOTEDISPLAY_INCLUDE_DIR})

include_directories(build_ext_libs/librsync/src)

if(MSVC)
  add_definitions(-D_WIN32_WINNT=0x0601)
  add_definitions(-DWINVER=0x0601)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)

  # used in our source
  add_definitions(-DPM_WINDOWS)

  # enable parallel build
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} /MP")

  # Exception Handling (https://msdn.microsoft.com/en-us/library/1deeycx5.aspx)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
  
  if(NOT DEFINED BUILD_SHARED_LIBS)
    MESSAGE("Build with STATIC Runtime, Build-Type: ${CMAKE_BUILD_TYPE}")
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
  else()
    MESSAGE("Building with SHARED Runtime, Build-Type: ${CMAKE_BUILD_TYPE}")
  endif()
else()
  MESSAGE("Build-Type: ${CMAKE_BUILD_TYPE}")
endif()

# Build all executables and libraries to specific directories: very usefull for installation and RPATH-Handling
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/working_dir)
if(MSVC)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/working_dir)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/working_dir/build_libs) #*.lib needed for linking
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/working_dir/lib)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/working_dir/lib)
endif()
# see https://cmake.org/Wiki/CMake_RPATH_handling
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib" /opt/privacymachine/lib)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)


# Needed for QuaZip
set (CMAKE_POSITION_INDEPENDENT_CODE TRUE)

# Disabled by default: set to ON to get quick compile times but a useless PM ;)
set(SKIP_FREERDP_CODE "OFF") # Valid values: ON/OFF
if (SKIP_FREERDP_CODE)
  # add a define useable in cpp source
  add_definitions(-DSKIP_FREERDP_CODE)
endif()

if(MSVC)
  # we use our own zlib on windows, not the qt-version
  set (ZLIB_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/ext_libs/zlib)
  set (ZLIB_LIBRARY zlib)
  include_directories(${ZLIB_ROOT}) # needed for including zlib.h
  include_directories(${CMAKE_CURRENT_BINARY_DIR}/ext_libs/zlib) # needed for including zconf.h
endif()

# Search the Qt-Packages which are needed for translation and the setups
find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5Multimedia)
find_package(Qt5LinguistTools)
find_package(Qt5Test)

# Build all external libraries (FreeRDP,...)
#add_subdirectory(ext_libs)

# Disabled by default: Copy the firefox profile to the VM at each VM-Mask start
set(USE_OUTSIDE_FIREFOX_PROFILE "OFF") # Valid values: ON/OFF

if (USE_OUTSIDE_FIREFOX_PROFILE)
  add_definitions(-DUSE_OUTSIDE_FIREFOX_PROFILE)
endif()

# Add the source of the PrivacyMachine
add_subdirectory(libpm)

# Add the ProblemReporter
add_subdirectory(problem_reporter)

## Add the uac_deploy tool
#add_subdirectory(uac_deploy)

# Add the UnitTests
add_subdirectory(tests)

# for Translation read <http://doc.qt.io/qt-5/internationalization.html>
#  <http://www.cmake.org/Wiki/CMake:How_To_Build_Qt4_Software>
#  <http://doc.qt.io/qt-5/cmake-manual.html#qt5linguisttools-macros>

# Define all Files parsed for Translations
file (GLOB FILES_TO_TRANSLATE_PM 
              libpm/*.h 
              libpm/*.hpp
              libpm/*.cpp
              libpm/*.ui)

file (GLOB FILES_TO_PARSE_TRANSLATE_REPORTER
              problem_reporter/*.h 
              problem_reporter/*.hpp
              problem_reporter/*.cpp
              problem_reporter/*.ui)

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE_PM} ${FILES_TO_PARSE_TRANSLATE_REPORTER})
set (TRANSLATIONS_FILES
     ${CMAKE_CURRENT_SOURCE_DIR}/libpm/translations/lang_de.ts
)

OPTION(REFRESH_TRANSLATIONS "Refresh translations on compile" OFF)

# HOWTO Updating new Translations
# o) To get new translateable items to the file libpm/translations/lang_de.ts change the following line to "set (REFRESH_TRANSLATIONS ON)"
# o) Compile the Project ONE TIME and imediately comment it out again, because a 'clean' command will delete the updated lang_de.ts file:
#      cd build
#      make translations_target
# o) Start 'linguist' and open the file libpm/translations/lang_de.ts
# o) Do all translations and save the file
# o) On the next build the translations are located in the file build/libpm/lang_de.qm
set (REFRESH_TRANSLATIONS OFF)

if (REFRESH_TRANSLATIONS)
  qt5_create_translation(QM_FILES
                         ${FILES_TO_TRANSLATE}
                         ${TRANSLATIONS_FILES})
else ()
  qt5_add_translation(QM_FILES 
                      ${TRANSLATIONS_FILES})
endif ()

add_custom_target (translations_target DEPENDS ${QM_FILES})

include(CPackComponent)

# Determine Installer-Name dependend on OS (and the Buildmachine-HostName)
site_name(ENV_HOSTNAME)
set (PMInstallerSuffix ${CMAKE_HOST_SYSTEM_NAME}) # Windows, Linux
if ("${ENV_HOSTNAME}" MATCHES "PM-Ubuntu-1604")
  set (PMInstallerSuffix ubuntu-1604)
endif()
if ("${ENV_HOSTNAME}" MATCHES "PM-debian-stable")
  set (PMInstallerSuffix debian-jessie)
endif()
if ("${ENV_HOSTNAME}" MATCHES "PM-Win7")
  set (PMInstallerSuffix win64)
endif()

if ("${CMAKE_BUILD_TYPE}" MATCHES "Release")
  set(CPACK_PACKAGE_VENDOR "PrivacyMachine-Team")
  set(CPACK_PACKAGE_CONTACT ${CPACK_PACKAGE_VENDOR})
  
  set(CPACK_PACKAGE_VERSION_MAJOR "0") # Major-Version
  set(CPACK_PACKAGE_VERSION_MINOR "9") # Minor-Version
  set(CPACK_PACKAGE_VERSION_PATCH "0") # Component-Major-Version
  set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}") # only the first 3 numbers are interpreted by the windows installer
  set(CPACK_PACKAGE_FILE_NAME "PrivacyMachine_${PMInstallerSuffix}_${CPACK_PACKAGE_VERSION_MAJOR}_${CPACK_PACKAGE_VERSION_MINOR}_${CPACK_PACKAGE_VERSION_PATCH}")
 
 
  set(CPACK_PACKAGE_NAME "PrivacyMachine")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "PrivacyMachine: a browser which protects your privacy")
  set(PM_HOMEPAGE "https://www.privacymachine.eu")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}") # Windows: this is needed to prevent the version in the target directory

  cpack_add_component(pm_binaries
    DISPLAY_NAME "PrivacyMachine - Binaries"
    HIDDEN
    GROUP "needed")

  cpack_add_component(pm_stuff
    DISPLAY_NAME "PrivacyMachine - Additional Components"
    HIDDEN
    GROUP "needed")              

  if(MSVC)
    set(CPACK_WIX_PRODUCT_GUID "70fa2aff-f244-42b9-b86e-421ec6d367c5") # do not change!
    set(CPACK_WIX_UPGRADE_GUID "80a64338-af01-463a-b7e5-3775f6bc10b8") # do not change!
    set(CPACK_WIX_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/install_windows/pm.template.in")
    set(CPACK_WIX_PATCH_FILE "${CMAKE_CURRENT_SOURCE_DIR}/install_windows/pm.XMLPatches.xml")
    set(CPACK_WIX_LIGHT_EXTRA_FLAGS -loc "${CMAKE_CURRENT_SOURCE_DIR}/install_windows/CustomStrings.wxl")
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_SOURCE_DIR}/install_windows/PrivacyMachine_Logo_AllIcons.ico") # Icon shown at Add/Remove Programs
    set(CPACK_WIX_UI_BANNER "${CMAKE_CURRENT_SOURCE_DIR}/install_windows/installer_banner.bmp")
    set(CPACK_WIX_UI_DIALOG "${CMAKE_CURRENT_SOURCE_DIR}/install_windows/installer_welcome_and_completion.bmp")
    set(CPACK_WIX_PROPERTY_ARPURLINFOABOUT ${PM_HOMEPAGE})
    set(CPACK_WIX_PROPERTY_ARPCOMMENTS ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
    
    cpack_add_component(openssl_libraries
      DISPLAY_NAME "OpenSSL - Libraries"
      HIDDEN
      GROUP "needed")

    cpack_add_component(ms_runtime
        DISPLAY_NAME "Visual Studio Runtimes"
        DESCRIPTION "Runtime dll's needed by Microsoft Visual Studio"
        HIDDEN
        GROUP "needed")

    install(FILES 
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plink.exe 
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pscp.exe 
            DESTINATION .
            COMPONENT pm_binaries)


    STRING(REGEX REPLACE "\\\\" "/" VS140COMNTOOLS_DIR $ENV{VS140COMNTOOLS}) # Needs installed Visual Studio 2015 (community edition is enough)
    install(FILES 
              "${VS140COMNTOOLS_DIR}../../VC/redist/x64/Microsoft.VC140.CRT/msvcp140.dll"
              "${VS140COMNTOOLS_DIR}../../VC/redist/x64/Microsoft.VC140.CRT/vcruntime140.dll"
              "${VS140COMNTOOLS_DIR}../../Team Tools/Performance Tools/x64/msvcr120.dll"
              "${VS140COMNTOOLS_DIR}../../Team Tools/Performance Tools/x64/msvcp120.dll"
              COMPONENT ms_runtime
              DESTINATION .)

    cpack_add_component(qtlibs
        DISPLAY_NAME "Qt Libraries"
        HIDDEN
        GROUP "needed")
        
    # Add main qt libraries          
    get_target_property(_loc Qt5::Core LOCATION)
    install(FILES ${_loc} COMPONENT qtlibs DESTINATION .)

    get_target_property(_loc Qt5::Gui LOCATION)
    install(FILES ${_loc} COMPONENT qtlibs DESTINATION .)

    get_target_property(_loc Qt5::Widgets LOCATION)
    install(FILES ${_loc} COMPONENT qtlibs DESTINATION .)
  
    get_target_property(_loc Qt5::Network LOCATION)
    install(FILES ${_loc} COMPONENT qtlibs DESTINATION .)
  
    # The QT-Plugin 'qwindows.dll' needs to be installed to the subdir 'platforms'
    get_target_property(_loc Qt5::QWindowsIntegrationPlugin LOCATION)
    install(FILES ${_loc} COMPONENT qtlibs DESTINATION platforms)
 
    install(FILES     
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/PrivacyMachine.exe
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ProblemReporter.exe
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/quazip5.dll
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zlib1.dll
            DESTINATION .
            COMPONENT pm_binaries)

    install(FILES 
            ## FreeRDP
            #${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/freerdp.dll
            #${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/freerdp-client.dll
            #${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rdtk.dll
            #${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RemoteDisplay.dll
            #${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/wfreerdp-client.dll
            #${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/winpr.dll
            # quazip
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/quazip5.dll
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zlib1.dll
            DESTINATION .
            COMPONENT pm_binaries)

    install(FILES 
            "${OPENSSL_BINARY_DIR}/libeay32.dll"
            "${OPENSSL_BINARY_DIR}/ssleay32.dll"
            DESTINATION .
            COMPONENT openssl_libraries)

    install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UserGuide_en.pdf 
                  DESTINATION .
                  COMPONENT pm_stuff)

    install(DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/conf/vpn/CryptoFree/
                  DESTINATION conf/vpn/CryptoFree
                  COMPONENT pm_stuff)

    install(DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/conf/vpn/VpnGate/
                  DESTINATION conf/vpn/VpnGate
                  COMPONENT pm_stuff)

    install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/conf/PrivacyMachine_Example_de.ini 
                  DESTINATION conf/
                  COMPONENT pm_stuff)

    install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/conf/PrivacyMachine_Example_en.ini 
                  DESTINATION conf/
                  COMPONENT pm_stuff)

    # WIX can only pack files below 2GB
    #install(DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/BaseDisk/
    #              DESTINATION BaseDisk
    #              COMPONENT pm_stuff)

  else()
    set(CPACK_DEBIAN_PACKAGE_NAME "privacymachine")
    set(CPACK_DEBIAN_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER  ${CPACK_PACKAGE_VENDOR})
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
    set(CPACK_DEBIAN_PACKAGE_SECTION "web")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE ${PM_HOMEPAGE})
    
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5widgets5 (>= 5.3.2)")
    
    set(CPACK_PACKAGING_INSTALL_PREFIX "/opt")
                
    install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/PrivacyMachine
            DESTINATION privacymachine/bin
            COMPONENT pm_binaries)

    install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ProblemReporter
            DESTINATION privacymachine/bin
            COMPONENT pm_binaries)

    install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UserGuide_en.pdf
            DESTINATION privacymachine/conf/
            COMPONENT pm_stuff)

    install(DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/conf/vpn/CryptoFree/
            DESTINATION privacymachine/conf/vpn/CryptoFree
            COMPONENT pm_stuff)

    install(DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/conf/vpn/VpnGate/
            DESTINATION privacymachine/conf/vpn/VpnGate
            COMPONENT pm_stuff)

    install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/conf/PrivacyMachine_Example_de.ini 
            DESTINATION privacymachine/conf/
            COMPONENT pm_stuff)

    install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/conf/PrivacyMachine_Example_en.ini 
            DESTINATION privacymachine/conf/
            COMPONENT pm_stuff)

  endif()

  include(CPack)
endif()
