##=============================================================================
##
## CMakeLists.txt fiel for DACTYLOS
## 
## DACTYLOS - work with CAEN N6725 didigiezers
## 
##=============================================================================

## Set project name
project(dactylos C CXX)
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

# we want to use modern c++ features, however this is 
# very sensitiv to the standard CERN's root has been built with
# it most likely must be the same.
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard to be used, 11, 14, 17 or 20")
message(STATUS "Building with C++ standard ${CMAKE_CXX_STANDARD}")

# FIXME: this is primed for Ubuntu currently,
# but it needs to become more system independend

# make sure we use python 3
set(PYBIND11_PYTHON_VERSION 3.6)

find_package (Python COMPONENTS Interpreter Development)

# You need to tell CMake where to find the ROOT installation.
# For ROOT built with CMake.
# Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG=1")
## do a release build by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# the CAEN digitizer libraries need to be installed
# they will just be linked with -l, so they must 
# be in one of the paths known to the linker
set(CAEN_LIBRARIES "CAENDigitizer")


#### Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT 6.00 REQUIRED COMPONENTS Core)
include(${ROOT_USE_FILE})

# build shared libraries
# for the digitizer readout
#set(DIGITIZER_LIBRARY_SHARED CaenN6725)
# the analysis lib
#set(DACTYLOS_ANALYSIS_LIBRARY_SHARED DactylosAnalysis)

set(DACTYLOS_LIBRARY_SHARED Dactylos)

# build pybindings
option(BUILD_PYBINDINGS "pybindings for the digitizer" ON)
set(PYBIND_VERSION 2.2.4 CACHE STRING "used pybindings version" FORCE)
set(pybindings "_pyCaenN6725")
#set(pybindings_analysis "_trapezoidal_shaper")
set(PYBINDINGS_DACTYLOS "_dactylos")

# building - libraries
#add_library(${DIGITIZER_LIBRARY_SHARED} SHARED src/CaenN6725.cxx)
#target_include_directories(${DIGITIZER_LIBRARY_SHARED}
#                           PRIVATE
#                                ${ROOT_INCLUDE_DIRS}
#                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#                                $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#                              )
#
#target_link_libraries(${DIGITIZER_LIBRARY_SHARED}
#                          ${ROOT_LIBRARIES}
#                          ${CAEN_LIBRARIES}
#                          )
#
#add_library(${DACTYLOS_ANALYSIS_LIBRARY_SHARED} SHARED dactylos/analysis/shaping/trapezoidal_shaper.cxx)
#target_include_directories(${DACTYLOS_ANALYSIS_LIBRARY_SHARED}
#                           PRIVATE
#                                ${ROOT_INCLUDE_DIRS}
#                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#                                $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#                              )
#
#target_link_libraries(${DACTYLOS_ANALYSIS_LIBRARY_SHARED}
#                          ${ROOT_LIBRARIES}
#                          ${CAEN_LIBRARIES}
#                          )

# simplify - add everything together in one library
add_library(${DACTYLOS_LIBRARY_SHARED} SHARED src/trapezoidal_shaper.cxx src/CaenN6725.cxx)
target_include_directories(${DACTYLOS_LIBRARY_SHARED}
                           PRIVATE
                                ${ROOT_INCLUDE_DIRS}
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                              )

target_link_libraries(${DACTYLOS_LIBRARY_SHARED}
                          ${ROOT_LIBRARIES}
                          ${CAEN_LIBRARIES}
                          )


if (BUILD_PYBINDINGS)
message(STATUS "Checking for pyoind11....")
find_package(pybind11 )
  if(NOT ${pybind11_FOUND})
    message(WARNING "-- Can not find pybind11! Please install with sudo apt install pybind11-dev on a linux system!")
    set(BUILD_PYBINDINGS OFF CACHE BOOL "pybindings for the digitizer" FORCE)
  else()
    message(STATUS "-- found pybind11, building pybindings.")
    pybind11_add_module(${pybindings} MODULE src/module.cxx)
    target_include_directories(${pybindings}
                               PRIVATE
                                    ${ROOT_INCLUDE_DIRS}
                                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                   $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                              )

    target_link_libraries(${pybindings}
                          PRIVATE
                            ${ROOT_LIBRARIES}
                            ${CAEN_LIBRARIES}
                            ${DIGITIZER_LIBRARY_SHARED}
                            ${DACTYLOS_LIBRARY_SHARED}
                          )

# for now is included in the above, but in the future also the pybindings 
# will get renamed    
#    pybind11_add_module(${pybindings_analysis} MODULE dactylos/analysis/shaping/module.cxx)
#    target_include_directories(${pybindings_analysis}
#                               PRIVATE
#                                   dactylos/analysis/shaping/
#                                   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#                                   $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#                              )
#
#    target_link_libraries(${pybindings_analysis}
#                          PRIVATE
#                            ${DACTYLOS_ANALYSIS_LIBRARY_SHARED}
#                          )

  endif()

endif(BUILD_PYBINDINGS)

#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/python/configuration-example.json
#        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
#        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#
## create symlinks for the binaries
#install(CODE "execute_process( \
#        COMMAND ${CMAKE_COMMAND} -E create_symlink \
#        ${CMAKE_CURRENT_SOURCE_DIR}/python/CraneLab \
#        ${CMAKE_CURRENT_BINARY_DIR}/CraneLab   \
#        )"
#       )
#
