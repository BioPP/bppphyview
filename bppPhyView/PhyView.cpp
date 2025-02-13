// SPDX-FileCopyrightText: The Bio++ Development Group
//
// SPDX-License-Identifier: CECILL-2.1

#include "PhyView.h"
#include "TreeSubWindow.h"
#include "TreeDocument.h"

#include <QApplication>
#include <QtGui>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QButtonGroup>
#include <QDockWidget>
#include <QUndoView>
#include <QLineEdit>
#include <QAction>
#include <QMenuBar>
#include <QInputDialog>
#include <QGraphicsTextItem>

#include <Bpp/Qt/QtGraphicDevice.h>

#include <Bpp/Numeric/DataTable.h>

#include <Bpp/Phyl/Tree/Tree.h>
#include <Bpp/Phyl/Io/Nhx.h>
#include <Bpp/Phyl/Graphics/PhylogramPlot.h>

#include <fstream>

using namespace std;
using namespace bpp;

MouseActionListener::MouseActionListener(PhyView* phyview) :
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
  treeChooser_->connect(treeList_, &QListWidget::itemClicked, treeChooser_, &QDialog::accept);
}


TranslateNameChooser::TranslateNameChooser(PhyView* phyview) :
  QDialog(phyview), phyview_(phyview), fileDialog_(new QFileDialog(this))
{
  fileFilters_ << "Coma separated columns (*.txt *.csv)"
               << "Tab separated columns (*.txt *.csv)";
  fileDialog_->setNameFilters(fileFilters_);
  fileDialog_->setOptions(QFileDialog::DontUseNativeDialog);
  hasHeader_ = new QCheckBox(tr("File has header line"));
  QGridLayout* dlayout = dynamic_cast<QGridLayout*>(fileDialog_->layout()); // Check that here!!
  dlayout->addWidget(hasHeader_, 4, 0);
  QFormLayout* layout = new QFormLayout;
  fromList_ = new QComboBox;
  toList_   = new QComboBox;
  ok_       = new QPushButton(tr("Ok"));
  cancel_   = new QPushButton(tr("Cancel"));
  layout->addRow(tr("From"), fromList_);
  layout->addRow(tr("To"), toList_);
  layout->addRow(cancel_, ok_);
  connect(ok_, &QPushButton::clicked, this, &TranslateNameChooser::accept);
  connect(cancel_, &QPushButton::clicked, this, &TranslateNameChooser::reject);
  setLayout(layout);
}

void TranslateNameChooser::translateTree(TreeTemplate<Node>& tree)
{
  fileDialog_->setAcceptMode(QFileDialog::AcceptOpen);
  if (fileDialog_->exec() == QDialog::Accepted)
  {
    QStringList path = fileDialog_->selectedFiles();
    string sep = ",";
    if (fileDialog_->selectedNameFilter() == fileFilters_[1])
      sep = "\t";
    ifstream file(path[0].toStdString().c_str(), ios::in);
    try
    {
      auto table = DataTable::read(file, sep, hasHeader_->isChecked());

      // Clean button groups:
      fromList_->clear();
      toList_->clear();

      // Now add the new ones:
      if (!hasHeader_->isChecked())
      {
        vector<string> names;
        for (unsigned int i = 0; i < table->getNumberOfColumns(); ++i)
        {
          names.push_back("Col" + TextTools::toString(i + 1));
        }
        table->setColumnNames(names);
      }
      for (unsigned int i = 0; i < table->getNumberOfColumns(); ++i)
      {
        fromList_->addItem(QtTools::toQt(table->getColumnName(i)));
        toList_->addItem(QtTools::toQt(table->getColumnName(i)));
      }
      if (exec() == QDialog::Accepted)
        phyview_->submitCommand(new TranslateNodeNamesCommand(
				phyview_->getActiveDocument(),
			       	*table, fromList_->currentIndex(),
			      	toList_->currentIndex()));
    }
    catch (Exception& e)
    {
      QMessageBox::critical(this, tr("Ouch..."), tr("Error when reading table:\n") + tr(e.what()));
    }
  }
}

NamesFromDataDialog::NamesFromDataDialog(PhyView* phyview) :
  QDialog(phyview), phyview_(phyview)
{
  QFormLayout* layout = new QFormLayout;
  variableCol_    = new QComboBox;
  innerNodesOnly_ = new QCheckBox(tr("Only inner nodes"));
  ok_             = new QPushButton(tr("Ok"));
  cancel_         = new QPushButton(tr("Cancel"));
  layout->addRow(tr("Variable"), variableCol_);
  layout->addRow(tr(""), innerNodesOnly_);
  layout->addRow(cancel_, ok_);
  connect(ok_, &QPushButton::clicked, this, &NamesFromDataDialog::accept);
  connect(cancel_, &QPushButton::clicked, this, &NamesFromDataDialog::reject);
  setLayout(layout);
}

void NamesFromDataDialog::setNamesFromData()
{
  variableCol_->clear();
  vector<string> names;
  TreeTemplateTools::getNodePropertyNames(phyview_->getActiveDocument()->tree().rootNode(), names);
  if (names.size() == 0) {
    QMessageBox::critical(this, tr("No data available"), tr("Associate data to the tree\nto enable node (re)naming."));
    return;
  }
  for (const auto& name : names)
  {
    variableCol_->addItem(QtTools::toQt(name));
  }
  if (exec() == QDialog::Accepted)
    phyview_->submitCommand(new SetNamesFromDataCommand(
			    phyview_->getActiveDocument(),
			    variableCol_->currentText().toStdString(),
			    innerNodesOnly_->isChecked()));
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
  connect(ok_, &QPushButton::clicked, this, &DataLoader::accept);
  connect(cancel_, &QPushButton::clicked, this, &DataLoader::reject);
  setLayout(layout);
}

void DataLoader::load(const DataTable& data)
{
  indexCol_->clear();
  for (unsigned int i = 0; i < data.getNumberOfColumns(); ++i)
  {
    indexCol_->addItem(QtTools::toQt(data.getColumnName(i)));
  }
  if (exec() == QDialog::Accepted)
  {
    unsigned int index = static_cast<unsigned int>(indexCol_->currentIndex());
    phyview_->submitCommand(new AttachDataCommand(phyview_->getActiveDocument(), data, index, nameIndex_->isChecked()));
  }
}

AsrDialog::AsrDialog(PhyView* phyview) :
  QDialog(phyview), phyview_(phyview)
{
  QFormLayout* layout = new QFormLayout;
  variableCol_ = new QComboBox;
  asrMethod_   = new QComboBox;
  asrMethod_->addItem(QString("Naive ASR"));
  ok_          = new QPushButton(tr("Ok"));
  cancel_      = new QPushButton(tr("Cancel"));
  layout->addRow(tr("Variable"), variableCol_);
  layout->addRow(tr("Method"), asrMethod_);
  layout->addRow(cancel_, ok_);
  connect(ok_, &QPushButton::clicked, this, &AsrDialog::accept);
  connect(cancel_, &QPushButton::clicked, this, &AsrDialog::reject);
  setLayout(layout);
}

