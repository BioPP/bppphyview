// SPDX-FileCopyrightText: The Bio++ Development Group
//
// SPDX-License-Identifier: CECILL-2.1

#ifndef _TREESUBWINDOW_H_
#define _TREESUBWINDOW_H_

#include "TreeDocument.h"

//From Qt:
#include <QMdiSubWindow>
#include <QSplitter>
#include <QTableWidget>

//From PhylLib:
#include <Bpp/Phyl/Graphics/TreeDrawing.h>

//From Bpp-Qt
#include <Bpp/Qt/Tree/TreeCanvas.h>

using namespace bpp;

class PhyView;

class TreeSubWindow:
  public QMdiSubWindow,
  public DocumentView
{
  Q_OBJECT

  private:
    PhyView* phyview_;
    TreeDocument* treeDocument_;
    TreeCanvas* treeCanvas_;
    QSplitter* splitter_;
    QTableWidget* nodeEditor_;
    std::vector<Node*> nodes_;
    bool stopSignal_;

  public:
    TreeSubWindow(PhyView* phyview, TreeDocument* document, TreeDrawing* td);

    virtual ~TreeSubWindow();

  public:
    TreeDocument* getDocument() { return treeDocument_; }
    const Tree& getTree() const { return *treeDocument_->getTree(); }
    const TreeCanvas& getTreeCanvas() const { return *treeCanvas_; }
    TreeCanvas& getTreeCanvas() { return *treeCanvas_; }

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

#endif //_TREESUBWINDOW_H_

