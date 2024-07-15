// SPDX-FileCopyrightText: The Bio++ Development Group
//
// SPDX-License-Identifier: CECILL-2.1

#include "TreeSubWindow.h"
#include "PhyView.h"

// From Qt:
#include <QScrollArea>
#include <QMessageBox>

// From bpp-qt:
#include <Bpp/Qt/QtTools.h>

TreeSubWindow::TreeSubWindow(
    PhyView* phyview,
    std::shared_ptr<TreeDocument> document,
    const TreeDrawing& td) :
  phyview_(phyview),
  treeDocument_(document),
  treeCanvas_()
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowFilePath(QtTools::toQt(treeDocument_->getFilePath()));
  treeDocument_->addView(this);
  treeCanvas_ = new TreeCanvas();
  treeCanvas_->setTree(treeDocument_->getTree());
  treeCanvas_->setTreeDrawing(td);
  treeCanvas_->setMinimumSize(400, 400);
  treeCanvas_->addMouseListener(phyview_->getMouseActionListener());
  connect(treeCanvas_, &TreeCanvas::drawingChanged, phyview, &PhyView::clearSearchResults);

  nodeEditor_ = new QTableWidget();
  nodeEditor_->setColumnCount(3);
  connect(nodeEditor_, &QTableWidget::itemChanged, this, &TreeSubWindow::nodeEditorHasChanged);
  QStringList labels;
  labels.append(tr("Id"));
  labels.append(tr("Name"));
  labels.append(tr("Branch length"));
  nodeEditor_->setHorizontalHeaderLabels(labels);
  splitter_ = new QSplitter(this);
  splitter_->addWidget(treeCanvas_);
  splitter_->addWidget(nodeEditor_);
  splitter_->setCollapsible(0, true);
  splitter_->setCollapsible(1, true);
  // Move the splitter to the right:
  QList<int> currentSizes = splitter_->sizes();
  currentSizes[0] = currentSizes[0] + currentSizes[1];
  currentSizes[1] = 0;
  splitter_->setSizes(currentSizes);

  setMinimumSize(400, 400);
  setWidget(splitter_);
  updateTable();
}

TreeSubWindow::~TreeSubWindow()
{
  delete splitter_;
  phyview_->checkLastWindow();
}

QTableWidgetItem* TreeSubWindow::getTableWigetItem_(Clonable* property)
{
  QTableWidgetItem* propItem = 0;
  BppString* str = dynamic_cast<BppString*>(property);
  if (str)
  {
    propItem = new QTableWidgetItem();
    propItem->setText(QtTools::toQt(*str));
  }
  else
  {
    Number<double>* num = dynamic_cast<Number<double>*>(property);
    if (num)
    {
      propItem = new QTableWidgetItem();
      propItem->setText(QtTools::toQt(*num));
    }
  }
  return propItem;
}

void TreeSubWindow::updateTable()
{
  stopSignal_ = true;
  nodes_ = treeDocument_->getTree()->getNodes();
  nodeEditor_->clearContents();
  nodeEditor_->setRowCount(nodes_.size());

  vector<string> nodeProperties;
  TreeTemplateTools::getNodePropertyNames(*treeDocument_->getTree()->getRootNode(), nodeProperties);
  vector<string> branchProperties;
  TreeTemplateTools::getBranchPropertyNames(*treeDocument_->getTree()->getRootNode(), branchProperties);
  QStringList labels;
  labels.append(tr("Id"));
  labels.append(tr("Name"));
  labels.append(tr("Branch length"));
  for (size_t i = 0; i < nodeProperties.size(); ++i)
  {
    labels.append(QtTools::toQt(nodeProperties[i]));
  }
  for (size_t i = 0; i < branchProperties.size(); ++i)
  {
    labels.append(QtTools::toQt(branchProperties[i]));
  }
  nodeEditor_->setColumnCount(labels.size());
  nodeEditor_->setHorizontalHeaderLabels(labels);

  for (size_t i = 0; i < nodes_.size(); ++i)
  {
    QTableWidgetItem* idItem = new QTableWidgetItem(QtTools::toQt(TextTools::toString(nodes_[i]->getId())));
    Qt::ItemFlags flags = idItem->flags();
    flags &= ~Qt::ItemIsEditable;
    idItem->setFlags(flags);
    nodeEditor_->setItem(i, 0, idItem);

    QTableWidgetItem* nameItem = new QTableWidgetItem();
    if (nodes_[i]->hasName())
      nameItem->setText(QtTools::toQt(nodes_[i]->getName()));
    nodeEditor_->setItem(i, 1, nameItem);

    QTableWidgetItem* brlenItem = new QTableWidgetItem();
    if (nodes_[i]->hasDistanceToFather())
      brlenItem->setText(QtTools::toQt(TextTools::toString(nodes_[i]->getDistanceToFather())));
    nodeEditor_->setItem(i, 2, brlenItem);

    for (size_t j = 0; j < nodeProperties.size(); ++j)
    {
      QTableWidgetItem* item = 0;
      if (nodes_[i]->hasNodeProperty(nodeProperties[j]))
      {
        item = getTableWigetItem_(nodes_[i]->getNodeProperty(nodeProperties[j]));
      }
      else
      {
        item = new QTableWidgetItem();
      }
      nodeEditor_->setItem(i, 3 + j, item);
    }

    for (size_t j = 0; j < branchProperties.size(); ++j)
    {
      QTableWidgetItem* item = 0;
      if (nodes_[i]->hasBranchProperty(branchProperties[j]))
      {
        item = getTableWigetItem_(nodes_[i]->getBranchProperty(branchProperties[j]));
      }
      else
      {
        item = new QTableWidgetItem();
      }
      nodeEditor_->setItem(i, 3 + nodeProperties.size() + j, item);
    }
  }
  stopSignal_ = false;
}

