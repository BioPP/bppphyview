//
// File: PhyView.h
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

#include "TreeSubWindow.h"
#include "TreeCommands.h"

//From Qt:
#include <QWidget>
#include <QPaintEvent>
#include <QMainWindow>
#include <QFileDialog>
#include <QMdiArea>
#include <QUndoGroup>
#include <QDialog>
#include <QListWidget>
#include <QRadioButton>

class QAction;
class QLabel;

#include <Bpp/Phyl/Graphics/TreeDrawing.h>
#include <Bpp/Phyl/Io/IoTreeFactory.h>
#include <Bpp/Qt/Tree/TreeCanvas.h>
#include <Bpp/Qt/Tree/TreeCanvasControlers.h>
#include <Bpp/Qt/Tree/TreeStatisticsBox.h>

using namespace bpp;

class PhyView;

class MouseActionListener:
  public MouseAdapter
{
  private:
    PhyView* phyview_;
    QDialog* treeChooser_;
    QListWidget* treeList_;

  public:
    MouseActionListener(PhyView* phyview);

    MouseActionListener* clone() const { return new MouseActionListener(*this); }
    
    void mousePressEvent(QMouseEvent *event);

    bool isAutonomous() const { return false; }

  private:
    TreeTemplate<Node>* pickTree_();

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
    QPushButton* ok_, *cancel_;

  public:
    TranslateNameChooser(PhyView* phyview);

    ~TranslateNameChooser()
    {
      delete fileDialog_;
    }

  public:
    void translateTree(TreeTemplate<Node>& tree);
};



class DataLoader :
  public QDialog
{
  Q_OBJECT

  private:
    PhyView* phyview_;
    QRadioButton* idIndex_, * nameIndex_;
    QComboBox* indexCol_;
    QPushButton* ok_, *cancel_;

  public:
    DataLoader(PhyView* phyview);

    ~DataLoader()
    {
    }

  public:
    void load(const DataTable* data);

  private:
    void addProperties_(Node* node, const DataTable& data);
};



class TypeNumberDialog :
  public QDialog
{
  Q_OBJECT

  private:
    QSpinBox* spinBox_;
    QPushButton* ok_, *cancel_;

  public:
    TypeNumberDialog(PhyView* phyview, const string& what, unsigned int min, unsigned int max);

    ~TypeNumberDialog()
    {
    }

  public:
    unsigned int getValue() const { return spinBox_->value(); }
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
    TreeCanvasControlers* treeControlers_;
    QWidget* displayPanel_;
    TreeStatisticsBox* statsBox_;
    QWidget* statsPanel_;
    QWidget* brlenPanel_;
    QWidget* mouseControlPanel_;
    QWidget* dataPanel_;

    QDockWidget* statsDockWidget_; 
    QDockWidget* displayDockWidget_;
    QDockWidget* undoDockWidget_;
    
    //Branch lengths operations:
    QDockWidget* brlenDockWidget_;
    QDoubleSpinBox* brlenSetLengths_;
    QDoubleSpinBox* brlenComputeGrafen_;

    //Mouse actions change:
    QDockWidget* mouseControlDockWidget_;
    QComboBox* leftButton_;
    QComboBox* middleButton_;
    QComboBox* rightButton_;

    //Names operations:
    QDockWidget* dataDockWidget_;
    QPushButton* translateNames_;
    QPushButton* loadData_;

    LabelCollapsedNodesTreeDrawingListener collapsedNodesListener_;

    TranslateNameChooser* translateNameChooser_;

    DataLoader* dataLoader_;

  public:
    PhyView();

  public:
    bool hasActiveDocument() const
    {
      return mdiArea_->currentSubWindow() != 0;
    }

    TreeDocument* getActiveDocument()
    {
      return dynamic_cast<TreeSubWindow*>(mdiArea_->currentSubWindow())->getDocument();
    }

    QList<TreeDocument*> getNonActiveDocuments();

    TreeSubWindow* getActiveSubWindow()
    {
      return dynamic_cast<TreeSubWindow*>(mdiArea_->currentSubWindow());
    }

    void submitCommand(QUndoCommand* cmd)
    {
      manager_.activeStack()->push(cmd);
    }

    TreeDocument* createNewDocument(Tree* tree);

    MouseActionListener* getMouseActionListener()
    {
      return new MouseActionListener(this);
    }

    QString getMouseLeftButtonActionType() const { return leftButton_->currentText(); }
    QString getMouseMiddleButtonActionType() const { return middleButton_->currentText(); }
    QString getMouseRightButtonActionType() const { return rightButton_->currentText(); }

    void controlerTakesAction();

  protected:
    void closeEvent(QCloseEvent* event);

  private slots:
    void openTree();
    bool saveTree();
    bool saveTreeAs();
    void closeTree();
    void exit();
    void about();
    void aboutBpp();
    void updateStatusBar();
    void setCurrentSubWindow(TreeSubWindow* tsw);
    void setCurrentSubWindow(QMdiSubWindow *msw)
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
    void translateNames();

    void attachData();

  private:
    void initGui_();
    void createActions_();
    void createMenus_();
    void createStatusBar_();

    void createStatsPanel_();
    void createDisplayPanel_();
    void createBrlenPanel_();
    void createMouseControlPanel_();
    void createDataPanel_();

};




int main(int argc, char *argv[]);

