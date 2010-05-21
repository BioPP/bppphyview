//
// File: TreeSubWindow.cpp
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

#include "TreeSubWindow.h"
#include "PhyView.h"

//From Qt:
#include <QScrollArea>

//From bpp-qt:
#include <Bpp/Qt/QtTools.h>

TreeSubWindow::TreeSubWindow(PhyView* phyview, TreeDocument* document, TreeDrawing* td):
  phyview_(phyview), treeDocument_(document), treeCanvas_()
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowFilePath(QtTools::toQt(treeDocument_->getFilePath()));
  treeDocument_->addView(this);
  treeCanvas_ = new TreeCanvas();
  treeCanvas_->setTree(treeDocument_->getTree());
  treeCanvas_->setTreeDrawing(*td);
  treeCanvas_->setMinimumSize(400,400);
  treeCanvas_->addMouseListener(reinterpret_cast<MouseListener*>(phyview_->getMouseActionListener()));
  
  nodeEditor_ = new QTableWidget();
  nodeEditor_->setColumnCount(3);
  connect(nodeEditor_, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(nodeEditorHasChanged(QTableWidgetItem*)));
  QStringList labels;
  labels.append(tr("Id"));
  labels.append(tr("Name"));
  labels.append(tr("Branch length"));
  nodeEditor_->setHorizontalHeaderLabels (labels);
  splitter_ = new QSplitter(this);
  splitter_->addWidget(treeCanvas_);
  splitter_->addWidget(nodeEditor_);

  setMinimumSize(400, 400);
  setWidget(splitter_);
  updateTable();
}

void TreeSubWindow::updateTable()
{
  stopSignal_ = true;
  nodes_ = treeDocument_->getTree()->getNodes();
  nodeEditor_->clearContents();
  nodeEditor_->setRowCount(nodes_.size());

  for (unsigned int i = 0; i < nodes_.size(); ++i) {
    QTableWidgetItem* idItem = new QTableWidgetItem(QtTools::toQt(TextTools::toString(nodes_[i]->getId())));
    idItem->setFlags(!Qt::ItemIsEditable);
    nodeEditor_->setItem(i, 0, idItem);

    QTableWidgetItem* nameItem = new QTableWidgetItem();
    if (nodes_[i]->hasName()) nameItem->setText(QtTools::toQt(nodes_[i]->getName()));
    nodeEditor_->setItem(i, 1, nameItem);

    QTableWidgetItem* brlenItem = new QTableWidgetItem();
    if (nodes_[i]->hasDistanceToFather()) brlenItem->setText(QtTools::toQt(TextTools::toString(nodes_[i]->getDistanceToFather())));
    nodeEditor_->setItem(i, 2, brlenItem);
  }
  stopSignal_ = false;
}

void TreeSubWindow::nodeEditorHasChanged(QTableWidgetItem* item)
{
  if (stopSignal_) return;
  if (item->column() == 1) {
    //Change name:
    phyview_->submitCommand(new ChangeNodeNameCommand(treeDocument_, nodes_[item->row()]->getId(), item->text().toStdString()));
  } else if(item->column() == 2) {
    //Change branch length:
    phyview_->submitCommand(new ChangeBranchLengthCommand(treeDocument_, nodes_[item->row()]->getId(), item->text().toDouble()));
  }
  treeCanvas_->setTree(treeDocument_->getTree());
}

