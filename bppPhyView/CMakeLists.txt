# CMake script for Bio++ PhyView
# Authors:
#   Julien Dutheil
#   Francois Gindraud (2017)
# Created: 22/08/2009

cmake_policy(SET CMP0028 NEW)

set (CPP_FILES
  PhyView.cpp
  TreeSubWindow.cpp
  TreeCommands.cpp
  )
set (H_MOC_FILES
  PhyView.h
  TreeSubWindow.h
  )

# Phyview
add_executable (phyview ${CPP_FILES})
if (BUILD_STATIC)
  target_link_libraries (phyview ${BPP_LIBS_STATIC} ${qt-libs})
  set_target_properties (phyview PROPERTIES LINK_SEARCH_END_STATIC TRUE)
else (BUILD_STATIC)
  target_link_libraries (phyview ${BPP_LIBS_SHARED} ${qt-libs})
  set_target_properties (phyview PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
endif (BUILD_STATIC)

install (TARGETS phyview DESTINATION ${CMAKE_INSTALL_BINDIR})