void AsrDialog::asr()
{
  variableCol_->clear();
  vector<string> names;
  TreeTemplateTools::getNodePropertyNames(phyview_->getActiveDocument()->tree().rootNode(), names);
  if (names.size() == 0) {
    QMessageBox::critical(this, tr("No data available"), tr("Associate data to the tree\nto enable automatic collapsing of nodes."));
    return;
  }
  for (const auto& name : names)
  {
    variableCol_->addItem(QtTools::toQt(name));
  }
  if (exec() == QDialog::Accepted)
  {
    auto propertyName = variableCol_->currentText().toStdString();
    TreeCanvas& tc = phyview_->getActiveSubWindow()->treeCanvas();
    auto x = dynamic_cast<const TreeTemplate<Node>&>(tc.tree());
    //So far, only the naive ASR is supported.
    phyview_->submitCommand(new NaiveAsrCommand(phyview_->getActiveDocument(), propertyName));
  }
}

ImageExportDialog::ImageExportDialog(PhyView* phyview) :
  QDialog(phyview)
{
  QGridLayout* layout = new QGridLayout;
  path_   = new QLabel;
  path_->setText("(none selected)");
  layout->addWidget(path_, 1, 1);

  browse_ = new QPushButton(tr("&Browse"));
  connect(browse_, &QPushButton::clicked, this, &ImageExportDialog::chosePath);
  layout->addWidget(browse_, 1, 2);

  height_ = new QSpinBox;
  height_->setRange(100, 10000);
  layout->addWidget(new QLabel(tr("Height:")), 2, 1);
  layout->addWidget(height_, 2, 2);

  width_ = new QSpinBox;
  width_->setRange(100, 10000);
  layout->addWidget(new QLabel(tr("Width:")), 3, 1);
  layout->addWidget(width_, 3, 2);

  transparent_ = new QCheckBox(tr("Transparent"));
  layout->addWidget(transparent_, 4, 1, 1, 2);

  keepAspectRatio_ = new QCheckBox(tr("Keep aspect ratio"));
  layout->addWidget(keepAspectRatio_, 5, 1, 1, 2);

  ok_       = new QPushButton(tr("Ok"));
  ok_->setDisabled(true);
  connect(ok_, &QPushButton::clicked, this, &ImageExportDialog::accept);
  layout->addWidget(ok_, 6, 2);

  cancel_   = new QPushButton(tr("Cancel"));
  connect(cancel_, &QPushButton::clicked, this, &ImageExportDialog::reject);
  layout->addWidget(cancel_, 6, 1);

  setLayout(layout);

  imageFileDialog_ = new QFileDialog(this, "Image File");
  QList<QByteArray> formats = QImageWriter::supportedImageFormats();
  for (int i = 0; i < formats.size(); ++i)
  {
    imageFileFilters_ << QString(formats[i]) + QString(" (*.*)");
  }
  imageFileDialog_->setNameFilters(imageFileFilters_);
}

void ImageExportDialog::chosePath()
{
  if (imageFileDialog_->exec() == QDialog::Accepted)
  {
    QStringList path = imageFileDialog_->selectedFiles();
    int i = imageFileFilters_.indexOf(imageFileDialog_->selectedNameFilter());
    path_->setText(path[0] + " (" + QString(QImageWriter::supportedImageFormats()[i]) + ")");
    ok_->setEnabled(true);
  }
}

