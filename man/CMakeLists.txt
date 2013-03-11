# CMake script for PhyView.
# Author: Julien Dutheil
# Created: 22/08/2009

IF(MAN)
  INSTALL(FILES phyview.1.${DOC_COMPRESS_EXT} DESTINATION share/man/man1)
ENDIF(MAN)
