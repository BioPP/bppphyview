// SPDX-FileCopyrightText: The Bio++ Development Group
//
// SPDX-License-Identifier: CECILL-2.1

#ifndef _TREESUBWINDOW_H_
#define _TREESUBWINDOW_H_

#include "TreeDocument.h"

// From Qt:
#include <QMdiSubWindow>
#include <QSplitter>
#include <QTableWidget>

// From bpp-phyl:
#include <Bpp/Phyl/Graphics/TreeDrawing.h>

// From bpp-qt
#include <Bpp/Qt/Tree/TreeCanvas.h>

using namespace bpp;

class PhyView;

class TreeSubWindow :
  public QMdiSubWindow,
  public DocumentView
{
  Q_OBJECT

private:
  PhyView* phyview_;
  std::shared_ptr<TreeDocument> treeDocument_;
  TreeCanvas* treeCanvas_;
  QSplitter* splitter_;
  QTableWidget* nodeEditor_;
  std::vector<Node*> nodes_;
  bool stopSignal_;

public:
  TreeSubWindow(
      PhyView* phyview,
      std::shared_ptr<TreeDocument> document,
      const TreeDrawing& td);

  virtual ~TreeSubWindow();

public:
  std::shared_ptr<TreeDocument> getDocument() { return treeDocument_; }
  const TreeTemplate<Node>& tree() const { return treeDocument_->tree(); }

  const TreeCanvas* getTreeCanvas() const { return treeCanvas_; }
  TreeCanvas* getTreeCanvas() { return treeCanvas_; }

  const TreeCanvas& treeCanvas() const { return *treeCanvas_; }
  TreeCanvas& treeCanvas() { return *treeCanvas_; }

  void duplicateDownSelection(unsigned int rep);

  void updateView()
  {
    treeCanvas_->setTree(treeDocument_->getTree());
    updateTable();
  }

  void updateTable();

  void writeTableToFile(const string& file, const string& sep);

private:
  QTableWidgetItem* getTableWigetItem_(Clonable* property);

private slots:
  void nodeEditorHasChanged(QTableWidgetItem* item);
};

#endif // _TREESUBWINDOW_H_
