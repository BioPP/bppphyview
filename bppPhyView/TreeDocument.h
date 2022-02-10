//
// File: TreeDocument.h
// Created by: Julien Dutheil
// Created on: Tue Oct 5 22:05 2006
//

/*
Copyright or © or Copr. Bio++ Development Team, (November 16, 2004)

This software is a computer program whose purpose is to provide classes
for phylogenetic data analysis.

This software is governed by the CeCILL  license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.
*/

#ifndef _TREEDOCUMENT_H_
#define _TREEDOCUMENT_H_

#include <Bpp/Io/FileTools.h>

//From bpp-phyl:
#include <Bpp/Phyl/Tree/Tree.h>
#include <Bpp/Phyl/Tree/TreeTemplate.h>

//From the STL:
#include <string>

//From Qt:
#include <QUndoStack>

using namespace bpp;
using namespace std;

/**
 * @brief Interface for document viewers.
 */
class DocumentView
{
  public:
    virtual ~DocumentView() {}

  public:
    virtual void updateView() = 0;
};

/**
 * Contains a tree and all associated data, if any.
 * Also contains a path where to write, and a format,
 * which are use for actions like "save", "save as", "save a copy".
 */
class TreeDocument
{
  private:
    TreeTemplate<Node>* tree_;
    std::string documentName_;
    bool modified_;
    std::string currentFilePath_;
    std::string currentFileFormat_;
    QUndoStack undoStack_;
    vector<DocumentView*> viewers_;

  public:
    TreeDocument():
      tree_(0),
      documentName_(),
      modified_(false),
      currentFilePath_(),
      currentFileFormat_(),
      undoStack_()
    {}

    virtual ~TreeDocument()
    {
      if (tree_) delete tree_;
    }
    
  public:
    const TreeTemplate<Node>* getTree() const { return tree_; }
    
    TreeTemplate<Node>* getTree() { return tree_; }
    
    void setTree(const Tree& tree)
    {
      if (tree_) delete tree_;
      tree_ = new TreeTemplate<Node>(tree);
    }

    const std::string& getName() const { return documentName_; }

    void setFile(const string& filePath, const string& fileFormat)
    {
      currentFilePath_   = filePath;
      currentFileFormat_ = fileFormat;
      documentName_      = FileTools::getFileName(filePath);
    }
    
    const string& getFilePath() const { return currentFilePath_; }
    const string& getFileFormat() const { return currentFileFormat_; }
      
    void modified(bool yn) { modified_ = yn; }
    bool modified() const { return modified_; }

    QUndoStack& getUndoStack() { return undoStack_; }

    void addView(DocumentView* viewer)
    {
      viewers_.push_back(viewer);
    }

    void updateAllViews()
    {
      for (size_t i = 0; i < viewers_.size(); i++)
        viewers_[i]->updateView();
    }
};

#endif //_TREEDOCUMENT_H_

