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
#
#set(HDFWRITER_LIBRARY_SHARED ${HDFWRITER_PATH}/libhdfwriter.so)

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
                          #GAPS::GAPSReconstruction
                          #${HDFWRITER_LIBRARY_SHARED}
                          #${Boost_LIBRARIES})
                          )
endforeach()