void TreeSubWindow::writeTableToFile(const string& file, const string& sep)
{
  ofstream out(file.c_str(), ios::out);
  for (int j = 0; j < nodeEditor_->columnCount(); ++j)
  {
    QTableWidgetItem* hitem = nodeEditor_->horizontalHeaderItem(j);
    out << (j > 0 ? sep : "") << (hitem ? hitem->text().toStdString() : "");
  }
  out << endl;
  for (int i = 0; i < nodeEditor_->rowCount(); ++i)
  {
    for (int j = 0; j < nodeEditor_->columnCount(); ++j)
    {
      QTableWidgetItem* item = nodeEditor_->item(i, j);
      out << (j > 0 ? sep : "") << (item ? item->text().toStdString() : 0);
    }
    out << endl;
  }
  out.close();
}

void TreeSubWindow::nodeEditorHasChanged(QTableWidgetItem* item)
{
  if (stopSignal_)
    return;
  if (item->column() == 1)
  {
    // Change name:
    phyview_->submitCommand(new ChangeNodeNameCommand(treeDocument_, nodes_[item->row()]->getId(), item->text().toStdString()));
  }
  else if (item->column() == 2)
  {
    // Change branch length:
    phyview_->submitCommand(new ChangeBranchLengthCommand(treeDocument_, nodes_[item->row()]->getId(), item->text().toDouble()));
  }
  else
  {
    // Change node property:
    nodes_[item->row()]->setNodeProperty(nodeEditor_->horizontalHeaderItem(item->column())->text().toStdString(), BppString(item->text().toStdString()));
  }
  treeCanvas_->setTree(treeDocument_->getTree());
}

void TreeSubWindow::duplicateDownSelection(unsigned int rep)
{
  QList<QTableWidgetSelectionRange> selection = nodeEditor_->selectedRanges();
  if (selection.size() == 0)
  {
    QMessageBox::critical(phyview_, QString("Oups..."), QString("No selection."));
    return;
  }
  // Perform some checking:
  int row = -1;
  for (int i = 0; i < selection.size(); ++i)
  {
    QTableWidgetSelectionRange range = selection[i];
    if (range.rowCount() != 1)
    {
      QMessageBox::critical(phyview_, QString("Oups..."), QString("Only one row can be selected."));
      return;
    }
    if (i == 0)
    {
      row = range.topRow();
    }
    else
    {
      if (range.topRow() != row)
      {
        QMessageBox::critical(phyview_, QString("Oups..."), QString("Only one row can be selected."));
        return;
      }
    }
  }
  // Ok, if we reach this stage, then everything is ok...
  int j;
  for (j = row + 1; j < nodeEditor_->rowCount() && j - row <= static_cast<int>(rep); ++j)
  {
    for (int i = 0; i < selection.size(); ++i)
    {
      QTableWidgetSelectionRange range = selection[i];
      for (int k = range.leftColumn(); k <= range.rightColumn(); ++k)
      {
        nodeEditor_->setItem(j, k, nodeEditor_->item(row, k)->clone());
      }
    }
  }
  // Shift selection:
  for (int i = 0; i < selection.size(); ++i)
  {
    QTableWidgetSelectionRange range = selection[i];
    nodeEditor_->setRangeSelected(range, false);
    nodeEditor_->setRangeSelected(QTableWidgetSelectionRange(j - 1, range.leftColumn(), j - 1, range.rightColumn()), true);
  }
}
