#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileInfo>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(int isExist, char* filePath[], QWidget *parent = nullptr);
    ~MainWindow();





protected:
    void resizeEvent(QResizeEvent *event);
    void wheelEvent(QWheelEvent *event);
    void setImage(QFileInfo targetFile);

    int m_nMouseClick_X_Coordinate;
    int m_nMouseClick_Y_Coordinate;
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);

    QPixmap bufP;
    QPixmap bufN;
    QPixmap bufS;



private slots:
    void on_btnPrevious_clicked();

    void on_btnNext_clicked();

    void view();

    void unview();


    void on_btnQuit_clicked();

private:
    Ui::MainWindow *ui;
    QMovie *mov = nullptr;
    QPixmap buf;
    QFileInfo targetFile;
    QFileInfo targetFileP;
    QFileInfo targetFileN;
    QImage img;
    QSize bufSize;

};

#endif // MAINWINDOW_H
