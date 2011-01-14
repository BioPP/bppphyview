//
// File: PhyView.cpp
// Created by: Julien Dutheil
// Created on: Tue Aug 05 14:59 2009
//

/*
Copyright or © or Copr. Bio++ Development Team, (November 16, 2004)

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

#include "PhyView.h"
#include "TreeSubWindow.h"
#include "TreeDocument.h"

#include <QApplication>
#include <QtGui>

#include <Bpp/Qt/QtGraphicDevice.h>

#include <Bpp/Numeric/DataTable.h>

#include <Bpp/Phyl/Tree.h>
#include <Bpp/Phyl/Io.all>
#include <Bpp/Phyl/Graphics/PhylogramPlot.h>

#include <fstream>

using namespace std;    
using namespace bpp;

MouseActionListener::MouseActionListener(PhyView* phyview):
  phyview_(phyview),
  treeChooser_(new QDialog()),
  treeList_(new QListWidget(treeChooser_))
{
  treeChooser_->setParent(phyview_);
  treeChooser_->setModal(true);
  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(treeList_);
  layout->addStretch(1);
  treeChooser_->setLayout(layout);
  treeChooser_->connect(treeList_, SIGNAL(itemClicked(QListWidgetItem*)), treeChooser_, SLOT(accept()));
}



TranslateNameChooser::TranslateNameChooser(PhyView* phyview) :
  QDialog(phyview), phyview_(phyview), fileDialog_(new QFileDialog(this))
{
  fileFilters_ << "Coma separated columns (*.txt *.csv)"
               << "Tab separated columns (*.txt *.csv)";
  fileDialog_->setNameFilters(fileFilters_);

  QFormLayout* layout = new QFormLayout;
  fromList_ = new QComboBox;
  toList_   = new QComboBox;
  ok_       = new QPushButton(tr("Ok"));
  cancel_   = new QPushButton(tr("Cancel"));
  layout->addRow(tr("From"), fromList_);
  layout->addRow(tr("To")  , toList_);
  layout->addRow(cancel_, ok_);
  connect(ok_, SIGNAL(clicked(bool)), this, SLOT(accept()));
  connect(cancel_, SIGNAL(clicked(bool)), this, SLOT(reject()));
  setLayout(layout);
}

void TranslateNameChooser::translateTree(TreeTemplate<Node>& tree)
{
  fileDialog_->setAcceptMode(QFileDialog::AcceptOpen);
  if (fileDialog_->exec() == QDialog::Accepted) {
    QStringList path = fileDialog_->selectedFiles();
    string sep = ",";
    if (fileDialog_->selectedNameFilter() == fileFilters_[1])
      sep = "\t";
    ifstream file(path[0].toStdString().c_str(), ios::in);
    DataTable* table = DataTable::read(file, sep);

    //Clean button groups:
    fromList_->clear();
    toList_->clear();

    //Now add the new ones:
    for (unsigned int i = 0; i < table->getNumberOfColumns(); ++i) {
      fromList_->addItem(QtTools::toQt(table->getColumnName(i)));
        toList_->addItem(QtTools::toQt(table->getColumnName(i)));
    }
    if (exec() == QDialog::Accepted)
      phyview_->submitCommand(new TranslateNodeNamesCommand(phyview_->getActiveDocument(), *table, fromList_->currentIndex(),  toList_->currentIndex()));
  }
}




DataLoader::DataLoader(PhyView* phyview) :
  QDialog(phyview), phyview_(phyview)
{
  QFormLayout* layout = new QFormLayout;
  idIndex_   = new QRadioButton(tr("Index from id"));
  idIndex_->setChecked(true);
  nameIndex_ = new QRadioButton(tr("Index from name"));
  indexCol_  = new QComboBox;
  QButtonGroup* bg = new QButtonGroup();
  bg->addButton(idIndex_);
  bg->addButton(nameIndex_);
  ok_       = new QPushButton(tr("Ok"));
  cancel_   = new QPushButton(tr("Cancel"));
  layout->addRow(idIndex_, nameIndex_);
  layout->addRow(tr("Column"), indexCol_);
  layout->addRow(cancel_, ok_);
  connect(ok_, SIGNAL(clicked(bool)), this, SLOT(accept()));
  connect(cancel_, SIGNAL(clicked(bool)), this, SLOT(reject()));
  setLayout(layout);
}

void DataLoader::load(const DataTable* data)
{
  indexCol_->clear();
  for (unsigned int i = 0; i < data->getNumberOfColumns(); ++i)
    indexCol_->addItem(QtTools::toQt(data->getColumnName(i)));
  if(exec() == QDialog::Accepted) {
    unsigned int index = static_cast<unsigned int>(indexCol_->currentIndex());
    phyview_->submitCommand(new AttachDataCommand(phyview_->getActiveDocument(), *data, index, nameIndex_->isChecked()));
  }
}

TypeNumberDialog::TypeNumberDialog(PhyView* phyview, const string& what, unsigned int min, unsigned int max) :
  QDialog(phyview)
{
  QFormLayout* layout = new QFormLayout;
  spinBox_  = new QSpinBox;
  spinBox_->setRange(min, max);
  ok_       = new QPushButton(tr("Ok"));
  cancel_   = new QPushButton(tr("Cancel"));
  layout->addRow(QtTools::toQt(what), spinBox_);
  layout->addRow(cancel_, ok_);
  connect(ok_, SIGNAL(clicked(bool)), this, SLOT(accept()));
  connect(cancel_, SIGNAL(clicked(bool)), this, SLOT(reject()));
  setLayout(layout);
}



void MouseActionListener::mousePressEvent(QMouseEvent *event)
{
  if (dynamic_cast<NodeMouseEvent*>(event)->hasNodeId())
  {
    int nodeId = dynamic_cast<NodeMouseEvent*>(event)->getNodeId();
    QString action;
    if (event->button() == Qt::LeftButton)
      action = phyview_->getMouseLeftButtonActionType();
    else if (event->button() == Qt::MidButton)
      action = phyview_->getMouseMiddleButtonActionType();
    else if (event->button() == Qt::RightButton)
      action = phyview_->getMouseRightButtonActionType();
    else
      action = "None";

    if (action == "Swap")
    {
      if (!phyview_->getActiveDocument()->getTree()->isRoot(nodeId))
      {
        int fatherId = phyview_->getActiveDocument()->getTree()->getFatherId(nodeId);
        vector<int> sonsId = phyview_->getActiveDocument()->getTree()->getSonsId(fatherId);
        unsigned int i1 = 0, i2 = 0;
        if (sonsId[0] == nodeId) {
          i1 = 0;
          i2 = sonsId.size() - 1;
        } else {
          for (unsigned int i = 1; i < sonsId.size(); ++i)
            if (sonsId[i] == nodeId) {
              i1 = i;
              i2 = i - 1;
            }
        }
        phyview_->submitCommand(new SwapCommand(phyview_->getActiveDocument(), fatherId, i1, i2 , nodeId, sonsId[i2]));
      }
    }
    else if (action == "Order down")
    {
      phyview_->submitCommand(new OrderCommand(phyview_->getActiveDocument(), nodeId, true));
    }
    else if (action == "Order up")
    {
      phyview_->submitCommand(new OrderCommand(phyview_->getActiveDocument(), nodeId, false));
    }
    else if (action == "Root on node")
      phyview_->submitCommand(new RerootCommand(phyview_->getActiveDocument(), nodeId));
    else if (action == "Root on branch")
      phyview_->submitCommand(new OutgroupCommand(phyview_->getActiveDocument(), nodeId));
    else if (action == "Collapse") {
      TreeCanvas& tc = phyview_->getActiveSubWindow()->getTreeCanvas();
      tc.collapseNode(nodeId, !tc.isNodeCollapsed(nodeId));
      tc.redraw();
    }
    else if (action == "Sample subtree") {
      Node* n = phyview_->getActiveDocument()->getTree()->getNode(nodeId);
      TypeNumberDialog dial(phyview_, "Sample size", 1u, TreeTemplateTools::getNumberOfLeaves(*n));
      if (dial.exec() == QDialog::Accepted) {
        unsigned int size = dial.getValue();
        phyview_->submitCommand(new SampleSubtreeCommand(phyview_->getActiveDocument(), nodeId, size));
      }
    } else if (action == "Delete subtree") {
      phyview_->submitCommand(new DeleteSubtreeCommand(phyview_->getActiveDocument(), nodeId));
    }
    else if (action == "Copy subtree") {
      Node* subtree = TreeTemplateTools::cloneSubtree<Node>(*phyview_->getActiveDocument()->getTree()->getNode(nodeId));
      auto_ptr< TreeTemplate<Node> > tt(new TreeTemplate<Node>(subtree));
      phyview_->createNewDocument(tt.get());
    }
    else if (action == "Cut subtree") {
      Node* subtree = TreeTemplateTools::cloneSubtree<Node>(*phyview_->getActiveDocument()->getTree()->getNode(nodeId));
      auto_ptr< TreeTemplate<Node> > tt(new TreeTemplate<Node>(subtree));
      phyview_->submitCommand(new DeleteSubtreeCommand(phyview_->getActiveDocument(), nodeId));
      phyview_->createNewDocument(tt.get());
    }
    else if (action == "Insert on node") {
      if (phyview_->getNonActiveDocuments().size() == 0) {
        QMessageBox::critical(phyview_, QString("Oups..."), QString("No tree to insert."));
        return;
      }
      TreeTemplate<Node>* tree = pickTree_();
      if (tree) {
        Node* subtree = TreeTemplateTools::cloneSubtree<Node>(*tree->getRootNode());
        phyview_->submitCommand(new InsertSubtreeAtNodeCommand(phyview_->getActiveDocument(), nodeId, subtree));
      }
    }
    else if (action == "Insert on branch") {
      if (phyview_->getNonActiveDocuments().size() == 0) {
        QMessageBox::critical(phyview_, QString("Oups..."), QString("No tree to insert."));
        return;
      }
      TreeTemplate<Node>* tree = pickTree_();
      if (tree) {
        Node* subtree = TreeTemplateTools::cloneSubtree<Node>(*tree->getRootNode());
        phyview_->submitCommand(new InsertSubtreeOnBranchCommand(phyview_->getActiveDocument(), nodeId, subtree));
      }
    }
  }
}



TreeTemplate<Node>* MouseActionListener::pickTree_()
{
  QList<TreeDocument*> documents = phyview_->getNonActiveDocuments();
  treeList_->clear();
  for (int i = 0; i < documents.size(); ++i) {
    QString text = QtTools::toQt(documents[i]->getName());
    if (text == "") text = "(unknown)";
    vector<string> leaves = documents[i]->getTree()->getLeavesNames(); 
    text += QtTools::toQt(" " + TextTools::toString(leaves.size()) + " leaves ");
    for (unsigned int j = 0; j < min(static_cast<unsigned int>(leaves.size()), 5u); ++j) {
      text += QtTools::toQt(", " + leaves[j]);
    }
    if (leaves.size() >= 5) text += "...";
    treeList_->addItem(text);
  }
  treeChooser_->exec();
  return documents[treeList_->currentRow()]->getTree();
}




PhyView::PhyView():
  manager_(),
  collapsedNodesListener_(true)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setAttribute(Qt::WA_QuitOnClose);
  initGui_();
  createActions_();
  createMenus_();
  createStatusBar_();
  resize(1000, 600);
}



void PhyView::initGui_()
{
  mdiArea_ = new QMdiArea;
  connect(mdiArea_, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(setCurrentSubWindow(QMdiSubWindow*)));
  setCentralWidget(mdiArea_);
  
  //Stats panel:
  createStatsPanel_();
  statsDockWidget_ = new QDockWidget(tr("Statistics"));
  statsDockWidget_->setWidget(statsPanel_);
  statsDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  addDockWidget(Qt::RightDockWidgetArea, statsDockWidget_);

  //Display panel:
  createDisplayPanel_();
  displayDockWidget_ = new QDockWidget(tr("Display"));
  displayDockWidget_->setWidget(displayPanel_);
  displayDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  addDockWidget(Qt::RightDockWidgetArea, displayDockWidget_);

  //Undo panel:
  QUndoView* undoView = new QUndoView;
  undoView->setGroup(&manager_);
  undoDockWidget_ = new QDockWidget(tr("Undo list"));
  undoDockWidget_->setWidget(undoView);
  undoDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  addDockWidget(Qt::LeftDockWidgetArea, undoDockWidget_);

  //Branch lengths panel:
  createBrlenPanel_();
  brlenDockWidget_ = new QDockWidget(tr("Branch lengths"));
  brlenDockWidget_->setWidget(brlenPanel_);
  brlenDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  addDockWidget(Qt::LeftDockWidgetArea, brlenDockWidget_);

  //Mouse control panel:
  createMouseControlPanel_();
  mouseControlDockWidget_ = new QDockWidget(tr("Mouse control"));
  mouseControlDockWidget_->setWidget(mouseControlPanel_);
  mouseControlDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  addDockWidget(Qt::LeftDockWidgetArea, mouseControlDockWidget_);

  //Names operations panel:
  createDataPanel_();
  dataDockWidget_ = new QDockWidget(tr("Associated Data"));
  dataDockWidget_->setWidget(dataPanel_);
  dataDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  addDockWidget(Qt::LeftDockWidgetArea, dataDockWidget_);
  dataDockWidget_->setVisible(false);
  
  //Other stuff...
  treeFileDialog_ = new QFileDialog(this, "Tree File");
  treeFileFilters_ << "Newick files (*.dnd *.tre *.tree *.nwk *.newick *.phy *.txt)"
                   << "Nexus files (*.nx *.nex *.nexus)"
                   << "Nhx files (*.nhx)";
  treeFileDialog_->setNameFilters(treeFileFilters_);
  treeFileDialog_->setConfirmOverwrite(true);

  dataFileDialog_ = new QFileDialog(this, "Data File");
  dataFileFilters_ << "Coma separated columns (*.txt *.csv)"
                   << "Tab separated columns (*.txt *.csv)";
  dataFileDialog_->setNameFilters(dataFileFilters_);

  translateNameChooser_ = new TranslateNameChooser(this);

  dataLoader_ = new DataLoader(this);
}

void PhyView::createDisplayPanel_()
{
  displayPanel_ = new QWidget(this);
  treeControlers_ = new TreeCanvasControlers();
  treeControlers_->addActionListener(this);
  for (unsigned int i = 0; i < treeControlers_->getNumberOfTreeDrawings(); ++i)
    treeControlers_->getTreeDrawing(i)->addTreeDrawingListener(&collapsedNodesListener_);
  
  QGroupBox* drawingOptions = new QGroupBox(tr("Drawing"));
  QFormLayout* drawingLayout = new QFormLayout;
  drawingLayout->addRow(tr("&Type:"),        treeControlers_->getControlerById(TreeCanvasControlers::ID_DRAWING_CTRL));
  drawingLayout->addRow(tr("&Orientation:"), treeControlers_->getControlerById(TreeCanvasControlers::ID_ORIENTATION_CTRL));
  drawingLayout->addRow(tr("Width (px):"),   treeControlers_->getControlerById(TreeCanvasControlers::ID_WIDTH_CTRL));
  drawingLayout->addRow(tr("&Height (px):"), treeControlers_->getControlerById(TreeCanvasControlers::ID_HEIGHT_CTRL));
  drawingOptions->setLayout(drawingLayout);

  QGroupBox* displayOptions = new QGroupBox(tr("Display"));
  QVBoxLayout* displayLayout = new QVBoxLayout;
  displayLayout->addWidget(treeControlers_->getControlerById(TreeCanvasControlers::ID_DRAW_NODE_IDS_CTRL));
  displayLayout->addWidget(treeControlers_->getControlerById(TreeCanvasControlers::ID_DRAW_LEAF_NAMES_CTRL));
  displayLayout->addWidget(treeControlers_->getControlerById(TreeCanvasControlers::ID_DRAW_BRANCH_LENGTHS_CTRL));
  displayLayout->addWidget(treeControlers_->getControlerById(TreeCanvasControlers::ID_DRAW_BOOTSTRAP_VALUES_CTRL));
  displayLayout->addWidget(treeControlers_->getControlerById(TreeCanvasControlers::ID_DRAW_CLICKABLE_AREAS_CTRL));
  displayOptions->setLayout(displayLayout);
  
  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(drawingOptions);
  layout->addWidget(displayOptions);
  layout->addStretch(1);
  displayPanel_->setLayout(layout);
}

void PhyView::createStatsPanel_()
{
  statsPanel_ = new QWidget(this);
  QVBoxLayout* statsLayout = new QVBoxLayout;
  statsBox_ = new TreeStatisticsBox;
  statsLayout->addWidget(statsBox_);
  QPushButton* update = new QPushButton(tr("Update"));
  connect(update, SIGNAL(clicked(bool)), this, SLOT(updateStatistics()));
  statsLayout->addWidget(update);
  statsLayout->addStretch(1);
  statsPanel_->setLayout(statsLayout);
}

void PhyView::createBrlenPanel_()
{
  brlenPanel_ = new QWidget(this);
  QVBoxLayout* brlenLayout = new QVBoxLayout;

  //Set all lengths:
  brlenSetLengths_ = new QDoubleSpinBox;
  brlenSetLengths_->setDecimals(6);
  brlenSetLengths_->setSingleStep(0.01);
  QPushButton* brlenSetLengthsGo = new QPushButton(tr("Go!"));
  connect(brlenSetLengthsGo, SIGNAL(clicked(bool)), this, SLOT(setLengths()));
  
  QGroupBox* brlenSetLengthsBox = new QGroupBox(tr("Set all lengths"));
  QHBoxLayout* brlenSetLengthsBoxLayout = new QHBoxLayout;
  brlenSetLengthsBoxLayout->addWidget(brlenSetLengths_);
  brlenSetLengthsBoxLayout->addWidget(brlenSetLengthsGo);
  brlenSetLengthsBoxLayout->addStretch(1);
  brlenSetLengthsBox->setLayout(brlenSetLengthsBoxLayout);
  
  brlenLayout->addWidget(brlenSetLengthsBox);

  //Grafen method:
  QPushButton* brlenInitGrafen = new QPushButton(tr("Init"));
  connect(brlenInitGrafen, SIGNAL(clicked(bool)), this, SLOT(initLengthsGrafen()));
  
  brlenComputeGrafen_ = new QDoubleSpinBox;
  brlenComputeGrafen_->setValue(1.);
  brlenComputeGrafen_->setDecimals(2);
  brlenComputeGrafen_->setSingleStep(0.1);
  QPushButton* brlenComputeGrafenGo = new QPushButton(tr("Go!"));
  connect(brlenComputeGrafenGo, SIGNAL(clicked(bool)), this, SLOT(computeLengthsGrafen()));

  QGroupBox* brlenGrafenBox = new QGroupBox(tr("Grafen"));
  QHBoxLayout* brlenGrafenBoxLayout = new QHBoxLayout;
  brlenGrafenBoxLayout->addWidget(brlenInitGrafen);
  brlenGrafenBoxLayout->addWidget(brlenComputeGrafen_);
  brlenGrafenBoxLayout->addWidget(brlenComputeGrafenGo);
  brlenGrafenBoxLayout->addStretch(1);
  brlenGrafenBox->setLayout(brlenGrafenBoxLayout);
  
  brlenLayout->addWidget(brlenGrafenBox);

  //To clock tree:
  QPushButton* brlenToClockTree = new QPushButton(tr("Convert to clock"));
  connect(brlenToClockTree, SIGNAL(clicked(bool)), this, SLOT(convertToClockTree()));
  brlenLayout->addWidget(brlenToClockTree);

  //Midpoint rooting:
  QPushButton* brlenMidpointRooting = new QPushButton(tr("Midpoint rooting"));
  connect(brlenMidpointRooting, SIGNAL(clicked(bool)), this, SLOT(midpointRooting()));
  brlenLayout->addWidget(brlenMidpointRooting);

  ////
  brlenLayout->addStretch(1);
  brlenPanel_->setLayout(brlenLayout);
}

void PhyView::createMouseControlPanel_()
{
  mouseControlPanel_ = new QWidget;

  QStringList mouseActions;
  mouseActions.append(tr("None"));
  mouseActions.append(tr("Swap"));
  mouseActions.append(tr("Order down"));
  mouseActions.append(tr("Order up"));
  mouseActions.append(tr("Root on node"));
  mouseActions.append(tr("Root on branch"));
  mouseActions.append(tr("Sample subtree"));
  mouseActions.append(tr("Collapse"));
  mouseActions.append(tr("Delete subtree"));
  mouseActions.append(tr("Copy subtree"));
  mouseActions.append(tr("Cut subtree"));
  mouseActions.append(tr("Insert on node"));
  mouseActions.append(tr("Insert on branch"));

  leftButton_ = new QComboBox;
  leftButton_->addItems(mouseActions);
  middleButton_ = new QComboBox;
  middleButton_->addItems(mouseActions);
  rightButton_ = new QComboBox;
  rightButton_->addItems(mouseActions);

  QFormLayout* formLayout = new QFormLayout;
  formLayout->addRow(tr("Left:"), leftButton_);
  formLayout->addRow(tr("Middle:"), middleButton_);
  formLayout->addRow(tr("Right:"), rightButton_);

  mouseControlPanel_->setLayout(formLayout);
}

void PhyView::createDataPanel_()
{
  dataPanel_ = new QWidget;
  QVBoxLayout* dataLayout = new QVBoxLayout;
  
  loadData_ = new QPushButton(tr("Load Data"));
  connect(loadData_, SIGNAL(clicked(bool)), this, SLOT(attachData()));
  dataLayout->addWidget(loadData_);
  
  translateNames_ = new QPushButton(tr("Translate"));
  connect(translateNames_, SIGNAL(clicked(bool)), this, SLOT(translateNames()));
  dataLayout->addWidget(translateNames_);
  
  dataPanel_->setLayout(dataLayout);
}

void PhyView::createActions_()
{
  openAction_ = new QAction(tr("&Open"), this);
  openAction_->setShortcut(tr("Ctrl+O"));
  openAction_->setStatusTip(tr("Open a new tree file"));
  connect(openAction_, SIGNAL(triggered()), this, SLOT(openTree()));

  saveAction_ = new QAction(tr("&Save"), this);
  saveAction_->setShortcut(tr("Ctrl+S"));
  saveAction_->setStatusTip(tr("Save the current tree to file"));
  connect(saveAction_, SIGNAL(triggered()), this, SLOT(saveTree()));

  saveAsAction_ = new QAction(tr("Save &as"), this);
  saveAsAction_->setShortcut(tr("Ctrl+Shift+S"));
  saveAsAction_->setStatusTip(tr("Save the current tree to a file"));
  connect(saveAsAction_, SIGNAL(triggered()), this, SLOT(saveTreeAs()));

  closeAction_ = new QAction(tr("&Close"), this);
  closeAction_->setShortcut(tr("Ctrl+W"));
  closeAction_->setStatusTip(tr("Close the current tree plot."));
  connect(closeAction_, SIGNAL(triggered()), this, SLOT(closeTree()));

  exitAction_ = new QAction(tr("&Quit"), this);
  exitAction_->setShortcut(tr("Ctrl+Q"));
  exitAction_->setStatusTip(tr("Quit PhyView"));
  connect(exitAction_, SIGNAL(triggered()), this, SLOT(exit()));

  cascadeWinAction_ = new QAction(tr("&Cascade windows"), this);
  connect(cascadeWinAction_, SIGNAL(triggered()), mdiArea_, SLOT(cascadeSubWindows()));

  tileWinAction_ = new QAction(tr("&Tile windows"), this);
  connect(tileWinAction_, SIGNAL(triggered()), mdiArea_, SLOT(tileSubWindows()));
  
  aboutAction_ = new QAction(tr("About"), this);
  connect(aboutAction_, SIGNAL(triggered()), this, SLOT(about()));
  aboutBppAction_ = new QAction(tr("About Bio++"), this);
  connect(aboutBppAction_, SIGNAL(triggered()), this, SLOT(aboutBpp()));
  aboutQtAction_ = new QAction(tr("About Qt"), this);
  connect(aboutQtAction_, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  undoAction_ = manager_.createUndoAction(this);
  redoAction_ = manager_.createRedoAction(this);
  undoAction_->setShortcut(QKeySequence("Ctrl+Z"));
  redoAction_->setShortcut(QKeySequence("Shift+Ctrl+Z"));
}




void PhyView::createMenus_()
{
  fileMenu_ = menuBar()->addMenu(tr("&File"));
  fileMenu_->addAction(openAction_);
  fileMenu_->addAction(saveAction_);
  fileMenu_->addAction(saveAsAction_);
  fileMenu_->addAction(closeAction_);
  fileMenu_->addAction(exitAction_);
  
  editMenu_ = menuBar()->addMenu(tr("&Edit"));
  editMenu_->addAction(undoAction_);
  editMenu_->addAction(redoAction_);
  
  viewMenu_ = menuBar()->addMenu(tr("&View"));
  viewMenu_->addAction(statsDockWidget_->toggleViewAction());
  viewMenu_->addAction(displayDockWidget_->toggleViewAction());
  viewMenu_->addAction(brlenDockWidget_->toggleViewAction());
  viewMenu_->addAction(undoDockWidget_->toggleViewAction());
  viewMenu_->addAction(mouseControlDockWidget_->toggleViewAction());
  viewMenu_->addAction(dataDockWidget_->toggleViewAction());
  viewMenu_->addAction(cascadeWinAction_);
  viewMenu_->addAction(tileWinAction_);
  
  helpMenu_ = menuBar()->addMenu(tr("&Help"));
  helpMenu_->addAction(aboutAction_);
  helpMenu_->addAction(aboutBppAction_);
  helpMenu_->addAction(aboutQtAction_);
}




void PhyView::createStatusBar_()
{
  updateStatusBar();
}


void PhyView::closeEvent(QCloseEvent* event)
{

}



TreeDocument* PhyView::createNewDocument(Tree* tree)
{
  TreeDocument* doc = new TreeDocument();
  doc->setTree(*tree);
  manager_.addStack(&doc->getUndoStack());
  TreeSubWindow *subWindow = new TreeSubWindow(this, doc, treeControlers_->getSelectedTreeDrawing());
  mdiArea_->addSubWindow(subWindow);
  treeControlers_->applyOptions(&subWindow->getTreeCanvas());
  subWindow->show();
  setCurrentSubWindow(subWindow);
  return doc;
}



QList<TreeDocument*> PhyView::getNonActiveDocuments()
{
  QList<TreeDocument*> documents;
  QList<QMdiSubWindow *> lst = mdiArea_->subWindowList();
  for (int i = 0; i < lst.size(); ++i) {
    if (lst[i] != mdiArea_->currentSubWindow())
      documents.push_back(dynamic_cast<TreeSubWindow*>(lst[i])->getDocument());
  }
  return documents;
}



void PhyView::openTree()
{
  treeFileDialog_->setAcceptMode(QFileDialog::AcceptOpen);
  if (treeFileDialog_->exec() == QDialog::Accepted) {
    QStringList path = treeFileDialog_->selectedFiles();
    string format = IOTreeFactory::NEWICK_FORMAT;
    if (treeFileDialog_->selectedNameFilter() == treeFileFilters_[1])
      format = IOTreeFactory::NEXUS_FORMAT;
    else if (treeFileDialog_->selectedNameFilter() == treeFileFilters_[2])
      format = IOTreeFactory::NHX_FORMAT;
    auto_ptr<ITree> treeReader(ioTreeFactory_.createReader(format));
    try {
      auto_ptr<Tree> tree(treeReader->read(path[0].toStdString()));
      TreeDocument* doc = createNewDocument(tree.get());
      doc->setFile(path[0].toStdString(), format);
    } catch(Exception& e) {
      QMessageBox::critical(this, tr("Ouch..."), tr("Error when reading file:\n") + tr(e.what()));
    }
  }
}

void PhyView::setCurrentSubWindow(TreeSubWindow* tsw)
{
  if (tsw)
  {
    statsBox_->updateTree(tsw->getTree());
    treeControlers_->setTreeCanvas(&tsw->getTreeCanvas());
    treeControlers_->actualizeOptions();
    manager_.setActiveStack(&tsw->getDocument()->getUndoStack());
  }
}

bool PhyView::saveTree()
{
  TreeDocument* doc = getActiveDocument();
  if (doc->getFilePath() == "")
    return saveTreeAs();
  string format = doc->getFileFormat();
  auto_ptr<OTree> treeWriter(ioTreeFactory_.createWriter(format));
  treeWriter->write(*doc->getTree(), doc->getFilePath(), true); 
  return true;
}

bool PhyView::saveTreeAs()
{
  treeFileDialog_->setAcceptMode(QFileDialog::AcceptSave);
  if (treeFileDialog_->exec() == QDialog::Accepted) {
    QStringList path = treeFileDialog_->selectedFiles();
    TreeDocument* doc = getActiveDocument();
    string format = IOTreeFactory::NEWICK_FORMAT;
    if (treeFileDialog_->selectedNameFilter() == treeFileFilters_[1])
      format = IOTreeFactory::NEXUS_FORMAT;
    doc->setFile(path[0].toStdString(), format);
    return saveTree();
  }
  return false;
}

void PhyView::closeTree()
{
  if (mdiArea_->currentSubWindow())
    mdiArea_->currentSubWindow()->close();
}

void PhyView::exit()
{
  close();
}

void PhyView::aboutBpp()
{
  QMessageBox msgBox;
  msgBox.setText("Bio++ CVS version.");
  msgBox.setInformativeText("bpp-core XXX\nbpp-seq XXX\nbpp-phyl XXX\nbpp-qt 0.1.0");
  msgBox.exec();
}

void PhyView::about()
{
  QMessageBox msgBox;
  msgBox.setText("This is Bio++ Phy View version 0.1.0.");
  msgBox.setInformativeText("Julien Dutheil <jdutheil@birc.au.dk>.");
  msgBox.exec();
}

void PhyView::updateStatusBar()
{
}

void PhyView::setLengths()
{
  if (hasActiveDocument())
    submitCommand(new SetLengthCommand(getActiveDocument(), brlenSetLengths_->value()));
}

void PhyView::initLengthsGrafen()
{
  if (hasActiveDocument())
    submitCommand(new InitGrafenCommand(getActiveDocument()));
}

void PhyView::computeLengthsGrafen()
{
  if (hasActiveDocument())
    submitCommand(new ComputeGrafenCommand(getActiveDocument(), brlenComputeGrafen_->value()));
}

void PhyView::convertToClockTree()
{
  if (hasActiveDocument())
    submitCommand(new ConvertToClockTreeCommand(getActiveDocument()));
}

void PhyView::midpointRooting()
{
  if (hasActiveDocument())
    try {
      submitCommand(new MidpointRootingCommand(getActiveDocument()));
    } catch (NodeException& ex) {
      QMessageBox::critical(this, tr("Oups..."), tr("Some branch do not have lengths."));
    }
}

void PhyView::translateNames()
{
  if (hasActiveDocument())
  {
    translateNameChooser_->translateTree(*getActiveDocument()->getTree()); 
  }
}

void PhyView::controlerTakesAction()
{
  QList<QMdiSubWindow *> lst = mdiArea_->subWindowList();
  for (int i = 0; i < lst.size(); ++i) {
    dynamic_cast<TreeSubWindow*>(lst[i])->getTreeCanvas().redraw();
  }
}

void PhyView::attachData()
{
  dataFileDialog_->setAcceptMode(QFileDialog::AcceptOpen);
  if (dataFileDialog_->exec() == QDialog::Accepted) {
    QStringList path = dataFileDialog_->selectedFiles();
    string sep = ",";
    if (dataFileDialog_->selectedNameFilter() == dataFileFilters_[1])
      sep = "\t";
    ifstream file(path[0].toStdString().c_str(), ios::in);
    DataTable* table = DataTable::read(file, sep);
    dataLoader_->load(table);
  }
}

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  PhyView* phyview = new PhyView();
  phyview->show();

  return app.exec();
}


