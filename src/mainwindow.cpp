#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QImage>
#include <QPixmap>
#include <QFileDialog>
#include <QDir>
#include <QStringList>
#include <QShortcut>
#include <QWheelEvent>
#include <QMovie>


MainWindow::MainWindow(int isExist, char* filePath[], QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    //타이틀바 제거

    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground,true);
    setAttribute(Qt::WA_NoSystemBackground,false);
    ui->verticalLayout->setAlignment(Qt::AlignHCenter); //중앙정렬


    //버튼 커서아이콘 어플리케이션패스 + 파일명
    ui->btnNext->setCursor(QPixmap(":/imgs/right.png"));
    ui->btnPrevious->setCursor(QPixmap(":/imgs/left.png"));
    ui->imgClose->setPixmap(QPixmap(":/imgs/x.png")); //임시
    ui->imgMaximize->setPixmap(QPixmap(":/imgs/maximize.png")); //임시
    ui->imgMinimize->setPixmap(QPixmap(":/imgs/minimize.png")); //임시
    ui->imgFullScreen->setPixmap(QPixmap(":/imgs/fullscreen.png")); //임시
    ui->imgList->setPixmap(QPixmap(":/imgs/list.png")); //임시

    //키이벤트 -> Previous, Next버튼 호출
    new QShortcut(QKeySequence(Qt::Key_Left), this, SLOT(on_btnPrevious_clicked()));
    new QShortcut(QKeySequence(Qt::Key_Right), this, SLOT(on_btnNext_clicked()));
    new QShortcut(QKeySequence(Qt::Key_F7), this, SLOT(view()));
    new QShortcut(QKeySequence(Qt::Key_F8), this, SLOT(unview()));
    new QShortcut(QKeySequence(Qt::Key_F6), this, SLOT(alwaysOnTop()));


    //argc가 1이면 (인자없이 실행되면) 다이얼로그 실행
    if (isExist == 1)
    {
        printf_s("ss");
        QString fileURL = QFileDialog::getOpenFileName(this, tr("Choose a Picture"), tr("./test"), tr("Images (*.bmp *.png *.jpeg *.jpg, *.gif)"));

        if (fileURL == nullptr)
        {
            this->close();
        }
        else
        {
            targetFile.setFile(fileURL);
            setImage(targetFile);
        }
    }
    else
    {
        targetFile.setFile(filePath[1]); //argv[1]에서 받아온 인자값을 target으로
        setImage(targetFile);
    }

}


MainWindow::~MainWindow()
{
    delete ui;
}



bool static isMouseDown = false; //스태틱으로

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
       m_nMouseClick_X_Coordinate = event->x();
       m_nMouseClick_Y_Coordinate = event->y();
       isMouseDown = true;
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
   isMouseDown = false;
   Q_UNUSED(event); //이벤트 사용안할때 이렇게함
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (isMouseDown == true) {
       //SettingData setData;
       move(event->globalX()-m_nMouseClick_X_Coordinate,event->globalY()-m_nMouseClick_Y_Coordinate);
       //setData.pos_x = event->globalX()-m_nMouseClick_X_Coordinate;
       //setData.pos_y = event->globalY()-m_nMouseClick_Y_Coordinate;
       //setData.SAVE();
    }
}


//setImage함수정의
void MainWindow::setImage(QFileInfo targetFile)
{
    if(mov != nullptr)
    {
        mov->stop();
        mov = nullptr;
    }

    if(targetFile.suffix() == "gif")
    {
        mov = new QMovie(targetFile.filePath());
        buf.load(targetFile.filePath());
        ui->imageView->setFixedSize(buf.size());

        ui->imageView->setMovie(mov);


        mov->start();
        bufSize = buf.size();
        ui->imageView->setScaledContents(true);
    }
    else
    {
        img.load(targetFile.filePath());
        buf = QPixmap::fromImage(img);

        ui->imageView->setFixedSize(img.size());
        ui->imageView->setPixmap(buf);
        bufSize = buf.size();
        ui->imageView->setScaledContents(true);

        //이미지를 창에맞춤 비율유지
        //ui->imageView->setPixmap(buf.scaled(this->width() - 30 , this->height() - 30, Qt::KeepAspectRatio));
    }


    bufResize = bufSize;
    if(mov != nullptr)
    {
        if(bufSize.width() + 30 > ui->verticalFrame->width() || bufSize.height() + 30 > ui->verticalFrame->height())
        {
            bufResize.scale(ui->verticalFrame->width() - 30 , ui->verticalFrame->height() - 30, Qt::KeepAspectRatio);
            ui->imageView->setFixedSize(bufResize);
            ui->imageView->setMovie(mov);
        }
    }
    else if(bufSize.width() + 30 > ui->verticalFrame->width() || bufSize.height() + 30 > ui->verticalFrame->height())
    {
        bufResize.scale(ui->verticalFrame->width() - 30, ui->verticalFrame->height() - 30, Qt::KeepAspectRatio);
        ui->imageView->setFixedSize(bufResize);
        ui->imageView->setPixmap(buf);
        //ui->imageView->setPixmap(buf.scaled(this->width() - 30 , this->height() - 30, Qt::KeepAspectRatio));
    }

}