void ImageExportDialog::process(QGraphicsScene* scene)
{
  if (ok_->isEnabled())
  {
    QStringList path = imageFileDialog_->selectedFiles();
    int i = imageFileFilters_.indexOf(imageFileDialog_->selectedNameFilter());
    // Chose the correct format according to options:
    QImage::Format format = QImage::Format_RGB32;
    QBrush bckBrush = scene->backgroundBrush();
    if (transparent_->isChecked())
    {
      format = QImage::Format_ARGB32_Premultiplied;
      scene->setBackgroundBrush(Qt::NoBrush);
    }
    else
    {
      if (bckBrush == Qt::NoBrush)
        scene->setBackgroundBrush(Qt::white);
    }
    QImage image(width_->value(), height_->value(), format);
    QPainter painter;
    painter.begin(&image);
    if (keepAspectRatio_->isChecked())
      scene->render(&painter);
    else
      scene->render(&painter, QRectF(), QRectF(), Qt::IgnoreAspectRatio);
    painter.end();
    scene->setBackgroundBrush(bckBrush);
    image.save(path[0], QImageWriter::supportedImageFormats()[i]);
  }
  else
  {
    throw Exception("Can't process image as no file has been selected.");
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
  connect(ok_, &QPushButton::clicked, this, &TypeNumberDialog::accept);
  connect(cancel_, &QPushButton::clicked, this, &TypeNumberDialog::reject);
  setLayout(layout);
}



CollapseDialog::CollapseDialog(PhyView* phyview) :
  QDialog(phyview), phyview_(phyview)
{
  QFormLayout* layout = new QFormLayout;
  variableCol_     = new QComboBox;
  allowMissing_    = new QCheckBox(tr("Allow missing data"));
  missingDataText_ = new QLineEdit();
  missingDataText_->setEnabled(false);
  connect(allowMissing_, &QCheckBox::checkStateChanged, missingDataText_, [this](){ missingDataText_->setEnabled(!missingDataText_->isEnabled()); });
  ok_              = new QPushButton(tr("Ok"));
  cancel_          = new QPushButton(tr("Cancel"));
  layout->addRow(tr("Variable"), variableCol_);
  layout->addRow(tr(""), allowMissing_);
  layout->addRow(tr("Missing data text:"), missingDataText_);
  layout->addRow(cancel_, ok_);
  connect(ok_, &QPushButton::clicked, this, &CollapseDialog::accept);
  connect(cancel_, &QPushButton::clicked, this, &CollapseDialog::reject);
  setLayout(layout);
}

void CollapseDialog::collapse()
{
  variableCol_->clear();
  vector<string> names;
  TreeTemplateTools::getNodePropertyNames(phyview_->getActiveDocument()->tree().rootNode(), names);
  if (names.size() == 0) {
    QMessageBox::critical(this, tr("No data available"), tr("Associate data to the tree\nto enable automatic collapsing of nodes."));
    return;
  }
  for (const auto& name : names)
  {
    variableCol_->addItem(QtTools::toQt(name));
  }
  if (exec() == QDialog::Accepted)
  {
    string propertyName = variableCol_->currentText().toStdString();
    bool allowMissingData = allowMissing_->isChecked();
    string naString = missingDataText_->text().toStdString();
    TreeCanvas& tc = phyview_->getActiveSubWindow()->treeCanvas();
    bool isMonophyletic;
    scan_(tc, dynamic_cast<const TreeTemplate<Node>&>(tc.tree()).rootNode(), propertyName, isMonophyletic, allowMissingData, naString);
    tc.redraw();
  }
}

std::string CollapseDialog::scan_(
    TreeCanvas& tc,
    const Node& node,
    const string& propertyName,
    bool& isMonophyletic,
    bool allowMissingData,
    const string& naString)
{
  if (node.isLeaf()) {
    isMonophyletic = false;
    if (node.hasNodeProperty(propertyName)) {
      string state = dynamic_cast<const BppString*>(node.getNodeProperty(propertyName))->toSTL();
      if (state != naString) isMonophyletic = true;
      return state;
    } else {
      return naString;
    }
  } else {
    vector<string> states(node.getNumberOfSons());
    //We first call the function recursively on all subtrees:
    isMonophyletic = true;
    for (size_t i = 0; i < node.getNumberOfSons(); ++i) {
      bool test;
      states[i] = scan_(tc, node.son(i), propertyName, test, allowMissingData, naString);
      isMonophyletic = isMonophyletic && test;
    }
    string state;
    bool first = true;
    for (size_t i = 0; i < node.getNumberOfSons(); ++i) {
      if (states[i] == naString) {
	if (!allowMissingData) {
          isMonophyletic = false;
          return naString;
	} // else check next subtree
      } else {
	if (first) {
          state = states[i];
	  first = false;
	} else {
          if (states[i] != state) {
            isMonophyletic = false;
	    return naString;
	  }// same state, proceed
	}
      } 
    }
    if (isMonophyletic)
      tc.collapseNode(node.getId(), true);
    return state;  
  }
}


void MouseActionListener::mousePressEvent(QMouseEvent* event)
{
  if (dynamic_cast<NodeMouseEvent*>(event)->hasNodeId())
  {
    int nodeId = dynamic_cast<NodeMouseEvent*>(event)->getNodeId();
    QString action;
    if (event->button() == Qt::LeftButton)
      action = phyview_->getMouseLeftButtonActionType();
    else if (event->button() == Qt::MiddleButton)
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
        if (sonsId[0] == nodeId)
        {
          i1 = 0;
          i2 = sonsId.size() - 1;
        }
        else
        {
          for (unsigned int i = 1; i < sonsId.size(); ++i)
          {
            if (sonsId[i] == nodeId)
            {
              i1 = i;
              i2 = i - 1;
            }
          }
        }
        phyview_->submitCommand(new SwapCommand(phyview_->getActiveDocument(), fatherId, i1, i2, nodeId, sonsId[i2]));
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
    {
      if (phyview_->getActiveDocument()->getTree()->getNode(nodeId)->isLeaf())
      {
        QMessageBox::warning(phyview_, "PhyView", "Cannot root on a leaf.", QMessageBox::Cancel);
      }
      else
      {
        phyview_->submitCommand(new RerootCommand(phyview_->getActiveDocument(), nodeId));
      }
    }
    else if (action == "Root on branch")
    {
      phyview_->submitCommand(new OutgroupCommand(phyview_->getActiveDocument(), nodeId));
    }
    else if (action == "Collapse")
    {
      TreeCanvas& tc = phyview_->getActiveSubWindow()->treeCanvas();
      tc.collapseNode(nodeId, !tc.isNodeCollapsed(nodeId));
      tc.redraw();
    }
    else if (action == "Sample subtree")
    {
      Node* n = phyview_->getActiveDocument()->tree().getNode(nodeId);
      TypeNumberDialog dial(phyview_, "Sample size", 1u, TreeTemplateTools::getNumberOfLeaves(*n));
      if (dial.exec() == QDialog::Accepted)
      {
        unsigned int size = dial.getValue();
        phyview_->submitCommand(new SampleSubtreeCommand(phyview_->getActiveDocument(), nodeId, size));
      }
    }
    else if (action == "Delete subtree")
    {
      phyview_->submitCommand(new DeleteSubtreeCommand(phyview_->getActiveDocument(), nodeId));
    }
    else if (action == "Copy subtree")
    {
      Node* subtree = TreeTemplateTools::cloneSubtree<Node>(*phyview_->getActiveDocument()->tree().getNode(nodeId));
      unique_ptr< TreeTemplate<Node>> tt(new TreeTemplate<Node>(subtree));
      phyview_->createNewDocument(tt.get());
    }
    else if (action == "Cut subtree")
    {
      Node* subtree = TreeTemplateTools::cloneSubtree<Node>(*phyview_->getActiveDocument()->tree().getNode(nodeId));
      unique_ptr< TreeTemplate<Node>> tt(new TreeTemplate<Node>(subtree));
      phyview_->submitCommand(new DeleteSubtreeCommand(phyview_->getActiveDocument(), nodeId));
      phyview_->createNewDocument(tt.get());
    }
    else if (action == "Insert on node")
    {
      auto tree = phyview_->pickTree();
      if (tree)
      {
        Node* subtree = TreeTemplateTools::cloneSubtree<Node>(*tree->getRootNode());
        phyview_->submitCommand(new InsertSubtreeAtNodeCommand(phyview_->getActiveDocument(), nodeId, subtree));
      }
    }
    else if (action == "Insert on branch")
    {
      auto tree = phyview_->pickTree();
      if (tree)
      {
        Node* subtree = TreeTemplateTools::cloneSubtree<Node>(*tree->getRootNode());
        phyview_->submitCommand(new InsertSubtreeOnBranchCommand(phyview_->getActiveDocument(), nodeId, subtree));
      }
    }
    else if (action == "Show associated data")
    {
      phyview_->updateDataViewer(phyview_->getActiveDocument()->tree(), nodeId);
    }
  }
}


PhyView::PhyView() :
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
  connect(mdiArea_, &QMdiArea::subWindowActivated, this, qOverload<QMdiSubWindow*>(&PhyView::setCurrentSubWindow));
  setCentralWidget(mdiArea_);

  // Trees panel:
  createTreesPanel_();
  treesDockWidget_ = new QDockWidget(tr("Trees"));
  treesDockWidget_->setWidget(treesPanel_);
  treesDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  addDockWidget(Qt::LeftDockWidgetArea, treesDockWidget_);

  // Stats panel:
  createStatsPanel_();
  statsDockWidget_ = new QDockWidget(tr("Statistics"));
  statsDockWidget_->setWidget(statsPanel_);
  statsDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  addDockWidget(Qt::RightDockWidgetArea, statsDockWidget_);

  // Display panel:
  createDisplayPanel_();
  displayDockWidget_ = new QDockWidget(tr("Display"));
  displayDockWidget_->setWidget(displayPanel_);
  displayDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  addDockWidget(Qt::RightDockWidgetArea, displayDockWidget_);

  // Search panel:
  createSearchPanel_();
  searchDockWidget_ = new QDockWidget(tr("Search in tree"));
  searchDockWidget_->setWidget(searchPanel_);
  searchDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  addDockWidget(Qt::LeftDockWidgetArea, searchDockWidget_);

  // Undo panel:
  QUndoView* undoView = new QUndoView;
  undoView->setGroup(&manager_);
  undoDockWidget_ = new QDockWidget(tr("Undo list"));
  undoDockWidget_->setWidget(undoView);
  undoDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  addDockWidget(Qt::LeftDockWidgetArea, undoDockWidget_);

  // Branch lengths panel:
  createBrlenPanel_();
  brlenDockWidget_ = new QDockWidget(tr("Branch lengths"));
  brlenDockWidget_->setWidget(brlenPanel_);
  brlenDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  addDockWidget(Qt::LeftDockWidgetArea, brlenDockWidget_);
  brlenDockWidget_->setVisible(false);

  // Mouse control panel:
  createMouseControlPanel_();
  mouseControlDockWidget_ = new QDockWidget(tr("Mouse control"));
  mouseControlDockWidget_->setWidget(mouseControlPanel_);
  mouseControlDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  addDockWidget(Qt::LeftDockWidgetArea, mouseControlDockWidget_);

  // Associated data panel:
  createDataPanel_();
  dataDockWidget_ = new QDockWidget(tr("Associated Data"));
  dataDockWidget_->setWidget(dataPanel_);
  dataDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  addDockWidget(Qt::RightDockWidgetArea, dataDockWidget_);
  dataDockWidget_->setVisible(false);

  // Associated data viewer panel:
  createDataViewerPanel_();
  dataViewerDockWidget_ = new QDockWidget(tr("Associated Data Viewer"));
  dataViewerDockWidget_->setWidget(dataViewerPanel_);
  dataViewerDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  addDockWidget(Qt::RightDockWidgetArea, dataViewerDockWidget_);
  dataViewerDockWidget_->setVisible(false);

  // Other stuff...
  treeFileDialog_ = new QFileDialog(this, "Tree File");
  treeFileFilters_ << "Newick files (*.dnd *.tre *.tree *.nwk *.newick *.phy *.txt)"
                   << "Nexus files (*.nx *.nex *.nexus)"
                   << "Nhx files (*.nhx)";
  treeFileDialog_->setNameFilters(treeFileFilters_);
  treeFileDialog_->setOption(QFileDialog::DontConfirmOverwrite, false);

  dataFileDialog_ = new QFileDialog(this, "Data File");
  dataFileFilters_ << "Coma separated columns (*.txt *.csv)"
                   << "Tab separated columns (*.txt *.csv)";
  dataFileDialog_->setNameFilters(dataFileFilters_);

  imageExportDialog_ = new ImageExportDialog(this);

  printer_ = new QPrinter(QPrinter::HighResolution);
  printDialog_ = new QPrintDialog(printer_, this);

  translateNameChooser_ = new TranslateNameChooser(this);
  
  namesFromDataDialog_ = new NamesFromDataDialog(this);

  dataLoader_ = new DataLoader(this);
  
  collapseDialog_ = new CollapseDialog(this);
  
  asrDialog_ = new AsrDialog(this);
}

