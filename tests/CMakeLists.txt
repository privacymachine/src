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


# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)


# add the build dir to the include path
# because moc generates the files there
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Test REQUIRED)
find_package(Qt5Widgets REQUIRED)

# needed to find the generated moc-files
include_directories(${CMAKE_BINARY_DIR}/libpm)

# needed for quazip
set (QUAZIP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/quazip/quazip)
include_directories(${QUAZIP_INCLUDE_DIR})
link_directories(${CMAKE_BINARY_DIR}/../working_dir)
link_directories(${CMAKE_BINARY_DIR}/../working_dir/lib)
set (QUAZIP_LIBRARIES quazip5${PM_DEBUG_D_SUFFIX})

# needed for librsync
set (RSYNC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/librsync/src)
include_directories(${RSYNC_INCLUDE_DIR})
include_directories(../build/ext_libs/librsync/src)

macro (ADD_PM_TEST testclass)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../libpm/")
  add_executable(${testclass} ${testclass}.cpp)
  find_package(Qt5Multimedia)
  target_link_libraries(${testclass} pm Qt5::Test Qt5::Widgets Qt5::Multimedia ${QUAZIP_LIBRARIES} librsync)
  set_target_properties(${testclass}
                      PROPERTIES
                      SKIP_BUILD_RPATH FALSE
                      BUILD_WITH_INSTALL_RPATH FALSE
                      )

  add_test( NAME ${testclass} 
  COMMAND ${testclass} -v2 -vs -platform offscreen -xml -o testresults.xml
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/../working_dir)
endmacro ()

# Build a test-binary for each test class
# i.e. to start one manual with full logging:
#   cd working_dir
#   ../tests/TestGui
#   start with more verbosity:
#   ../tests/TestGui -v2 -vs

add_pm_test(TestUserConfigOpenVPN)
add_pm_test(TestGui)
add_pm_test(TestPatching)
