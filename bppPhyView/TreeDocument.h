// SPDX-FileCopyrightText: The Bio++ Development Group
//
// SPDX-License-Identifier: CECILL-2.1

#ifndef _TREEDOCUMENT_H_
#define _TREEDOCUMENT_H_

#include <Bpp/Io/FileTools.h>

// From bpp-phyl:
#include <Bpp/Phyl/Tree/Tree.h>
#include <Bpp/Phyl/Tree/TreeTemplate.h>

// From the STL:
#include <string>

// From Qt:
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
  std::shared_ptr<TreeTemplate<Node>> tree_;
  std::string documentName_;
  bool modified_;
  std::string currentFilePath_;
  std::string currentFileFormat_;
  QUndoStack undoStack_;
  vector<DocumentView*> viewers_;

public:
  TreeDocument() :
    tree_(),
    documentName_(),
    modified_(false),
    currentFilePath_(),
    currentFileFormat_(),
    undoStack_()
  {}

  virtual ~TreeDocument() = default;

public:
  bool hasTree() const {
    if (tree_) return true;
    else       return false;
  }

  std::shared_ptr<const TreeTemplate<Node>> getTree() const {
    return tree_;
  }
  
  std::shared_ptr<TreeTemplate<Node>> getTree() {
    return tree_;
  }
  
  const TreeTemplate<Node>& tree() const { 
    if (hasTree()) {
      return *tree_;
    } else {
      throw Exception("TreeDocument::tree(). No tree associated to document.");
    }
  }

  TreeTemplate<Node>& tree() { 
    if (hasTree()) {
      return *tree_;
    } else {
      throw Exception("TreeDocument::tree(). No tree associated to document.");
    }
  }

  void setTree(const Tree& tree)
  {
    tree_.reset(new TreeTemplate<Node>(tree));
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
    {
      viewers_[i]->updateView();
    }
  }
};

#endif // _TREEDOCUMENT_H_
