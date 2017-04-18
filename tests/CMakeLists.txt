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


# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# Add CMake submodules to the path
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules ${CMAKE_MODULE_PATH})
include(PmCommon)

# add the build dir to the include path
# because moc generates the files there
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Test REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)

# needed to find the generated moc-files
include_directories(${CMAKE_BINARY_DIR}/libpm)

include_directories(${Qt5Test_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Xml_INCLUDE_DIRS})

# needed for quazip
#set (QUAZIP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/quazip/quazip)
#include_directories(${QUAZIP_INCLUDE_DIR})
#link_directories(${CMAKE_BINARY_DIR}/../working_dir)
#link_directories(${CMAKE_BINARY_DIR}/../working_dir/lib)
#set (QUAZIP_LIBRARIES quazip5${PM_DEBUG_D_SUFFIX})

# needed for quazip
find_package(QuaZip REQUIRED)
include_directories(${QUAZIP_INCLUDE_DIRS})

## needed for librsync
#set (RSYNC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/librsync/src)
#include_directories(${RSYNC_INCLUDE_DIR})
#include_directories(../build/ext_libs/librsync/src)

# needed for librsync
find_package(LibRsync REQUIRED)
include_directories(${LIBRSYNC_INCLUDE_DIRS})

macro (ADD_PM_TEST testclass)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../libpm/")
  add_executable(${testclass} ${testclass}.cpp)
  find_package(Qt5Multimedia)
  target_link_libraries(${testclass} pm Qt5::Test Qt5::Widgets Qt5::Multimedia ${QUAZIP_LIBRARIES} ${LIBRSYNC_LIBRARIES})
  set_target_properties(${testclass}
                      PROPERTIES
                      SKIP_BUILD_RPATH FALSE
                      BUILD_WITH_INSTALL_RPATH FALSE
                      )

  add_test( NAME ${testclass} 
  COMMAND ${testclass} -v2 -vs -platform offscreen -xml -o testresults.xml
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/../working_dir)
endmacro ()

# For each (Unit-)Test call the macro add_pm_test(TestMyCoolTest):
#  The Files TestMyCoolTest.cpp and TestMyCoolTest.h will be inlcuded automatically
#  A binary TestMyCoolTest is also builded which could be manually started:
#   cd working_dir
#   ./TestGui
#   start with more verbosity:
#   ./TestGui -v2 -vs

add_pm_test(TestUserConfigOpenVPN)
add_pm_test(TestGui)
add_pm_test(TestPatching)
add_pm_test(TestXmlUpdateParser)