//리사이즈 이벤트 -> 레이아웃 크기 리사이징

void MainWindow::resizeEvent(QResizeEvent *)
{

    bufResize = bufSize;
    //ui->verticalLayoutWidget->setGeometry(QRect(0,30,this->geometry().width(),this->geometry().height()-50));
    ui->verticalFrame->setGeometry(0,20,this->geometry().width(),this->geometry().height()-20);
    //ui->imageView->setGeometry(0,20,this->geometry().width(),this->geometry().height()-30);

    ui->btnClose->setGeometry(this->geometry().width()-20,0,20,20); //임시
    ui->imgClose->setGeometry(this->geometry().width()-20,0,20,20); //임시
    ui->btnMaximize->setGeometry(this->geometry().width()-40,0,20,20); //임시
    ui->imgMaximize->setGeometry(this->geometry().width()-40,0,20,20); //임시
    ui->btnMinimize->setGeometry(this->geometry().width()-60,0,20,20); //임시
    ui->imgMinimize->setGeometry(this->geometry().width()-60,0,20,20); //임시
    ui->btnFullScreen->setGeometry(this->geometry().width()-80,0,20,20);
    ui->imgFullScreen->setGeometry(this->geometry().width()-80,0,20,20); //임시
    ui->btnList->setGeometry(this->geometry().width()-100,0,20,20); //임시
    ui->imgList->setGeometry(this->geometry().width()-100,0,20,20); //임시


    //ui->imageView->setGeometry(0,20,this->geometry().width(),this->geometry().height()-30);
    ui->btnPrevious->setGeometry(0, 30, this->geometry().width()/2, this->geometry().height()-50);
    ui->btnNext->setGeometry(this->geometry().width()/2, 30, this->geometry().width()/2, this->geometry().height()-50);
    ui->txtTitle->setGeometry(0, 0, 200, 30);

    if(mov != nullptr)
    {
        if(bufSize.width() + 30 > ui->verticalFrame->width() || bufSize.height() + 30 > ui->verticalFrame->height())
        {
            bufResize.scale(ui->verticalFrame->width() - 30 , ui->verticalFrame->height() - 30, Qt::KeepAspectRatio);
            ui->imageView->setFixedSize(bufResize);
            ui->imageView->setMovie(mov);
        }
    }
    else if(bufSize.width() + 30 > ui->verticalFrame->width() || bufSize.height() + 30 > ui->verticalFrame->height())
    {
        bufResize.scale(ui->verticalFrame->width() - 30, ui->verticalFrame->height() - 30, Qt::KeepAspectRatio);
        ui->imageView->setFixedSize(bufResize);
        ui->imageView->setPixmap(buf);
        //ui->imageView->setPixmap(buf.scaled(this->width() - 30 , this->height() - 30, Qt::KeepAspectRatio));
    }

}


//휠 이벤트 -> Pixmap 사이즈 조정
void MainWindow::wheelEvent(QWheelEvent *event)
{

    QPoint numDegrees = event->angleDelta() / 8;

    bufSize.scale(ui->imageView->size().width() + numDegrees.ry(), ui->imageView->size().height() + numDegrees.ry(),Qt::KeepAspectRatio);
    ui->imageView->setFixedSize(bufSize);
    /*
    if(numDegrees.ry() > 0 && (ui->verticalFrame->width() < ui->imageView->size().width() +30 || ui->verticalFrame->height() < ui->imageView->size().height()+30) )
    {

    }
    else if (!numDegrees.isNull())
    {

    }
    */
    event->accept();
}







