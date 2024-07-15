// SPDX-FileCopyrightText: The Bio++ Development Group
//
// SPDX-License-Identifier: CECILL-2.1

#include "TreeSubWindow.h"
#include "TreeCommands.h"

// From Qt:
#include <QWidget>
#include <QPaintEvent>
#include <QMainWindow>
#include <QFileDialog>
#include <QMdiArea>
#include <QUndoGroup>
#include <QDialog>
#include <QListWidget>
#include <QRadioButton>
#include <QPrinter>
#include <QPrintDialog>

class QAction;
class QLabel;

#include <Bpp/Phyl/Graphics/TreeDrawing.h>
#include <Bpp/Phyl/Io/IoTreeFactory.h>
#include <Bpp/Qt/Tree/TreeCanvas.h>
#include <Bpp/Qt/Tree/TreeCanvasControlers.h>
#include <Bpp/Qt/Tree/TreeStatisticsBox.h>

using namespace bpp;

class PhyView;

class MouseActionListener :
  public MouseAdapter
{
private:
  PhyView* phyview_;
  QDialog* treeChooser_;
  QListWidget* treeList_;

public:
  MouseActionListener(PhyView* phyview);

  MouseActionListener* clone() const { return new MouseActionListener(*this); }

  void mousePressEvent(QMouseEvent* event);

  bool isAutonomous() const { return false; }
};


class TranslateNameChooser :
  public QDialog
{
  Q_OBJECT

private:
  PhyView* phyview_;
  QFileDialog* fileDialog_;
  QStringList fileFilters_;
  QComboBox* fromList_, * toList_;
  QCheckBox* hasHeader_;
  QPushButton* ok_, * cancel_;

public:
  TranslateNameChooser(PhyView* phyview);

  ~TranslateNameChooser()
  {
    delete fileDialog_;
  }

public:
  void translateTree(TreeTemplate<Node>& tree);
};


class NamesFromDataDialog :
  public QDialog
{
  Q_OBJECT

private:
  PhyView* phyview_;
  QComboBox* variableCol_;
  QCheckBox* innerNodesOnly_;
  QPushButton* ok_, * cancel_;

public:
  NamesFromDataDialog(PhyView* phyview);

  ~NamesFromDataDialog() {}

public:
  void setNamesFromData();
};


class DataLoader :
  public QDialog
{
  Q_OBJECT

private:
  PhyView* phyview_;
  QRadioButton* idIndex_, * nameIndex_;
  QComboBox* indexCol_;
  QPushButton* ok_, * cancel_;

public:
  DataLoader(PhyView* phyview);

  ~DataLoader() {}

public:
  void load(const DataTable& data);

private:
  void addProperties_(Node* node, const DataTable& data);
};


class AsrDialog :
  public QDialog
{
  Q_OBJECT

private:
  PhyView* phyview_;
  QComboBox* variableCol_, * asrMethod_;
  QPushButton* ok_, * cancel_;

public:
  AsrDialog(PhyView* phyview);

  ~AsrDialog() {}

  void asr();
};


class ImageExportDialog :
  public QDialog
{
  Q_OBJECT

private:
  PhyView* phyview_;
  QLabel* path_;
  QSpinBox* width_, * height_;
  QCheckBox* transparent_, * keepAspectRatio_;
  QPushButton* ok_, * cancel_, * browse_;
  QFileDialog* imageFileDialog_;
  QStringList imageFileFilters_;

public:
  ImageExportDialog(PhyView* phyview);

  ~ImageExportDialog() {}

public:
  void process(QGraphicsScene* scene);

public slots:
  void chosePath();
};


class TypeNumberDialog :
  public QDialog
{
  Q_OBJECT

private:
  QSpinBox* spinBox_;
  QPushButton* ok_, * cancel_;

public:
  TypeNumberDialog(PhyView* phyview, const string& what, unsigned int min, unsigned int max);

  ~TypeNumberDialog() {}

public:
  unsigned int getValue() const { return spinBox_->value(); }
};


class CollapseDialog :
  public QDialog
{
  Q_OBJECT

private:
  PhyView* phyview_;
  QComboBox* variableCol_;
  QPushButton* ok_, * cancel_;

public:
  CollapseDialog(PhyView* phyview);

  ~CollapseDialog() {}

public:
  void collapse();
  
private:
  std::string scan_(TreeCanvas& tc, const Node& node, const std::string& propertyName);
  
};


