# CMake script for Bio++ PhyView
# Author: Julien Dutheil
# Created: 22/08/2009

# Global parameters
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(bpp-qt CXX C)
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

SET(CMAKE_CXX_FLAGS "-Wall")
IF(NOT NO_VIRTUAL_COV)
  SET(NO_VIRTUAL_COV FALSE CACHE BOOL
      "Disable covariant return type with virtual inheritance, for compilers that do not support it."
      FORCE)
ENDIF(NOT NO_VIRTUAL_COV)

IF(NO_VIRTUAL_COV)
  MESSAGE("-- Covariant return with virtual inheritance disabled.")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_VIRTUAL_COV=1")
ENDIF(NO_VIRTUAL_COV)

#static linkage?
IF(NOT BUILD_STATIC)
  SET(BUILD_STATIC FALSE CACHE BOOL
      "Enable static linkage."
      FORCE)
ENDIF()
IF(BUILD_STATIC)
  MESSAGE("-- Static linkage requested.")
  SET(CMAKE_CXX_FLAGS "-static -static-libgcc ${CMAKE_CXX_FLAGS}")
ENDIF()

# Set the CMAKE_PREFIX_PATH for the find_library fonction when using non
# standard install location
IF(CMAKE_INSTALL_PREFIX)
  SET(CMAKE_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}" ${CMAKE_PREFIX_PATH})
ENDIF(CMAKE_INSTALL_PREFIX)

#here is a useful function:
MACRO(IMPROVED_FIND_LIBRARY OUTPUT_LIBS lib_name include_to_find)
  #start:
  FIND_PATH(${lib_name}_INCLUDE_DIR ${include_to_find})

  IF(BUILD_STATIC)
    FIND_PATH(${lib_name}_LIBRARY_DIR lib${lib_name}.a HINTS ${CMAKE_PREFIX_PATH} ${CMAKE_PREFIX_PATH}/lib)
    IF(${lib_name}_LIBRARY_DIR)
      SET(${lib_name}_LIBRARY ${${lib_name}_LIBRARY_DIR}/lib${lib_name}.a)
    ELSE(${lib_name}_LIBRARY_DIR)
      MESSAGE(FATAL_ERROR "${lib_name} static library could not be found.")
    ENDIF(${lib_name}_LIBRARY_DIR)
  ELSE(BUILD_STATIC)
    SET(${lib_name}_NAMES ${lib_name} ${lib_name}lib ${lib_name}dll)
    FIND_LIBRARY(${lib_name}_LIBRARY NAMES ${${lib_name}_NAMES})
  ENDIF(BUILD_STATIC)

  IF(${lib_name}_LIBRARY)
    MESSAGE("-- Library ${lib_name} found here:")
    MESSAGE("   includes : ${${lib_name}_INCLUDE_DIR}")
    MESSAGE("   libraries: ${${lib_name}_LIBRARY}")
  ELSE(${lib_name}_LIBRARY)
    MESSAGE(FATAL_ERROR "${lib_name} required but not found.")
  ENDIF(${lib_name}_LIBRARY)
  
  #add the dependency:
  INCLUDE_DIRECTORIES(${${lib_name}_INCLUDE_DIR})
  SET(${OUTPUT_LIBS} ${${OUTPUT_LIBS}} ${${lib_name}_LIBRARY})
ENDMACRO(IMPROVED_FIND_LIBRARY)

#Find the libraries. The order is very important for static linkage, it won't
#link if you change it!
IMPROVED_FIND_LIBRARY(LIBS bpp-qt Bpp/Qt/QtGraphicDevice.h)
IMPROVED_FIND_LIBRARY(LIBS bpp-phyl Bpp/Phyl/Tree.h)
#Not needed for now:
#IMPROVED_FIND_LIBRARY(LIBS bpp-seq Bpp/Seq/Alphabet/Alphabet.h)
IMPROVED_FIND_LIBRARY(LIBS bpp-core Bpp/Clonable.h)

# Find the Qt installation
FIND_PACKAGE(Qt4 4.4.0 COMPONENTS QtCore QtGui REQUIRED)
INCLUDE(${QT_USE_FILE})
SET(LIBS ${LIBS} ${QT_LIBRARIES})

# Subdirectories
ADD_SUBDIRECTORY(bppPhyView)

# Packager
SET(CPACK_PACKAGE_NAME "bppsuite")
SET(CPACK_PACKAGE_VENDOR "Bio++ Development Team")
SET(CPACK_PACKAGE_VERSION "0.1.0")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The Bio++ Program Suite")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING.txt")
SET(CPACK_RESOURCE_FILE_AUTHORS "${CMAKE_SOURCE_DIR}/AUTHORS.txt")
SET(CPACK_RESOURCE_FILE_INSTALL "${CMAKE_SOURCE_DIR}/INSTALL.txt")
SET(CPACK_SOURCE_GENERATOR "TGZ")
SET(CPACK_SOURCE_IGNORE_FILES
 "CMakeFiles"
 "Makefile"
 "_CPack_Packages"
 "CMakeCache.txt"
 ".*\\\\.cmake"
 ".*\\\\.gz"
 ".*\\\\.deb"
 ".*\\\\.rpm"
 "\\\\..*\\\\.swp"
 "bppPhyView/\\\\..*"
 "bppPhyView/phyview"
 "doc/bppphyview/"
 "doc/bppphyview.info"
 "debian/tmp"
 "debian/bppphyview"
 ${CPACK_SOURCE_IGNORE_FILES}
)
IF (MACOS)
  SET(CPACK_GENERATOR "Bundle")
ENDIF()

set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
include(CPack)

#This adds the 'dist' target
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
IF (UNIX)
#This creates deb packages:
ADD_CUSTOM_TARGET(deb fakeroot debian/rules binary)
ADD_DEPENDENCIES(deb dist info)

#This creates rpm packages:
ADD_CUSTOM_TARGET(rpm rpmbuild -ta ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.gz)
ADD_DEPENDENCIES(rpm dist info)

#ADD_DEPENDENCIES(info install)
ENDIF(UNIX)
