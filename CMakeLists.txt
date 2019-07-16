##=============================================================================
##
## GAPS Analysis Software CMake Build System
##
## Details: This is the top level CMakeLists.txt file
##
##=============================================================================

## Set project name
project(CraneLab C CXX)
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

# make sure we use python 3
set(PYBIND11_PYTHON_VERSION 3.6)
#set(PYTHON_INCLUDE_DIRS "/usr/include/python3.6")
#set(PYTHON_LIBRARIES "/usr/lib/python3.6/config-3.6m-x86_64-linux-gnu/libpython3.6.so")

#
# You need to tell CMake where to find the ROOT installation.
# For ROOT built with CMake.
# Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
list(APPEND CMAKE_PREFIX_PATH $ENV{GAPS})

find_package(GAPS 1.2.0 REQUIRED)
message("-- Found GAPS include dirs at ${GAPS_INCLUDE_DIRS}")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG=1")
## do a release build by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# the CAEN digitizer lib
set(CAEN_LIBRARIES "CAENDigitizer")

#### Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT 6.00 REQUIRED COMPONENTS RIO EG Core Net Minuit Geom GenVector Gui GuiBld Gviz3d GX11 GX11TTF)
include(${ROOT_USE_FILE})
###

# locate the hdf library
#find_package(HDF5)
find_package(Boost 1.54 REQUIRED program_options timer)

# build defintions
add_definitions(-DUSE_BOOST_PROGRAM_OPTIONS)
add_definitions(-DUSE_BOOST_TIMER)
add_definitions(-DUSE_AIXLOG)

# set the path to the hdfwriter project
#set(HDFWRITER_PATH $ENV{HOME}/gaps/projects/build-hdfwriter)
#set(HIGHFIVE_INCLUDE_DIR $ENV{HOME}/software/HighFive/include)
set(SOURCES
    CraneLabDigitizer
    )

option(BUILD_PYBINDINGS "pybindings for the digitizer" ON)

set(PYBIND_VERSION 2.2.4 CACHE STRING "used pybindings version" FORCE)
set(pybindings "pyCaenN6725")


set(DIGITIZER_LIBRARY_SHARED CLCAEN6725)
# build shared library
add_library(${DIGITIZER_LIBRARY_SHARED} SHARED src/CLCAEN6725.cxx)
target_include_directories(${DIGITIZER_LIBRARY_SHARED}
                           PRIVATE
                                #${HIGHFIVE_INCLUDE_DIR}
                                ${ROOT_INCLUDE_DIRS}
                                ${GAPS_INCLUDE_DIRS}
                                #${HDFWRITER_PATH}
                                #${HDF5_INCLUDE_DIR}
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                              )

target_link_libraries(${DIGITIZER_LIBRARY_SHARED}
                          ${ROOT_LIBRARIES}
                          #${HDF5_LIBRARIES}
                          #${Geant4_LIBRARIES}
                          #GAPS::GAPSCommon
                          #GAPS::GAPSAnalysis
                          ${GAPS_LIBRARIES}/libGAPSCommon.so
                          Boost::program_options
                          Boost::timer
                          ${CAEN_LIBRARIES}
                          #GAPS::GAPSReconstruction
                          #${HDFWRITER_LIBRARY_SHARED}
                          #${Boost_LIBRARIES})
                          )


foreach(source ${SOURCES})

    add_executable(${source} src/${source}.cxx)
    target_include_directories(${source}
                               PRIVATE
                                #${HIGHFIVE_INCLUDE_DIR}
                                ${ROOT_INCLUDE_DIRS}
                                ${GAPS_INCLUDE_DIRS}
                                #${HDFWRITER_PATH}
                                #${HDF5_INCLUDE_DIR}
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                              )

    target_link_libraries(${source}
                          ${ROOT_LIBRARIES}
                          #${HDF5_LIBRARIES}
                          #${Geant4_LIBRARIES}
                          #GAPS::GAPSCommon
                          #GAPS::GAPSAnalysis
                          ${GAPS_LIBRARIES}/libGAPSCommon.so
                          Boost::program_options
                          Boost::timer
                          ${CAEN_LIBRARIES}
                          ${DIGITIZER_LIBRARY_SHARED}
                          #GAPS::GAPSReconstruction
                          #${HDFWRITER_LIBRARY_SHARED}
                          #${Boost_LIBRARIES})
                          )
endforeach()

# get external projects - rename external headers to hpp to distinguish
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
                                    #${HIGHFIVE_INCLUDE_DIR}
                                    ${ROOT_INCLUDE_DIRS}
                                    ${GAPS_INCLUDE_DIRS}
                                    #${HDFWRITER_PATH}
                                    #${HDF5_INCLUDE_DIR}
                                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                   $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                              )

    target_link_libraries(${pybindings}
                          PRIVATE
                            ${ROOT_LIBRARIES}
                            #${HDF5_LIBRARIES}
                            #${Geant4_LIBRARIES}
                            #GAPS::GAPSCommon
                            #GAPS::GAPSAnalysis
                            ${GAPS_LIBRARIES}/libGAPSCommon.so
                            Boost::program_options
                            Boost::timer
                            ${CAEN_LIBRARIES}
                            ${DIGITIZER_LIBRARY_SHARED}
                            #GAPS::GAPSReconstruction
                            #${HDFWRITER_LIBRARY_SHARED}
                            #${Boost_LIBRARIES})
                          )

  endif()
endif(BUILD_PYBINDINGS)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/python/CraneLab
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/python/configuration-example.json
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})