class PhyView :
  public QMainWindow,
  public TreeCanvasControlersListener
{
  Q_OBJECT

private:
  QMenu* fileMenu_;
  QMenu* editMenu_;
  QMenu* viewMenu_;
  QMenu* helpMenu_;
  QAction* openAction_;
  QAction* saveAction_;
  QAction* saveAsAction_;
  QAction* closeAction_;
  QAction* printAction_;
  QAction* exportAction_;
  QAction* exitAction_;
  QAction* cascadeWinAction_;
  QAction* tileWinAction_;
  QAction* aboutAction_;
  QAction* aboutBppAction_;
  QAction* aboutQtAction_;
  QAction* undoAction_;
  QAction* redoAction_;

  QUndoGroup manager_;

  QMdiArea* mdiArea_;
  QFileDialog* treeFileDialog_;
  QStringList treeFileFilters_;
  QFileDialog* dataFileDialog_;
  QStringList dataFileFilters_;
  IOTreeFactory ioTreeFactory_;
  QPrinter* printer_;
  QPrintDialog* printDialog_;
  TreeCanvasControlers* treeControlers_;
  QWidget* displayPanel_;
  TreeStatisticsBox* statsBox_;
  QWidget* treesPanel_;
  QWidget* statsPanel_;
  QWidget* brlenPanel_;
  QWidget* mouseControlPanel_;
  QWidget* dataPanel_;
  QWidget* searchPanel_;

  QDockWidget* treesDockWidget_;
  QDockWidget* statsDockWidget_;
  QDockWidget* displayDockWidget_;
  QDockWidget* undoDockWidget_;

  // Trees:
  QTableWidget* treesTable_;

  // Branch lengths operations:
  QDockWidget* brlenDockWidget_;
  QDoubleSpinBox* brlenSetLengths_;
  QDoubleSpinBox* brlenComputeGrafen_;
  QComboBox* brlenMidpointRootingCriteria_;
  QDoubleSpinBox* bootstrapThreshold_;

  // Mouse actions change:
  QDockWidget* mouseControlDockWidget_;
  QComboBox* leftButton_;
  QComboBox* middleButton_;
  QComboBox* rightButton_;

  // Data operations:
  QDockWidget* dataDockWidget_;
  QPushButton* loadData_;
  QPushButton* saveData_;
  QPushButton* addData_;
  QPushButton* removeData_;
  QPushButton* renameData_;
  QPushButton* translateNames_;
  QPushButton* setNamesFromData_;
  QPushButton* duplicateDownSelection_;
  QPushButton* snapData_;
  QPushButton* asr_;
  QPushButton* uncollapseAll_;
  QPushButton* autoCollapse_;

  // Searching:
  QDockWidget* searchDockWidget_;
  QLineEdit*   searchText_;
  QListWidget* searchResults_;

  LabelCollapsedNodesTreeDrawingListener collapsedNodesListener_;

  TranslateNameChooser* translateNameChooser_;
  
  NamesFromDataDialog* namesFromDataDialog_;

  DataLoader* dataLoader_;

  CollapseDialog* collapseDialog_;

  AsrDialog* asrDialog_;
  
  ImageExportDialog* imageExportDialog_;

  QList<QGraphicsTextItem*> searchResultsItems_;

public:
  PhyView();

public:
  bool hasActiveDocument() const
  {
    return mdiArea_->currentSubWindow() != 0;
  }

  std::shared_ptr<TreeDocument> getActiveDocument()
  {
    return dynamic_cast<TreeSubWindow*>(mdiArea_->currentSubWindow())->getDocument();
  }

  QList<std::shared_ptr<TreeDocument>> getDocuments();

  QList<std::shared_ptr<TreeDocument>> getNonActiveDocuments();

  TreeSubWindow* getActiveSubWindow()
  {
    return dynamic_cast<TreeSubWindow*>(mdiArea_->currentSubWindow());
  }

  void submitCommand(QUndoCommand* cmd)
  {
    manager_.activeStack()->push(cmd);
  }

  std::shared_ptr<TreeDocument> createNewDocument(Tree* tree);

  MouseActionListener* getMouseActionListener()
  {
    return new MouseActionListener(this);
  }

  QString getMouseLeftButtonActionType() const { return leftButton_->currentText(); }
  QString getMouseMiddleButtonActionType() const { return middleButton_->currentText(); }
  QString getMouseRightButtonActionType() const { return rightButton_->currentText(); }

  void controlerTakesAction();

  void readTree(const QString& path, const string& format);

  std::shared_ptr<TreeTemplate<Node>> pickTree();

  void checkLastWindow()
  {
    // This is to avoid bugs when the last window is closed.
    // It should only be closed from the destructor of TreeSubWindow.
    if (mdiArea_->subWindowList().size() == 0)
    {
      treesTable_->clearContents();
      treesTable_->setRowCount(0);
    }
  }

protected:
  void closeEvent(QCloseEvent* event);

public slots:
  void updateTreesTable();
  void clearSearchResults()
  {
    searchResults_->clear();
    searchResultsItems_.clear();
  }

private slots:
  void openTree();
  bool saveTree();
  bool saveTreeAs();
  void closeTree();
  void exportTree();
  void printTree();
  void exit();
  void about();
  void aboutBpp();
  void updateStatusBar();
  void setCurrentSubWindow(TreeSubWindow* tsw);
  void setCurrentSubWindow(QMdiSubWindow* msw)
  {
    TreeSubWindow* tsw = dynamic_cast<TreeSubWindow*>(msw);
    if (tsw) setCurrentSubWindow(tsw);
  }

  void updateStatistics()
  {
    statsBox_->updateTree(*getActiveDocument()->getTree());
  }
  void setLengths();
  void initLengthsGrafen();
  void computeLengthsGrafen();
  void convertToClockTree();
  void midpointRooting();
  void deleteAllLengths();
  void deleteAllSupportValues();
  void unresolveUncertainNodes();
  void translateNames();
  void setNamesFromData();
  void uncollapseAll();
  void autoCollapse();

  void attachData();
  void saveData();
  void addData();
  void removeData();
  void renameData();
  void duplicateDownSelection();
  void snapData();
  void ancestralStateReconstruction();
  void searchText();
  void searchResultSelected();
  void activateSelectedDocument();

private:
  void initGui_();
  void createActions_();
  void createMenus_();
  void createStatusBar_();

  void createTreesPanel_();
  void createStatsPanel_();
  void createDisplayPanel_();
  void createBrlenPanel_();
  void createMouseControlPanel_();
  void createDataPanel_();
  void createSearchPanel_();
};


int main(int argc, char* argv[]);
