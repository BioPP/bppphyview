#Copyright or © or Copr. CNRS, (November 16, 2004, 2005, 2006)
#
#This software is a computer program whose purpose is to draw phylogenetic trees.
#
#This software is governed by the CeCILL  license under French law and
#abiding by the rules of distribution of free software.  You can  use, 
#modify and/ or redistribute the software under the terms of the CeCILL
#license as circulated by CEA, CNRS and INRIA at the following URL
#"http://www.cecill.info". 
#
#As a counterpart to the access to the source code and  rights to copy,
#modify and redistribute granted by the license, users are provided only
#with a limited warranty  and the software's author,  the holder of the
#economic rights,  and the successive licensors  have only  limited
#liability. 
#
#In this respect, the user's attention is drawn to the risks associated
#with loading,  using,  modifying and/or developing or reproducing the
#software by the user in light of its specific status of free software,
#that may mean  that it is complicated to manipulate,  and  that  also
#therefore means  that it is reserved for developers  and  experienced
#professionals having in-depth computer knowledge. Users are therefore
#encouraged to load and test the software's suitability as regards their
#requirements in conditions enabling the security of their systems and/or 
#data to be ensured and,  more generally, to use and operate it in the 
#same conditions as regards security. 
#
#The fact that you are presently reading this means that you have had
#knowledge of the CeCILL license and that you accept its terms.

H_FILES = \
PhyView.h

CPP_FILES = \
PhyView.cpp

AUTOMAKE_OPTIONS = gnu

bin_PROGRAMS = phyview
if STATIC
phyview_LDFLAGS = -static
else
phyview_LDFLAGS =
endif
phyview_LDADD = -lbpp-utils -lbpp-numcalc -lbpp-seq -lbpp-phyl -lbpp-qt
phyview_SOURCES = $(H_FILES) $(CPP_FILES)

# You have one .h file, it's called myapp.h. Therefore, here I list
# its mocced name, moc_myapp.cpp.
nodist_phyview_SOURCES = \
	moc_PhyView.cpp

# This rule lets GNU make create any moc_*.cpp from the equivalent *.h
moc_%.cpp: %.h
	moc $< -o $@

# This line ensures that generated moc_*.cpp files are
# deleted when we make clean.
CLEANFILES = $(filter moc_%.cpp,$(phyview_SOURCES))

