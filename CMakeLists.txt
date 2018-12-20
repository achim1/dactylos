##=============================================================================
##
## CraneLab Software CMake Build System
##
## Details: This is the top level CMakeLists.txt file
##
##=============================================================================

## Set project name
project(CraneLab C CXX)
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

## Set the software version number
set(CRANELAB_VERSION_MAJOR 0)
set(CRANELAB_VERSION_MINOR 0)
set(CRANELAB_VERSION_PATCH 1dev)

set(CRANELAB_VERSION_NUM "cranelab-v${CRANELAB_VERSION_MAJOR}-${CRANELAB_VERSION_MINOR}-${CRANELAB_VERSION_PATCH}" CACHE STRING "cranelab-v${CRANELAB_VERSION_MAJOR}-${CRANELAB_VERSION_MINOR}-${CRANELAB_VERSION_PATCH}")

message(STATUS "Running CMake configuration for CraneLab software version: ${CRANELAB_VERSION_NUM}")

# the CAEN digitizer lib
set(CAEN_LIBRARIES "CAENDigitizer")


## ================================================================
## cmake build type
## can be either "Release" or "Debug"

## in case of Release build, we set the NDEBUG preprocessor directive
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG=1")

## do a release build by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

## ==================================================================
# Set what to build

set(BUILD_TESTS OFF CACHE BOOL "build available test executables")

## ====================================================================
# Additional build options

# ask the user if she wants to use aixlog
# aixlog allows us to use advanced logging macros
# with a timestamp, number of line in code, colors, etc...
option(USE_AIXLOG "use advanced logging system with the help of aixlog")


## ============================================================
## if necessary, download third party stuff now

set(EXTERNAL_HEADER_FILES "")
# get external projects - rename external headers to hpp to distinguish
message(STATUS "Checking out external software..")
if (USE_AIXLOG)
message(STATUS "-- aixlog.hpp")
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/extern/aixlog/aixlog.hpp)
  message(STATUS "-- -- found!")
else()
  message(STATUS "-- -- attempting to download aixlog.hpp header file")
  file(DOWNLOAD https://raw.githubusercontent.com/badaix/aixlog/master/include/aixlog.hpp ${CMAKE_CURRENT_SOURCE_DIR}/extern/aixlog/aixlog.hpp SHOW_PROGRESS)
  message(STATUS "-- -- download completed!")
endif()
list(APPEND EXTERNAL_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/extern/aixlog/aixlog.hpp)
endif(USE_AIXLOG)

if (BUILD_TESTS)
  message(STATUS "-- catch.hpp")
  if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/extern/catch/catch.hpp)
    message(STATUS "-- -- found!")
  else()
    message(STATUS "-- -- attempting to download catch.hpp header file")
    # download the latest catch2 (single header)
    file(DOWNLOAD https://github.com/catchorg/Catch2/releases/download/v2.2.1/catch.hpp ${CMAKE_CURRENT_SOURCE_DIR}/extern/catch/catch.hpp SHOW_PROGRESS)
    message(STATUS "-- -- download completed")
  endif()
list(APPEND EXTERNAL_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/extern/catch/catch.hpp)
endif()

## ============================================================================
##
##  Setup install directories
##
## ============================================================================

# if CMAKE_INSTALL_PREFIX set to default (i.e. /usr/local) then change
# to binary directory
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE)
endif()

# define the install directory as CMAKE_INSTALL_PREFIX/CRANELAB_VERSION_NUM
if(NOT(CMAKE_INSTALL_PREFIX MATCHES "/${CRANELAB_VERSION_NUM}$"))
    set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/${CRANELAB_VERSION_NUM}" CACHE PATH "CRANELAB installation directory." FORCE)
endif()

message(STATUS "Installing into ${CMAKE_INSTALL_PREFIX}")

# define the install lib, include and bin directories
set(CRANELAB_INSTALL_LIBRARY_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "CraneLab installation directory for libraries")
set(CRANELAB_INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "CraneLab installation directory for header files")
set(CRANELAB_INSTALL_BINARY_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "CraneLab installation directory for executables")
set(CRANELAB_INSTALL_TEST_DIR   "${CMAKE_INSTALL_PREFIX}/tests" CACHE PATH "CraneLab installation directory for test executables")


# Make relative paths absolute (needed later on)
foreach(p LIBRARY BINARY INCLUDE MACRO)
  set(var CRANELAB_INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()


# set the CRANELAB header output directory
# all CRANELAB header files will be copied into the installataion directory
# the headers will be placed withing their CRANELAB module paths inside
# ${CMAKE_INSTALL_PREFIX}/${CRANELAB_VERSION_NUM}/include
set(CRANELAB_HEADER_OUTPUT_DIRECTORY  ${CMAKE_INSTALL_PREFIX}/include CACHE PATH "default header install path" FORCE)

# set the CRANELAB macros output directory
# all CRANELAB GEANT4 macros will be copied into the installataion directory
set(CRANELAB_MACRO_OUTPUT_DIRECTORY  ${CMAKE_INSTALL_PREFIX}/bin/macros CACHE PATH "default macro install path" FORCE)

# testdata dir
set(CRANELAB_TESTDATA ${CMAKE_INSTALL_PREFIX}/resources/testdata CACHE PATH "default testdata dir" FORCE)

## ============================================================================
##
##  Set options for building libraries
##
## ============================================================================

set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")


# In many cases we want to make sure that the required libraries are
# always found independently from LD_LIBRARY_PATH and the install location.
# To always use RPATH we set the following options:

#To enable RPATH usage on OSX and osMac
set(CMAKE_MACOSX_RPATH 1)

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CRANELAB_INSTALL_LIBRARY_DIR}")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CRANELAB_INSTALL_LIBRARY_DIR}" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CRANELAB_INSTALL_LIBRARY_DIR}")
ENDIF("${isSystemDir}" STREQUAL "-1")