void PhyView::createDisplayPanel_()
{
  displayPanel_ = new QWidget(this);
  treeControlers_ = new TreeCanvasControlers();
  treeControlers_->addActionListener(this);
  for (unsigned int i = 0; i < treeControlers_->getNumberOfTreeDrawings(); ++i)
  {
    treeControlers_->getTreeDrawing(i)->addTreeDrawingListener(&collapsedNodesListener_);
  }

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

  QGroupBox* collapseOptions = new QGroupBox(tr("Collapse"));
  QVBoxLayout* collapseLayout = new QVBoxLayout;
  uncollapseAll_ = new QPushButton(tr("Uncollapse all"));
  autoCollapse_ = new QPushButton(tr("Auto collapse"));
  collapseLayout->addWidget(uncollapseAll_);
  collapseLayout->addWidget(autoCollapse_);
  connect(uncollapseAll_, &QPushButton::clicked, this, &PhyView::uncollapseAll);
  connect(autoCollapse_, &QPushButton::clicked, this, &PhyView::autoCollapse);
  collapseOptions->setLayout(collapseLayout);

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(drawingOptions);
  layout->addWidget(displayOptions);
  layout->addWidget(collapseOptions);
  layout->addStretch(1);
  displayPanel_->setLayout(layout);
}

void PhyView::createTreesPanel_()
{
  treesPanel_ = new QWidget(this);
  QVBoxLayout* treesLayout = new QVBoxLayout;
  treesTable_ = new QTableWidget;
  treesTable_->setColumnCount(2);
  treesTable_->setHorizontalHeaderLabels(QString("Tree;Size").split(";"));
  treesTable_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  treesTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  treesTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
  treesTable_->setSelectionMode(QAbstractItemView::SingleSelection);
  connect(treesTable_, &QTableWidget::itemClicked, this, &PhyView::activateSelectedDocument);
  treesLayout->addWidget(treesTable_);
  treesLayout->addStretch(1);
  treesPanel_->setLayout(treesLayout);
}

void PhyView::createStatsPanel_()
{
  statsPanel_ = new QWidget(this);
  QVBoxLayout* statsLayout = new QVBoxLayout;
  statsBox_ = new TreeStatisticsBox;
  statsLayout->addWidget(statsBox_);
  QPushButton* update = new QPushButton(tr("Update"));
  connect(update, &QPushButton::clicked, this, &PhyView::updateStatistics);
  statsLayout->addWidget(update);
  statsLayout->addStretch(1);
  statsPanel_->setLayout(statsLayout);
}

