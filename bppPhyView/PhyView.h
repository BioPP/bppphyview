
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

class QAction;
class QLabel;

#include <Phyl/TreeDrawing.h>
#include <Phyl/IOTreeFactory.h>
#include <Bpp/Qt/TreeCanvas.h>
#include <Bpp/Qt/TreeCanvasControlers.h>
#include <Bpp/Qt/TreeStatisticsBox.h>

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
    QFileDialog* fileDialog_;
    QStringList fileFilters_;
    IOTreeFactory ioTreeFactory_;
    TreeCanvasControlers* treeControlers_;
    QWidget* displayPanel_;
    TreeStatisticsBox* statsBox_;
    QWidget* statsPanel_;
    QWidget* brlenPanel_;
    QWidget* mouseControlPanel_;
    QWidget* namesOperationsPanel_;

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
    QDockWidget* namesOperationsDockWidget_;
    QPushButton* translateNames_;

    LabelCollapsedNodesTreeDrawingListener collapsedNodesListener_;

    TranslateNameChooser* translateNameChooser_;

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

  private:
    void initGui_();
    void createActions_();
    void createMenus_();
    void createStatusBar_();

    void createStatsPanel_();
    void createDisplayPanel_();
    void createBrlenPanel_();
    void createMouseControlPanel_();
    void createNamesOperationsPanel_();

};




int main(int argc, char *argv[]);

