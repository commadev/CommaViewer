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
    ui->imgQuit->setPixmap(QPixmap(":/imgs/x.png")); //임시

    //키이벤트 -> Previous, Next버튼 호출
    new QShortcut(QKeySequence(Qt::Key_Left), this, SLOT(on_btnPrevious_clicked()));
    new QShortcut(QKeySequence(Qt::Key_Right), this, SLOT(on_btnNext_clicked()));
    new QShortcut(QKeySequence(Qt::Key_F7), this, SLOT(view()));
    new QShortcut(QKeySequence(Qt::Key_F8), this, SLOT(unview()));

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
        ui->imageView->setMovie(mov);
        //이미지 창에 맞춤
        ui->imageView->setScaledContents(false);

        //임시로 사이즈변경을 위해 Pixmap으로도 불러옴
        buf.load(targetFile.filePath());

        mov->start();
        bufSize = buf.size();

    }
    else
    {
        img.load(targetFile.filePath());
        buf = QPixmap::fromImage(img);

        ui->imageView->setPixmap(buf);
        ui->imageView->setScaledContents(false);

        //이미지를 창에맞춤 비율유지
        ui->imageView->setPixmap(buf.scaled(this->width() - 30 , this->height() - 30, Qt::KeepAspectRatio));
        bufSize = buf.size();

    }


}

//리사이즈 이벤트 -> 레이아웃 크기 리사이징

void MainWindow::resizeEvent(QResizeEvent *)
{

    ui->verticalLayoutWidget->setGeometry(QRect(0,30,this->geometry().width(),this->geometry().height()-50));

    ui->btnQuit->setGeometry(this->geometry().width()-30,0,30,30); //임시
    ui->imgQuit->setGeometry(this->geometry().width()-30,0,30,30); //임시

    ui->imageView->setGeometry(0,20,this->geometry().width(),this->geometry().height()-30);
    ui->btnPrevious->setGeometry(0, 30, this->geometry().width()/2, this->geometry().height()-50);
    ui->btnNext->setGeometry(this->geometry().width()/2, 30, this->geometry().width()/2, this->geometry().height()-50);
    ui->txtTitle->setGeometry(0, 0, 200, 30);

    if(mov != nullptr)
    {
        if(mov->scaledSize().width() + 30 > ui->verticalLayoutWidget->width() || mov->scaledSize().height() + 30 > ui->verticalLayoutWidget->height())
        {
            bufSize.scale(this->width() - 30 , this->height() - 30, Qt::KeepAspectRatio);
            mov->setScaledSize(bufSize);
            //mov->setScaledSize(QSize(QSize().scale(this->width() - 30 , this->height() - 30, Qt::KeepAspectRatio)));
            ui->imageView->setMovie(mov);
        }
    }
    else if(buf.width() + 30 > ui->verticalLayoutWidget->width() || buf.height() + 30 > ui->verticalLayoutWidget->height())
    {
        ui->imageView->setPixmap(buf.scaled(this->width() - 30 , this->height() - 30, Qt::KeepAspectRatio));
    }

}


//휠 이벤트 -> Pixmap 사이즈 조정
void MainWindow::wheelEvent(QWheelEvent *event)
{

    QPoint numDegrees = event->angleDelta() / 8;

    if(numDegrees.ry() > 0 && (ui->verticalLayoutWidget->width() < bufSize.width() +30 || ui->verticalLayoutWidget->height() < bufSize.height()+30) )
    {
        buf.size() = ui->verticalLayoutWidget->size();
    }
    else if (!numDegrees.isNull())
    {

        bufSize.scale(bufSize.width() + numDegrees.ry(),bufSize.height() + numDegrees.ry(),Qt::KeepAspectRatio);
        if(mov != nullptr)
        {
            mov->setScaledSize(bufSize);
            ui->imageView->setMovie(mov);
        }
        else
        {
            ui->imageView->setPixmap(buf.scaled(bufSize));
        }
    }
    event->accept();
}







//F7 F8
void MainWindow::view()
{
    ui->txtTitle->hide();
    ui->statusBar->hide();
    ui->btnQuit->hide();
    ui->imgQuit->hide();
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
    ui->btnQuit->show();
    ui->imgQuit->show();
    ui->btnNext->show();
    ui->btnPrevious->show();

    setAttribute(Qt::WA_TranslucentBackground,false);
    setAttribute(Qt::WA_NoSystemBackground,false);
    repaint();
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

void MainWindow::on_btnQuit_clicked()
{
    this->close();
}