void PhyView::createBrlenPanel_()
{
  brlenPanel_ = new QWidget(this);
  QVBoxLayout* brlenLayout = new QVBoxLayout;

  // Set all lengths:
  brlenSetLengths_ = new QDoubleSpinBox;
  brlenSetLengths_->setDecimals(6);
  brlenSetLengths_->setSingleStep(0.01);
  QPushButton* brlenSetLengthsGo = new QPushButton(tr("Go!"));
  connect(brlenSetLengthsGo, &QPushButton::clicked, this, &PhyView::setLengths);

  QGroupBox* brlenSetLengthsBox = new QGroupBox(tr("Set all lengths"));
  QHBoxLayout* brlenSetLengthsBoxLayout = new QHBoxLayout;
  brlenSetLengthsBoxLayout->addWidget(brlenSetLengths_);
  brlenSetLengthsBoxLayout->addWidget(brlenSetLengthsGo);
  brlenSetLengthsBoxLayout->addStretch(1);
  brlenSetLengthsBox->setLayout(brlenSetLengthsBoxLayout);

  brlenLayout->addWidget(brlenSetLengthsBox);

  // Remove all branch lengths:
  QPushButton* brlenRemoveAll = new QPushButton(tr("Remove all lengths"));
  connect(brlenRemoveAll, &QPushButton::clicked, this, &PhyView::deleteAllLengths);
  brlenLayout->addWidget(brlenRemoveAll);

  // Grafen method:
  QPushButton* brlenInitGrafen = new QPushButton(tr("Init"));
  connect(brlenInitGrafen, &QPushButton::clicked, this, &PhyView::initLengthsGrafen);

  brlenComputeGrafen_ = new QDoubleSpinBox;
  brlenComputeGrafen_->setValue(1.);
  brlenComputeGrafen_->setDecimals(2);
  brlenComputeGrafen_->setSingleStep(0.1);
  QPushButton* brlenComputeGrafenGo = new QPushButton(tr("Go!"));
  connect(brlenComputeGrafenGo, &QPushButton::clicked, this, &PhyView::computeLengthsGrafen);

  QGroupBox* brlenGrafenBox = new QGroupBox(tr("Grafen"));
  QHBoxLayout* brlenGrafenBoxLayout = new QHBoxLayout;
  brlenGrafenBoxLayout->addWidget(brlenInitGrafen);
  brlenGrafenBoxLayout->addWidget(brlenComputeGrafen_);
  brlenGrafenBoxLayout->addWidget(brlenComputeGrafenGo);
  brlenGrafenBoxLayout->addStretch(1);
  brlenGrafenBox->setLayout(brlenGrafenBoxLayout);

  brlenLayout->addWidget(brlenGrafenBox);

  // To clock tree:
  QPushButton* brlenToClockTree = new QPushButton(tr("Convert to clock"));
  connect(brlenToClockTree, &QPushButton::clicked, this, &PhyView::convertToClockTree);
  brlenLayout->addWidget(brlenToClockTree);

  // Midpoint rooting:
  brlenMidpointRootingCriteria_ =  new QComboBox;
  brlenMidpointRootingCriteria_->addItem("Sum of squares");
  brlenMidpointRootingCriteria_->addItem("Variance");
  brlenMidpointRootingCriteria_->setEditable(false);
  QPushButton* brlenMidpointRootingGo = new QPushButton(tr("Go!"));
  connect(brlenMidpointRootingGo, &QPushButton::clicked, this, &PhyView::midpointRooting);
  QGroupBox* brlenMidpointRootingBox = new QGroupBox(tr("Midpoint rooting"));
  QHBoxLayout* brlenMidpointRootingLayout = new QHBoxLayout;
  brlenMidpointRootingLayout->addWidget(brlenMidpointRootingCriteria_);
  brlenMidpointRootingLayout->addWidget(brlenMidpointRootingGo);
  brlenMidpointRootingLayout->addStretch(1);
  brlenMidpointRootingBox->setLayout(brlenMidpointRootingLayout);

  brlenLayout->addWidget(brlenMidpointRootingBox);

  // Unresolved uncertain trees:
  bootstrapThreshold_ = new QDoubleSpinBox;
  bootstrapThreshold_->setValue(60);
  bootstrapThreshold_->setDecimals(2);
  bootstrapThreshold_->setSingleStep(0.1);
  QPushButton* unresolveUncertainNodesGo = new QPushButton(tr("Go!"));
  connect(unresolveUncertainNodesGo, &QPushButton::clicked, this, &PhyView::unresolveUncertainNodes);

  QGroupBox* unresolveUncertainNodesBox = new QGroupBox(tr("Unresolve uncertain nodes"));
  QHBoxLayout* unresolveUncertainNodesLayout = new QHBoxLayout;
  unresolveUncertainNodesLayout->addWidget(bootstrapThreshold_);
  unresolveUncertainNodesLayout->addWidget(unresolveUncertainNodesGo);
  unresolveUncertainNodesLayout->addStretch(1);
  unresolveUncertainNodesBox->setLayout(unresolveUncertainNodesLayout);

  brlenLayout->addWidget(unresolveUncertainNodesBox);

  // Remove all support values:
  QPushButton* supportRemoveAll = new QPushButton(tr("Remove all support values"));
  connect(supportRemoveAll, &QPushButton::clicked, this, &PhyView::deleteAllSupportValues);
  brlenLayout->addWidget(supportRemoveAll);


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
  mouseActions.append(tr("Show associated data"));

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
  connect(loadData_, &QPushButton::clicked, this, &PhyView::attachData);
  dataLayout->addWidget(loadData_);

  saveData_ = new QPushButton(tr("Save Data"));
  connect(saveData_, &QPushButton::clicked, this, &PhyView::saveData);
  dataLayout->addWidget(saveData_);

  addData_ = new QPushButton(tr("Add Data"));
  connect(addData_, &QPushButton::clicked, this, &PhyView::addData);
  dataLayout->addWidget(addData_);

  removeData_ = new QPushButton(tr("Remove Data"));
  connect(removeData_, &QPushButton::clicked, this, &PhyView::removeData);
  dataLayout->addWidget(removeData_);

  renameData_ = new QPushButton(tr("Rename Data"));
  connect(renameData_, &QPushButton::clicked, this, &PhyView::renameData);
  dataLayout->addWidget(renameData_);

  translateNames_ = new QPushButton(tr("Translate"));
  connect(translateNames_, &QPushButton::clicked, this, &PhyView::translateNames);
  dataLayout->addWidget(translateNames_);

  setNamesFromData_ = new QPushButton(tr("Set node names from data"));
  connect(setNamesFromData_, &QPushButton::clicked, this, &PhyView::setNamesFromData);
  dataLayout->addWidget(setNamesFromData_);

  duplicateDownSelection_ = new QPushButton(tr("Duplicate down"));
  connect(duplicateDownSelection_, &QPushButton::clicked, this, &PhyView::duplicateDownSelection);
  dataLayout->addWidget(duplicateDownSelection_);

  asr_ = new QPushButton(tr("Ancestral State Reconstruction"));
  connect(asr_, &QPushButton::clicked, this, &PhyView::ancestralStateReconstruction);
  dataLayout->addWidget(asr_);

  snapData_ = new QPushButton(tr("Snap shot"));
  connect(snapData_, &QPushButton::clicked, this, &PhyView::snapData);
  dataLayout->addWidget(snapData_);

  dataPanel_->setLayout(dataLayout);
}

void PhyView::createDataViewerPanel_()
{
  dataViewerPanel_ = new QWidget;
  QVBoxLayout* dataViewerLayout = new QVBoxLayout;

  dataViewerTable_ = new QTableWidget(this);
  dataViewerTable_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  dataViewerTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  dataViewerLayout->addWidget(dataViewerTable_);

  dataViewerPanel_->setLayout(dataViewerLayout);
}

void PhyView::createSearchPanel_()
{
  searchPanel_ = new QWidget;
  QVBoxLayout* searchLayout = new QVBoxLayout;

  searchText_ = new QLineEdit();
  connect(searchText_, &QLineEdit::returnPressed, this, &PhyView::searchText);
  searchLayout->addWidget(searchText_);

  searchResults_ = new QListWidget();
  searchResults_->setSelectionMode(QAbstractItemView::SingleSelection);
  connect(searchResults_, &QListWidget::itemClicked, this, &PhyView::searchResultSelected);
  searchLayout->addWidget(searchResults_);

  searchPanel_->setLayout(searchLayout);
}