## ============================================================================
##
##  CERN ROOT ANALYSIS FRAMEWORK
##
## ============================================================================


# You need to tell CMake where to find the ROOT installation.
# For ROOT built with CMake.
# Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

if(APPLE)
    set(ROOT_REQUIRED_COMPONENTS RIO Core Net Geom GenVector GuiBld Eve Gviz Gui GCocoa RGL gdml)
    set(ROOT_OPTIONAL_COMPONENTS Minuit EG) #EG is not used yet, maybe needed in the future if enhnanced PDG functionality is needed
elseif(UNIX)
    set(ROOT_REQUIRED_COMPONENTS RIO Core Net Geom GenVector GuiBld Eve Gviz Gui GX11 GX11TTF Gdml)
    set(ROOT_OPTIONAL_COMPONENTS Minuit EG)
endif()

# Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT 6.00 REQUIRED COMPONENTS ${ROOT_REQUIRED_COMPONENTS} OPTIONAL_COMPONENTS ${ROOT_OPTIONAL_COMPONENTS})
#find_package(ROOT 6.00 REQUIRED COMPONENTS RIO Core Net Minuit Geom GenVector GuiBld Eve Gviz Gui GCocoa RGL)
#find_package(ROOT 6.00 REQUIRED COMPONENTS RIO Core Net Minuit Geom GenVector GuiBld Eve Gviz Gui GX11 GX11TTF)


## use this to print out all the cmake variables
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
#message(STATUS "${ROOT_REQUIRED_COMPONENTS}")
#message(STATUS "Component ${ROOT_FIND_COMPONENTS} found")
#message(STATUS "Component ${ROOT_minuit2_FOUND} found")
#message(STATUS "Component ${ROOT_FOUND} found")

# Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
if(ROOT_FOUND)
    include(${ROOT_USE_FILE})
else(NOT ROOT_FOUND)
    message(FATAL_ERROR "You need CERN ROOT for this program to work!")
endif(ROOT_FOUND)



## ============================================================================
#
#  Determine where all the header files are located
#  and set global include directories
#
## ============================================================================

file(GLOB_RECURSE CRANELAB_HEADER_FILES
    "*.hh"
    )

set(CRANELAB_INCLUDE_DIRS "")
foreach(_headerFile ${CRANELAB_HEADER_FILES})
    get_filename_component(_dir ${_headerFile} PATH)
    list(APPEND CRANELAB_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES CRANELAB_INCLUDE_DIRS)

# append the necessary extarnal includes too
#list(APPEND CRANELAB_INCLUDE_DIRS EXTERNAL_INCLUDE_DIRS)
set(EXTERNAL_INCLUDE_DIRS "")
foreach(_headerFile ${EXTERNAL_HEADER_FILES})
    get_filename_component(_dir ${_headerFile} PATH)
    list(APPEND EXTERNAL_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES EXTERNAL_INCLUDE_DIRS)

include_directories(${CRANELAB_INCLUDE_DIRS}
                    ${EXTERNAL_INCLUDE_DIRS}
                   )

## ==================================================================
# documentation


# add a target to generate API documentation with Doxygen
# taken from https://majewsky.wordpress.com/2010/08/14/tip-of-the-day-cmake-and-doxygen/
set(CRANELAB_DOCUMENTATION_DIR docs-html)
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc
${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generating API documentation with Doxygen" VERBATIM)
else()
message(WARNING "Doxygen not found! Unable to make the documentation!")
endif(DOXYGEN_FOUND)

## ============================================================================
##
##  Set directories to include in project
##
## ============================================================================

# a bunch of on/off switches for the different projects
# enable tests for ctest
# this will tell cmake to look for tests
if (BUILD_TESTS)
  enable_testing()
endif()


set(SOURCES 
      CraneLabDigitizer
)

# build the sources
foreach(source ${SOURCES})
    add_executable(${source} src/${source}.cxx)
    target_link_libraries(${source} ${ROOT_LIBRARIES} ${GAPS_LIBS} ${HDF5_LIBRARIES} ${Geant4_LIBRARIES} ${HDFWRITER_LIBRARY_SHARED} ${Boost_LIBRARIES} ${CAEN_LIBRARIES})
endforeach()


# install the external headers
foreach(header_ ${EXTERNAL_HEADER_FILES})
  install(FILES
          ${header_}
          DESTINATION ${CRANELAB_HEADER_OUTPUT_DIRECTORY}
          COMPONENT dev
         )
endforeach()

# install the documentation
install(DIRECTORY ${PROJECT_BINARY_DIR}/${CRANELAB_DOCUMENTATION_DIR} DESTINATION ${CMAKE_INSTALL_PREFIX} OPTIONAL)

