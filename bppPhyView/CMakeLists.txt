# CMake script for Bio++ PhyView
# Author: Julien Dutheil
# Created: 22/08/2009

set(phyview_SRCS
  PhyView.cpp
  TreeSubWindow.cpp
  )

set(phyview_MOC_HDRS
  PhyView.h
  TreeSubWindow.h
  )

qt4_wrap_cpp(phyview_MOC_SRCS ${phyview_MOC_HDRS})
add_executable(phyview ${phyview_SRCS} ${phyview_MOC_SRCS})
target_link_libraries(phyview ${LIBS})

# Install libs
install(TARGETS phyview DESTINATION bin)
