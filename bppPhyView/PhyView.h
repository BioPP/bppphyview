#include <QWidget>
#include <QPaintEvent>
#include <QMainWindow>
#include <QFileDialog>
#include <QScrollArea>

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
    QMenu* helpMenu_;
    QAction* openAction_;
    QAction* saveAction_;
    QAction* saveAsAction_;
    QAction* closeAction_;
    QAction* exitAction_;
    QAction* aboutQtAction_;

    QFileDialog* fileDialog_;
    TreeCanvas* treePanel_;
    TreeCanvasControlers* treeControlers_;
    QScrollArea* treePanelScrollArea_;
    QWidget* controlPanel_;
    TreeStatisticsBox* statsPanel_;

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

  private:
    void createActions_();
    void createMenus_();
    void createStatusBar_();

};




int main(int argc, char *argv[]);