void PhyView::createActions_()
{
  openAction_ = new QAction(tr("&Open"), this);
  openAction_->setShortcut(tr("Ctrl+O"));
  openAction_->setStatusTip(tr("Open a new tree file"));
  connect(openAction_, &QAction::triggered, this, &PhyView::openTree);

  saveAction_ = new QAction(tr("&Save"), this);
  saveAction_->setShortcut(tr("Ctrl+S"));
  saveAction_->setStatusTip(tr("Save the current tree to file"));
  saveAction_->setDisabled(true);
  connect(saveAction_, &QAction::triggered, this, &PhyView::saveTree);

  saveAsAction_ = new QAction(tr("Save &as"), this);
  saveAsAction_->setShortcut(tr("Ctrl+Shift+S"));
  saveAsAction_->setStatusTip(tr("Save the current tree to a file"));
  saveAsAction_->setDisabled(true);
  connect(saveAsAction_, &QAction::triggered, this, &PhyView::saveTreeAs);

  closeAction_ = new QAction(tr("&Close"), this);
  closeAction_->setShortcut(tr("Ctrl+W"));
  closeAction_->setStatusTip(tr("Close the current tree plot."));
  closeAction_->setDisabled(true);
  connect(closeAction_, &QAction::triggered, this, &PhyView::closeTree);

  exportAction_ = new QAction(tr("Export as &Image"), this);
  exportAction_->setShortcut(tr("Ctrl+I"));
  exportAction_->setStatusTip(tr("Print the current tree plot."));
  exportAction_->setDisabled(true);
  connect(exportAction_, &QAction::triggered, this, &PhyView::exportTree);

  printAction_ = new QAction(tr("&Print"), this);
  printAction_->setShortcut(tr("Ctrl+P"));
  printAction_->setStatusTip(tr("Print the current tree plot."));
  printAction_->setDisabled(true);
  connect(printAction_, &QAction::triggered, this, &PhyView::printTree);

  exitAction_ = new QAction(tr("&Quit"), this);
  exitAction_->setShortcut(tr("Ctrl+Q"));
  exitAction_->setStatusTip(tr("Quit PhyView"));
  connect(exitAction_, &QAction::triggered, this, &PhyView::exit);

  cascadeWinAction_ = new QAction(tr("&Cascade windows"), this);
  connect(cascadeWinAction_, &QAction::triggered, mdiArea_, &QMdiArea::cascadeSubWindows);

  tileWinAction_ = new QAction(tr("&Tile windows"), this);
  connect(tileWinAction_, &QAction::triggered, mdiArea_, &QMdiArea::tileSubWindows);

  aboutAction_ = new QAction(tr("About"), this);
  connect(aboutAction_, &QAction::triggered, this, &PhyView::about);
  aboutBppAction_ = new QAction(tr("About Bio++"), this);
  connect(aboutBppAction_, &QAction::triggered, this, &PhyView::aboutBpp);
  aboutQtAction_ = new QAction(tr("About Qt"), this);
  connect(aboutQtAction_, &QAction::triggered, qApp, &QApplication::aboutQt);

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
  fileMenu_->addAction(exportAction_);
  fileMenu_->addAction(printAction_);
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
  viewMenu_->addAction(dataViewerDockWidget_->toggleViewAction());
  viewMenu_->addAction(searchDockWidget_->toggleViewAction());
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


std::shared_ptr<TreeDocument> PhyView::createNewDocument(Tree* tree)
{
  auto doc = std::make_shared<TreeDocument>();
  doc->setTree(*tree);
  manager_.addStack(&doc->getUndoStack());
  TreeSubWindow* subWindow = new TreeSubWindow(this, doc, treeControlers_->selectedTreeDrawing());
  mdiArea_->addSubWindow(subWindow);
  treeControlers_->applyOptions(subWindow->treeCanvas());
  subWindow->show();
  setCurrentSubWindow(subWindow);
  updateTreesTable();
  return doc;
}


QList<std::shared_ptr<TreeDocument>> PhyView::getNonActiveDocuments()
{
  QList<std::shared_ptr<TreeDocument>> documents;
  QList<QMdiSubWindow*> lst = mdiArea_->subWindowList();
  for (int i = 0; i < lst.size(); ++i)
  {
    if (lst[i] != mdiArea_->currentSubWindow())
      documents.push_back(dynamic_cast<TreeSubWindow*>(lst[i])->getDocument());
  }
  return documents;
}

QList<std::shared_ptr<TreeDocument>> PhyView::getDocuments()
{
  QList<std::shared_ptr<TreeDocument>> documents;
  QList<QMdiSubWindow*> lst = mdiArea_->subWindowList();
  for (int i = 0; i < lst.size(); ++i)
  {
    documents.push_back(dynamic_cast<TreeSubWindow*>(lst[i])->getDocument());
  }
  return documents;
}


void PhyView::readTree(const QString& path, const string& format)
{
  unique_ptr<ITree> treeReader(ioTreeFactory_.createReader(format));
  try
  {
    unique_ptr<Tree> tree(treeReader->readTree(path.toStdString()));
    auto doc = createNewDocument(tree.get());
    doc->setFile(path.toStdString(), format);
    saveAction_->setEnabled(true);
    saveAsAction_->setEnabled(true);
    closeAction_->setEnabled(true);
    exportAction_->setEnabled(true);
    printAction_->setEnabled(true);
    // We need to remove and add action again for menu to be updated :s
    fileMenu_->removeAction(saveAction_);
    fileMenu_->removeAction(saveAsAction_);
    fileMenu_->removeAction(closeAction_);
    fileMenu_->removeAction(exportAction_);
    fileMenu_->removeAction(printAction_);
    fileMenu_->insertAction(exitAction_, saveAction_);
    fileMenu_->insertAction(exitAction_, saveAsAction_);
    fileMenu_->insertAction(exitAction_, closeAction_);
    fileMenu_->insertAction(exitAction_, exportAction_);
    fileMenu_->insertAction(exitAction_, printAction_);
    updateTreesTable();
  }
  catch (Exception& e)
  {
    QMessageBox::critical(this, tr("Ouch..."), tr("Error when reading file:\n") + tr(e.what()));
  }
}


void PhyView::openTree()
{
  treeFileDialog_->setAcceptMode(QFileDialog::AcceptOpen);
  if (treeFileDialog_->exec() == QDialog::Accepted)
  {
    QStringList path = treeFileDialog_->selectedFiles();
    string format = IOTreeFactory::NEWICK_FORMAT;
    if (treeFileDialog_->selectedNameFilter() == treeFileFilters_[1])
      format = IOTreeFactory::NEXUS_FORMAT;
    else if (treeFileDialog_->selectedNameFilter() == treeFileFilters_[2])
      format = IOTreeFactory::NHX_FORMAT;
    readTree(path[0], format);
  }
}


void PhyView::setCurrentSubWindow(TreeSubWindow* tsw)
{
  clearSearchResults();
  if (tsw)
  {
    statsBox_->updateTree(tsw->tree());
    treeControlers_->setTreeCanvas(tsw->getTreeCanvas());
    treeControlers_->actualizeOptions();
    manager_.setActiveStack(&tsw->getDocument()->getUndoStack());
  }
  // Update selection in tree table:
  updateTreesTable(); // We need this here as some windows may have been closed.
  QList<QMdiSubWindow*> lst = mdiArea_->subWindowList();
  for (int i = 0; i < lst.size(); ++i)
  {
    if (lst[i] == mdiArea_->activeSubWindow())
    {
      treesTable_->setRangeSelected(QTableWidgetSelectionRange(i, 0, i, 1), true);
    }
    else
    {
      treesTable_->setRangeSelected(QTableWidgetSelectionRange(i, 0, i, 1), false);
    }
  }
}

bool PhyView::saveTree()
{
  auto doc = getActiveDocument();
  if (doc->getFilePath() == "")
    return saveTreeAs();
  string format = doc->getFileFormat();
  shared_ptr<OTree> treeWriter = ioTreeFactory_.createWriter(format);
  auto nhx = dynamic_pointer_cast<Nhx>(treeWriter);
  if (nhx)
  {
    TreeTemplate<Node> treeCopy(doc->tree());
    nhx->changeNamesToTags(treeCopy.rootNode());
    treeWriter->writeTree(treeCopy, doc->getFilePath(), true);
  }
  else
  {
    treeWriter->writeTree(doc->tree(), doc->getFilePath(), true);
  }
  return true;
}

bool PhyView::saveTreeAs()
{
  treeFileDialog_->setAcceptMode(QFileDialog::AcceptSave);
  if (treeFileDialog_->exec() == QDialog::Accepted)
  {
    QStringList path = treeFileDialog_->selectedFiles();
    auto doc = getActiveDocument();
    string format = IOTreeFactory::NEWICK_FORMAT;
    if (treeFileDialog_->selectedNameFilter() == treeFileFilters_[1])
      format = IOTreeFactory::NEXUS_FORMAT;
    else if (treeFileDialog_->selectedNameFilter() == treeFileFilters_[2])
      format = IOTreeFactory::NHX_FORMAT;
    doc->setFile(path[0].toStdString(), format);
    return saveTree();
  }
  return false;
}

void PhyView::exportTree()
{
  if (imageExportDialog_->exec() == QDialog::Accepted)
  {
    imageExportDialog_->process(getActiveSubWindow()->treeCanvas().scene());
  }
}

void PhyView::printTree()
{
  if (printDialog_->exec() == QDialog::Accepted)
  {
    QPainter painter(printer_);
    getActiveSubWindow()->treeCanvas().scene()->render(&painter);
    painter.end();
  }
}

void PhyView::closeTree()
{
  if (mdiArea_->currentSubWindow())
    mdiArea_->currentSubWindow()->close();
  if (mdiArea_->subWindowList().size() == 0)
  {
    saveAction_->setDisabled(true);
    saveAsAction_->setDisabled(true);
    closeAction_->setDisabled(true);
    exportAction_->setDisabled(true);
    saveAction_->setDisabled(true);
  }
  updateTreesTable();
}

void PhyView::updateTreesTable()
{
  // Update tree list:
  treesTable_->clearSelection();
  treesTable_->clearContents();
  QList<QMdiSubWindow*> lst = mdiArea_->subWindowList();
  treesTable_->setRowCount(lst.size());
  for (int i = 0; i < lst.size(); ++i)
  {
    auto doc = dynamic_cast<TreeSubWindow*>(lst[i])->getDocument();
    string docName = doc->getName();
    if (docName == "")
      docName = "Tree#" + TextTools::toString(i + 1);
    treesTable_->setItem(i, 0, new QTableWidgetItem(QtTools::toQt(docName)));
    treesTable_->setItem(i, 1, new QTableWidgetItem(QtTools::toQt(TextTools::toString<unsigned int>(doc->getTree()->getNumberOfLeaves()))));
  }
}

void PhyView::updateDataViewer(const TreeTemplate<Node>& tree, int nodeId)
{
  dataViewerTable_->clearSelection();
  dataViewerTable_->clearContents();
  const auto& node = *tree.getNode(nodeId);
  
  vector<string> propertyNames;
  TreeTemplateTools::getNodePropertyNames(node, propertyNames);
  dataViewerTable_->setColumnCount(propertyNames.size());

  auto ids = TreeTemplateTools::getNodesId(node);
  dataViewerTable_->setRowCount(ids.size());
  
  QStringList colHeader, rowHeader;
  for (const auto& id : ids)
    rowHeader.append(QString::number(id));  
  dataViewerTable_->setVerticalHeaderLabels(rowHeader);
  
  for (size_t i = 0; i < propertyNames.size(); ++i)
  {
    colHeader.append(QString(propertyNames[i].c_str()));
    map<int, const Clonable*> properties;
    TreeTemplateTools::getNodeProperties(node, propertyNames[i], properties);
    for (size_t j = 0; j < ids.size(); ++j)
    {
      auto property = properties[ids[j]];
      if (property)
      {
        dataViewerTable_->setItem(j, i, new QTableWidgetItem(
			      QtTools::toQt(dynamic_cast<const BppString*>(property)->toSTL())));
      }
    }
  }
  dataViewerTable_->setHorizontalHeaderLabels(colHeader);
}

void PhyView::exit()
{
  close();
}

void PhyView::aboutBpp()
{
  QMessageBox msgBox;
  msgBox.setText("Bio++ 3.0.0.");
  msgBox.setInformativeText("bpp-core 3.0.0\nbpp-seq 3.0.0.\nbpp-phyl 3.0.0.\nbpp-qt 3.0.0");
  msgBox.exec();
}

void PhyView::about()
{
  QMessageBox msgBox;
  msgBox.setText("This is Bio++ Phylogenetic Viewer version 3.0.0.");
  msgBox.setInformativeText("Julien Dutheil <dutheil@evolbio.mpg.de>.");
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
    try
    {
      submitCommand(new MidpointRootingCommand(getActiveDocument(), brlenMidpointRootingCriteria_->currentText().toStdString()));
    }
    catch (NodeException& ex)
    {
      QMessageBox::critical(this, tr("Oups..."), tr("Some branch do not have lengths."));
    }
}

void PhyView::deleteAllLengths()
{
  if (hasActiveDocument())
    submitCommand(new DeleteLengthCommand(getActiveDocument()));
}

void PhyView::deleteAllSupportValues()
{
  if (hasActiveDocument())
    submitCommand(new DeleteSupportValuesCommand(getActiveDocument()));
}

void PhyView::unresolveUncertainNodes()
{
  if (hasActiveDocument())
  {
    try
    {
      submitCommand(new UnresolveUnsupportedNodesCommand(getActiveDocument(), bootstrapThreshold_->value()));
    }
    catch (NodeException& ex)
    {
      QMessageBox::critical(this, tr("Oups..."), tr("An exception occurred while unresolving your tree!"));
    }
  }
}

void PhyView::translateNames()
{
  if (hasActiveDocument())
  {
    translateNameChooser_->translateTree(getActiveDocument()->tree());
  }
}

void PhyView::setNamesFromData()
{
  if (hasActiveDocument())
  {
    namesFromDataDialog_->setNamesFromData();
  }
}

void PhyView::uncollapseAll()
{
  if (hasActiveDocument())
  {
    auto ids = getActiveDocument()->tree().getNodesId();
    TreeCanvas& tc = getActiveSubWindow()->treeCanvas();
    auto& td = tc.treeDrawing();
    for (const auto& id : ids) {
      td.collapseNode(id, false);
    }
    tc.redraw();
  }
}

void PhyView::autoCollapse()
{
  if (hasActiveDocument())
  {
    collapseDialog_->collapse();
  }
}

void PhyView::controlerTakesAction()
{
  QList<QMdiSubWindow*> lst = mdiArea_->subWindowList();
  for (int i = 0; i < lst.size(); ++i)
  {
    dynamic_cast<TreeSubWindow*>(lst[i])->treeCanvas().redraw();
  }
}

void PhyView::attachData()
{
  dataFileDialog_->setAcceptMode(QFileDialog::AcceptOpen);
  if (dataFileDialog_->exec() == QDialog::Accepted)
  {
    QStringList path = dataFileDialog_->selectedFiles();
    string sep = ",";
    if (dataFileDialog_->selectedNameFilter() == dataFileFilters_[1])
      sep = "\t";
    ifstream file(path[0].toStdString().c_str(), ios::in);
    auto table = DataTable::read(file, sep);
    dataLoader_->load(*table);
  }
}

void PhyView::saveData()
{
  if (hasActiveDocument())
  {
    dataFileDialog_->setAcceptMode(QFileDialog::AcceptSave);
    if (dataFileDialog_->exec() == QDialog::Accepted)
    {
      QStringList path = dataFileDialog_->selectedFiles();
      string sep = ",";
      if (dataFileDialog_->selectedNameFilter() == dataFileFilters_[1])
        sep = "\t";

      getActiveSubWindow()->writeTableToFile(path[0].toStdString(), sep);
    }
  }
}

void PhyView::addData()
{
  if (hasActiveDocument())
  {
    bool ok;
    QString name = QInputDialog::getText(this, tr("Set property name"), tr("Property name"), QLineEdit::Normal, tr("New property"), &ok);
    if (ok)
      submitCommand(new AddDataCommand(getActiveDocument(), name));
  }
}

void PhyView::removeData()
{
  if (hasActiveDocument())
  {
    vector<string> tmp;
    TreeTemplateTools::getNodePropertyNames(getActiveDocument()->tree().rootNode(), tmp);
    if (tmp.size() == 0)
    {
      QMessageBox::information(this, tr("Warning"), tr("No removable data is attached to this tree."), QMessageBox::Cancel);
      return;
    }
    QStringList properties;
    for (size_t i = 0; i < tmp.size(); ++i)
    {
      properties.append(QtTools::toQt(tmp[i]));
    }
    bool ok;
    QString name = QInputDialog::getItem(this, tr("Get property name"), tr("Property name"), properties, 0, false, &ok);
    if (ok)
      submitCommand(new RemoveDataCommand(getActiveDocument(), name));
  }
}

void PhyView::renameData()
{
  if (hasActiveDocument())
  {
    vector<string> tmp;
    TreeTemplateTools::getNodePropertyNames(getActiveDocument()->tree().rootNode(), tmp);
    if (tmp.size() == 0)
    {
      QMessageBox::information(this, tr("Warning"), tr("No data which can be renamed is attached to this tree."), QMessageBox::Cancel);
      return;
    }
    QStringList properties;
    for (size_t i = 0; i < tmp.size(); ++i)
    {
      properties.append(QtTools::toQt(tmp[i]));
    }
    bool ok;
    QString fromName = QInputDialog::getItem(this, tr("Get property name"), tr("Property name"), properties, 0, false, &ok);
    if (ok)
    {
      QString toName = QInputDialog::getText(this, tr("Set property name"), tr("Property name"), QLineEdit::Normal, tr("New property"), &ok);
      if (ok)
      {
        submitCommand(new RenameDataCommand(getActiveDocument(), fromName, toName));
      }
    }
  }
}

void PhyView::duplicateDownSelection()
{
  if (hasActiveDocument())
  {
    getActiveSubWindow()->duplicateDownSelection(1);
  }
}

void PhyView::snapData()
{
  if (hasActiveDocument())
  {
    submitCommand(new SnapCommand(getActiveDocument()));
  }
}


void PhyView::ancestralStateReconstruction()
{
  if (hasActiveDocument())
  {
    asrDialog_->asr();
  }
}



void PhyView::searchText()
{
  if (!getActiveSubWindow())
    return;
  getActiveSubWindow()->treeCanvas().redraw();
  clearSearchResults();
  QList<QGraphicsTextItem*> results = getActiveSubWindow()->treeCanvas().searchText(searchText_->text());
  for (int i = 0; i < results.size(); ++i)
  {
    searchResults_->addItem(results[i]->toPlainText());
    searchResultsItems_.append(results[i]);
    results[i]->setDefaultTextColor(Qt::red);
  }
}

void PhyView::searchResultSelected()
{
  getActiveSubWindow()->treeCanvas().ensureVisible(searchResultsItems_[searchResults_->currentRow()]);
}

void PhyView::activateSelectedDocument()
{
  if (treesTable_->selectedItems().size() > 0)
  {
    int index = treesTable_->selectedItems()[0]->row();
    mdiArea_->setActiveSubWindow(mdiArea_->subWindowList()[index]);
    mdiArea_->activeSubWindow()->showNormal();
    mdiArea_->activeSubWindow()->raise();
  }
}


std::shared_ptr<TreeTemplate<Node>> PhyView::pickTree()
{
  auto documents = getDocuments();
  // treeList_->clear();
  QStringList items;
  for (int i = 0; i < documents.size(); ++i)
  {
    QString text = QtTools::toQt(documents[i]->getName());
    if (text == "")
      text = "(unknown)";
    vector<string> leaves = documents[i]->tree().getLeavesNames();
    text += QtTools::toQt(" " + TextTools::toString(leaves.size()) + " leaves ");

    for (unsigned int j = 0; j < min(static_cast < unsigned int > (leaves.size()), 5u); ++j)
    {
      text += QtTools::toQt(", " + leaves[j]);
    }
    if (leaves.size() >= 5)
      text += "...";
    // treeList_->addItem(text);
    items << text;
  }

  // treeChooser_->exec();
  // int index = treeList_->currentRow();
  // return index > 0 ? documents[index]->getTree() : 0;
  bool ok;
  QString item = QInputDialog::getItem(this, "Pick a tree", "Tree to insert:", items, 0, false, &ok);
  if (ok && !item.isEmpty())
    return documents[items.indexOf(item)]->getTree();
  else
    return 0;
}


// This class is necessary to reimplement the notify method, in order to catch any foreign exception.
class PhyViewApplication :
  public QApplication
{
public:
  PhyViewApplication(int& argc, char* argv[]) :
    QApplication(argc, argv) {}

public:
  bool notify(QObject* receiver_, QEvent* event_)
  {
    try
    {
      return QApplication::notify(receiver_, event_);
    }
    catch (std::exception& ex)
    {
      std::cerr << "std::exception was caught" << std::endl;
      std::cerr << ex.what() << endl;
      QMessageBox msgBox;
      msgBox.setText(ex.what());
      msgBox.exec();
    }
    return false;
  }
};

int main(int argc, char* argv[])
{
  PhyViewApplication app(argc, argv);

  PhyView* phyview = new PhyView();
  phyview->show();

  // Parse command line arguments:
  QStringList args = app.arguments();
  string format = IOTreeFactory::NEWICK_FORMAT;
  // QTextCodec* codec = QTextCodec::codecForLocale(); Not supported in Qt5...
  for (int i = 1; i < args.size(); ++i)
  {
    if (args[i] == "--nhx")
    {
      format = IOTreeFactory::NHX_FORMAT;
    }
    else if (args[i] == "--nexus")
    {
      format = IOTreeFactory::NEWICK_FORMAT;
    }
    else if (args[i] == "--newick")
    {
      format = IOTreeFactory::NEWICK_FORMAT;
      // } else if (args[i] == "--enc") {
      //  if (i == args.size() - 1) {
      //    cerr << "You must specify a text encoding after --enc tag." << endl;
      //    exit(1);
      //  }
      //  ++i;
      //  codec = QTextCodec::codecForName(args[i].toStdString().c_str());
    }
    else
    {
      phyview->readTree(args[i], format);
    }
  }
  return app.exec();
}
