# CMake script for Bio++ PhyView
# Authors:
#   Julien Dutheil
#   Francois Gindraud (2017)
# Created: 22/08/2009

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
add_executable (phyview ${CPP_FILES} ${CPP_FILES})
if (BUILD_STATIC)
  target_link_libraries (phyview ${BPP_LIBS_STATIC} ${qt-libs})
else (BUILD_STATIC)
  target_link_libraries (phyview ${BPP_LIBS_SHARED} ${qt-libs})
endif (BUILD_STATIC)

install (TARGETS phyview DESTINATION ${CMAKE_INSTALL_BINDIR})
