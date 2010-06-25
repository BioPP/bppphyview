//
// File: TreeSubWindow.h
// Created by: Julien Dutheil
// Created on: Tue Aug 11 13:34 2009
//

/*
Copyright or © or Copr. CNRS, (November 16, 2004)

This software is a computer program whose purpose is to provide
graphic components to develop bioinformatics applications.

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

#ifndef _TREESUBWINDOW_H_
#define _TREESUBWINDOW_H_

#include "TreeDocument.h"

//From Qt:
#include <QMdiSubWindow>
#include <QSplitter>
#include <QTableWidget>

//From PhylLib:
#include <Phyl/TreeDrawing.h>

//From Bpp-Qt
#include <Bpp/Qt/TreeCanvas.h>

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

    virtual ~TreeSubWindow()
    {
      delete treeDocument_;
      delete splitter_;
    }

  public:
    TreeDocument* getDocument() { return treeDocument_; }
    const Tree& getTree() const { return *treeDocument_->getTree(); }
    const TreeCanvas& getTreeCanvas() const { return *treeCanvas_; }
    TreeCanvas& getTreeCanvas() { return *treeCanvas_; }

    void updateView()
    {
      treeCanvas_->setTree(treeDocument_->getTree());
      updateTable();
    }
    
    void updateTable();

  private:
    QTableWidgetItem* getTableWigetItem_(Clonable* property);

  private slots:
    void nodeEditorHasChanged(QTableWidgetItem* item);

};

#endif //_TREESUBWINDOW_H_

