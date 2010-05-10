
#include "TreeSubWindow.h"
#include "TreeCommands.h"

//From Qt:
#include <QWidget>
#include <QPaintEvent>
#include <QMainWindow>
#include <QFileDialog>
#include <QScrollArea>
#include <QMdiArea>
#include <QUndoGroup>

class QAction;
class QLabel;

#include <Phyl/TreeDrawing.h>
#include <Bpp/Qt/TreeCanvas.h>
#include <Bpp/Qt/TreeCanvasControlers.h>
#include <Bpp/Qt/TreeStatisticsBox.h>

using namespace bpp;

class PhyView :
  public QMainWindow
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
    TreeCanvasControlers* treeControlers_;
    QWidget* displayPanel_;
    TreeStatisticsBox* statsPanel_;
    QWidget* brlenPanel_;

    QDockWidget* statsDockWidget_; 
    QDockWidget* displayDockWidget_;
    
    //Branch lengths operations:
    QDockWidget* brlenDockWidget_;
    QDoubleSpinBox* brlenSetLengths_;
    


    
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

    void submitCommand(QUndoCommand* cmd)
    {
      manager_.activeStack()->push(cmd);
    }

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

    void setLengths();

  private:
    void initGui_();
    void createActions_();
    void createMenus_();
    void createStatusBar_();

};




int main(int argc, char *argv[]);

