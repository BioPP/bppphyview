# SPDX-FileCopyrightText: The Bio++ Development Group
#
# SPDX-License-Identifier: CECILL-2.1

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
  set_target_properties (phyview PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
endif (BUILD_STATIC)

install (TARGETS phyview DESTINATION ${CMAKE_INSTALL_BINDIR})