//F7 F8
void MainWindow::view()
{
    ui->txtTitle->hide();
    ui->statusBar->hide();
    ui->btnClose->hide();
    ui->imgClose->hide();
    ui->btnMaximize->hide();
    ui->imgMaximize->hide();
    ui->btnMinimize->hide();
    ui->imgMinimize->hide();
    ui->btnFullScreen->hide();
    ui->imgFullScreen->hide();
    ui->btnList->hide();
    ui->imgList->hide();
    ui->btnNext->hide();
    ui->btnPrevious->hide();


    setAttribute(Qt::WA_TranslucentBackground,true);
    setAttribute(Qt::WA_NoSystemBackground,true);
    repaint();


}

void MainWindow::unview()
{
    ui->txtTitle->show();
    ui->statusBar->show();
    ui->btnClose->show();
    ui->imgClose->show();
    ui->btnMaximize->show();
    ui->imgMaximize->show();
    ui->btnMinimize->show();
    ui->imgMinimize->show();
    ui->btnFullScreen->hide();
    ui->imgFullScreen->hide();
    ui->btnList->show();
    ui->imgList->show();
    ui->btnNext->show();
    ui->btnPrevious->show();

    setAttribute(Qt::WA_TranslucentBackground,false);
    setAttribute(Qt::WA_NoSystemBackground,false);
    repaint();
}

void MainWindow::alwaysOnTop()
{

    Qt::WindowFlags wFlag = windowFlags();


    if (wFlag != -2012940287)
    {
        wFlag |= Qt:: WindowStaysOnTopHint;
        //ui->imageView->setText((QString)wFlag);

    }
    else
    {
        wFlag &= ~Qt:: WindowStaysOnTopHint;
    }
    setWindowFlags(wFlag);
    this->show();
}


//버튼목록
void MainWindow::on_btnPrevious_clicked()
{
    QDir targetFileDir(targetFile.path());
    QStringList dirFileList;



    //현재 dir 의 모든 이미지파일의 이름을 List에 저장한다
    dirFileList = targetFileDir.entryList(QStringList() << "*.jpg" << "*.JPG" << "*.png" << "*.PNG" << "*.gif" << "*.GIF",QDir::Files);


    if(dirFileList.indexOf(targetFile.fileName()) > 0)
    {
        //targetFile을 새로 설정한다 기존파일path + / + 이전파일 name
        //이전파일 = List내의 현재파일의 index -1
        targetFile.setFile(targetFile.path()+"/"+dirFileList.value(dirFileList.indexOf(targetFile.fileName())-1));
    }
    else
    {
        //targetFile의 index가 0이하이면 list의 마지막 파일로 돌아간다
        //마지막파일 = List의 길이 -1
        targetFile.setFile(targetFile.path()+"/"+dirFileList.value(dirFileList.length()-1));
    }
    setImage(targetFile);
}



void MainWindow::on_btnNext_clicked()
{
    QDir targetFileDir(targetFile.path());
    QStringList dirFileList;



    dirFileList = targetFileDir.entryList(QStringList() << "*.jpg" << "*.JPG" << "*.png" << "*.PNG" << "*.gif" << "*.GIF",QDir::Files);
    if(dirFileList.indexOf(targetFile.fileName()) < dirFileList.length()-1)
    {
        targetFile.setFile(targetFile.path()+"/"+dirFileList.value(dirFileList.indexOf(targetFile.fileName())+1));
    }
    else
    {
        targetFile.setFile(targetFile.path()+"/"+dirFileList.value(0));
    }
    setImage(targetFile);
}

void MainWindow::on_btnClose_clicked()
{
    this->close();
}
void MainWindow::on_btnMaximize_clicked()
{

    if(this->isMaximized())
    {
        this->showNormal();
    }
    else
    {
        this->showMaximized();
    }

}
void MainWindow::on_btnMinimize_clicked()
{
    this->showMinimized();
}

void MainWindow::on_btnFullScreen_clicked()
{
    this->showFullScreen();
}

void MainWindow::on_btnList_clicked()
{
    this->close();
}

