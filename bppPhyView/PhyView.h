
#include "TreeSubWindow.h"

//From Qt:
#include <QWidget>
#include <QPaintEvent>
#include <QMainWindow>
#include <QFileDialog>
#include <QScrollArea>
#include <QMdiArea>

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
    QMenu* viewMenu_;
    QMenu* helpMenu_;
    QAction* openAction_;
    QAction* saveAction_;
    QAction* saveAsAction_;
    QAction* closeAction_;
    QAction* exitAction_;
    QAction* aboutQtAction_;

    QMdiArea* mdiArea_;
    QFileDialog* fileDialog_;
    TreeCanvasControlers* treeControlers_;
    QWidget* controlPanel_;
    TreeStatisticsBox* statsPanel_;

    QDockWidget* statsDockWidget_; 
    QDockWidget* controlsDockWidget_;
    
  public:
    PhyView();

  protected:
    void closeEvent(QCloseEvent* event);

  private slots:
    void open();
    bool save();
    bool saveAs();
    void close();
    void exit();
    void about();
    void updateStatusBar();
    void setCurrentSubWindow(TreeSubWindow* tsw);
    void setCurrentSubWindow(QMdiSubWindow *msw)
    {
      TreeSubWindow* tsw = dynamic_cast<TreeSubWindow*>(msw);
      if (tsw) setCurrentSubWindow(tsw);
    }

  private:
    void initGui_();
    void createActions_();
    void createMenus_();
    void createStatusBar_();

};




int main(int argc, char *argv[]);

