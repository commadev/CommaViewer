#include "imageitor.h"
#include "ui_imageitor.h"
#include "rgb_changer.h"
#include "image_resizer.h"
#include "option.h"

imageitor::imageitor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::imageitor)
{
    ui->setupUi(this);

    // Layout initialize
    ThemeSelect(config.Theme);
    // -----
    this->move(config.PosX, config.PosY);
    this->setGeometry(config.PosX, config.PosY, config.Width, config.Height);
    // -----
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),this, SLOT(ShowContextMenu(const QPoint&)));

    // Variable initialize
    ActivityPixmap = new QPixmap[config.UndoSize];
    ActivityCount = 0;
    StartPoint = 0;
    StopPoint = 0;
    nowPoint = 0;
    Cycle = false;
    runUndo = false;
    possibleRedo = false;
    scene = new paintScene(this);
    timer = new QTimer;

    // Shortcut initialize
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z), this, SLOT(Undo()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Y), this, SLOT(Redo()));
    // -----
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this, SLOT(on_actionOpen_triggered()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this, SLOT(on_actionSave_triggered()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S), this, SLOT(on_actionSave_as_triggered()));
    // -----
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), this, SLOT(on_actionRGB_triggered()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_R), this, SLOT(on_actionResizing_triggered()));
    // -----
    new QShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_O), this, SLOT(on_actionOption_triggered()));
    new QShortcut(QKeySequence(Qt::Key_F10), this, SLOT(on_actionInfo_triggered()));
    // -----
    new QShortcut(QKeySequence(Qt::Key_Minus + Qt::SHIFT), this, SLOT(on_zoomoutBtn_clicked()));
    new QShortcut(QKeySequence(Qt::Key_Plus), this, SLOT(on_zoominBtn_clicked()));



    timer->start();
    timer->setInterval(10);
    connect(timer,SIGNAL(timeout()),this,SLOT(PaintWatch()));

    ui->graphicsView->setScene(scene);
    ui->penSize->setValue(5);
    setColorStyle(0,0,0);

    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    allCheckFalse();
}

void imageitor::ShowContextMenu(const QPoint& pos) // this is a slot
{
    /*
     *
     * This menu appears when you right-click on a form.
     *
     */
    QPoint globalPos = this->mapToGlobal(pos);

    QMenu myMenu;
    myMenu.setStyleSheet("QMenu {color: #fff;background-color: #333;border: 1px solid black;} QMenu::item {background-color: transparent;} QMenu::item:selected {background-color: #555;}");
    myMenu.addAction("Undo (Ctrl + Z)", this, SLOT(Undo()));
    myMenu.addAction("Redo (Ctrl + Y)", this, SLOT(Redo()));
    myMenu.addSeparator();
    myMenu.addAction("Open (Ctrl + O)", this, SLOT(on_actionOpen_triggered()));
    myMenu.addAction("Save (Ctrl + S)", this, SLOT(on_actionSave_triggered()));
    myMenu.addAction("Save as (Ctrl + Shift + S)", this, SLOT(on_actionRGB_triggered()));
    myMenu.addSeparator();
    myMenu.addAction("RGB (Ctrl + C)", this, SLOT(on_actionRGB_triggered()));
    myMenu.addAction("Resize (Ctrl + R)", this, SLOT(on_actionResizing_triggered()));
    myMenu.addSeparator();
    myMenu.addAction("Option (Ctrl + Alt + O)", this, SLOT(on_actionOption_triggered()));
    myMenu.addAction("Infomation (F10)", this, SLOT(on_actionInfo_triggered()));

    QAction* selectedItem = myMenu.exec(globalPos);
    Q_UNUSED(selectedItem);
}

imageitor::~imageitor()
{
    /*
     *
     * Remember layout information when you exit.
     *
     */
    config.Width = this->geometry().width();
    config.Height = this->geometry().height();
    config.PosX = this->geometry().x();
    config.PosY = this->geometry().y();
    config.SAVE();
    delete ui;
}

/*
 *
 * <!-- Relating to Undu and Redu -->
 *
 */

void imageitor::PaintWatch()
{
    /*
     *
     * Monitors if the image has changed.
     *
     */
    if(scene->Drawing)
    {
        ImageBackup();
        scene->Drawing = !scene->Drawing;
    }
}

void imageitor::ImageBackup()
{
    /*
     *
     * The changed image is saved as like a queue.
     *
     */
    if(!Cycle)
    {
        if(runUndo)
        {
            ActivityCount = nowPoint + 2;
            runUndo = false;
        }
        ActivityPixmap[ActivityCount] = scanImage();
        ActivityCount++;

        if(ActivityCount >= (int)config.UndoSize)
        {
            ActivityCount = 0;
            StopPoint = (int)config.UndoSize - 1;
            Cycle = true;
        }
        else
        {
            StartPoint = 0;
            StopPoint = ActivityCount - 1;
        }
    }
    else
    {
        if(runUndo)
        {
            ActivityCount = nowPoint + 2;
            if(ActivityCount >= (int)config.UndoSize)
            {
                ActivityCount = 0;
                StartPoint = ActivityCount;
                StopPoint = (int)config.UndoSize - 1;
            }
            runUndo = false;
        }
        ActivityPixmap[ActivityCount] = scanImage();
        ActivityCount++;

        if(ActivityCount >= (int)config.UndoSize)
        {
            ActivityCount = 0;
            StartPoint = ActivityCount;
            StopPoint = (int)config.UndoSize - 1;
        }
        else
        {
            StartPoint = ActivityCount;
            StopPoint = ActivityCount - 1;
        }
    }

    //Created for testing.
    ui->l->setText(QString::number(ActivityCount));
    ui->l_2->setText(QString::number(StartPoint));
    ui->l_3->setText(QString::number(StopPoint));
    ui->l_4->setText(QString::number(nowPoint));
}

void imageitor::Undo()
{
    /*
     *
     * Import the old image from the backed up image.
     *
     */
    if(!runUndo)
    {
        nowPoint = StopPoint - 1;
        runUndo = true;
    }

    if(Cycle)
    {
        if(nowPoint < 0)
        {
            nowPoint = config.UndoSize - 1;
        }

        if(nowPoint == StartPoint)
        {
            QMessageBox::information(this, "Notify", "Cannot Undo.");
            return;
        }
    }
    else
    {
        if(nowPoint < StartPoint)
        {
            QMessageBox::information(this, "Notify", "Cannot Undo.");
            return;
        }
    }

    pixmap = ActivityPixmap[nowPoint];

    nowPoint--;

    scene = new paintScene(this);
    ui->graphicsView->setScene(scene);

    scene->setPenSize(ui->penSize->value());
    scene->setColor(penRed, penGreen, penBlue);

    item = new QGraphicsPixmapItem(pixmap);
    scene->addItem(item);

    FindPen();

    // Created for testing.
    ui->l->setText(QString::number(ActivityCount));
    ui->l_2->setText(QString::number(StartPoint));
    ui->l_3->setText(QString::number(StopPoint));
    ui->l_4->setText(QString::number(nowPoint));
}

void imageitor::Redo()
{

}

void imageitor::FindPen()
{
    /*
     *
     * Find the last pen that was used.
     * This feature requires modification. (Not Working)
     *
     */
    if(ui->dotBtn->isChecked())
    {
        allCheckFalse();
        ui->dotBtn->setChecked(true);
        scene->setDrawDot(true);
    }
    else if(ui->lineBtn->isChecked())
    {
        allCheckFalse();
        ui->lineBtn->setChecked(true);
        scene->setDrawLine(true);
    }
    else if(ui->squreBtn->isChecked())
    {
        allCheckFalse();
        ui->squreBtn->setChecked(true);
        scene->setDrawSqure(true);
    }
    else if(ui->roundBtn->isChecked())
    {
        allCheckFalse();
        ui->roundBtn->setChecked(true);
        scene->setDrawRound(true);
    }
}

/*
 *
 * <!-- End Relating to Undu and Redu -->
 *
 */

void imageitor::ThemeSelect(int arg1)
{
    /*
     *
     * Set the theme. We need to look for new ways to make modifications easier.
     *
     */
    switch (arg1) {
    case 1:
        this->setStyleSheet("Qimageitor { background:#444; } QMessageBox { background:#444; } QMessageBox QLabel { color: #fff; } QDialog { background: #333; } QDialog QLabel{ color:#fff; } QDialog QCheckBox{ color:#fff; } QSlider::handle:horizontal { background: #fff; border: 1px solid #5c5c5c; width: 18px; margin: -2px 0; border-radius: 3px; } QMenuBar { background:#444; color:#fff; } QMenuBar::item:selected { background: #555; } QMenuBar::item:pressed { background: #777; } QMenu { color: #fff; background-color: #333; border: 1px solid black; } QMenu::item { background-color: transparent; } QMenu::item:selected { background-color: #555; } QGraphicsView { background:#222; border:0px; } QSpinBox { background:#222; color:#fff; } QSpinBox::down-button { subcontrol-origin: border; } QPushButton { background:rgba(0,0,0,.0); color:#aaa; font-weight:bold; text-align:center; } QPushButton:hover { color:#ccc; } QPushButton:pressed { color:#fff; } QPushButton:checked { color:#fff; } QScrollBar:horizontal { border: 0px solid grey; background: #333; height: 15px; margin: 0px 22px 0 22px; } QScrollBar::handle:horizontal { background: #666; min-width: 20px; } QScrollBar::add-line:horizontal { border: 0px solid grey; background: #555; width: 20px; subcontrol-position: right; subcontrol-origin: margin; } QScrollBar::sub-line:horizontal { border: 0px solid grey; background: #555; width: 20px; subcontrol-position: left; subcontrol-origin: margin; } QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: none; } QScrollBar:vertical { border: 0px solid grey; background: #333; width: 15px; margin: 22px 0 22px 0; } QScrollBar::handle:vertical { background: #666; min-height: 20px; } QScrollBar::add-line:vertical { border: 0px solid grey; background: #555; height: 20px; subcontrol-position: bottom; subcontrol-origin: margin; } QScrollBar::sub-line:vertical { border: 0px solid grey; background: #555; height: 20px; subcontrol-position: top; subcontrol-origin: margin; } QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }");
        break;
    case 2:
        this->setStyleSheet("Qimageitor { background:#fff; } QMessageBox { background:#fff; } QMessageBox QLabel { color: #000; } QDialog { background: #fff; } QDialog QLabel{ color:#000; } QDialog QCheckBox{ color:#000; } QSlider::handle:horizontal { background: #000; border: 1px solid #c5c5c5; width: 18px; margin: -2px 0; border-radius: 3px; } QMenuBar { background:#fff; color:#000; } QMenuBar::item:selected { background: #eee; } QMenuBar::item:pressed { background: #ccc; } QMenu { color: #fff; background-color: #333; border: 1px solid black; } QMenu::item { background-color: transparent; } QMenu::item:selected { background-color: #555; } QGraphicsView { background:#eee; border:0px; } QSpinBox { background:#222; color:#fff; } QSpinBox::down-button { subcontrol-origin: border; } QPushButton { background:rgba(0,0,0,.0); color:#aaa; font-weight:bold; text-align:center; } QPushButton:hover { color:#666; } QPushButton:pressed { color:#000; } QPushButton:checked { color:#000; } QScrollBar:horizontal { border: 0px solid darkgrey; background: #ccc; height: 15px; margin: 0px 22px 0 22px; } QScrollBar::handle:horizontal { background: #fff; min-width: 20px; } QScrollBar::add-line:horizontal { border: 0px solid darkgrey; background: #aaa; width: 20px; subcontrol-position: right; subcontrol-origin: margin; } QScrollBar::sub-line:horizontal { border: 0px solid darkgrey; background: #aaa; width: 20px; subcontrol-position: left; subcontrol-origin: margin; } QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: none; } QScrollBar:vertical { border: 0px solid darkgrey; background: #ccc; width: 15px; margin: 22px 0 22px 0; } QScrollBar::handle:vertical { background: #fff; min-height: 20px; } QScrollBar::add-line:vertical { border: 0px solid darkgrey; background: #aaa; height: 20px; subcontrol-position: bottom; subcontrol-origin: margin; } QScrollBar::sub-line:vertical { border: 0px solid darkgrey; background: #aaa; height: 20px; subcontrol-position: top; subcontrol-origin: margin; } QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }");
        break;
    case 3:
        this->setStyleSheet("Qimageitor { background:#400071; } QMessageBox { background:#400071; } QMessageBox QLabel { color: #fff; } QDialog { background: #400071; } QDialog QLabel{ color:#fff; } QDialog QCheckBox{ color:#fff; } QSlider::handle:horizontal { background: #fff; border: 1px solid #5c5c5c; width: 18px; margin: -2px 0; border-radius: 3px; } QMenuBar { background:#400071; color:#fff; } QMenuBar::item:selected { background: #555; } QMenuBar::item:pressed { background: #777; } QMenu { color: #fff; background-color: #333; border: 1px solid black; } QMenu::item { background-color: transparent; } QMenu::item:selected { background-color: #555; } QGraphicsView { background:#300060; border:0px; } QSpinBox { background:#222; color:#fff; } QSpinBox::down-button { subcontrol-origin: border; } QPushButton { background:rgba(0,0,0,.0); color:#aaa; font-weight:bold; text-align:center; } QPushButton:hover { color:#ccc; } QPushButton:pressed { color:#fff; } QPushButton:checked { color:#fff; } QScrollBar:horizontal { border: 0px solid grey; background: #510082; height: 15px; margin: 0px 22px 0 22px; } QScrollBar::handle:horizontal { background: #7300a4; min-width: 20px; } QScrollBar::add-line:horizontal { border: 0px solid grey; background: #510082; width: 20px; subcontrol-position: right; subcontrol-origin: margin; } QScrollBar::sub-line:horizontal { border: 0px solid grey; background: #510082; width: 20px; subcontrol-position: left; subcontrol-origin: margin; } QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: none; } QScrollBar:vertical { border: 0px solid grey; background: #510082; width: 15px; margin: 22px 0 22px 0; } QScrollBar::handle:vertical { background: #7300a4; min-height: 20px; } QScrollBar::add-line:vertical { border: 0px solid grey; background: #510082; height: 20px; subcontrol-position: bottom; subcontrol-origin: margin; } QScrollBar::sub-line:vertical { border: 0px solid grey; background: #510082; height: 20px; subcontrol-position: top; subcontrol-origin: margin; } QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }");
        break;
    }
    config.Theme = arg1;
}

void imageitor::allCheckFalse()
{
    ui->dotBtn->setChecked(false);
    ui->lineBtn->setChecked(false);
    ui->squreBtn->setChecked(false);
    ui->roundBtn->setChecked(false);

    scene->setDrawDot(false);
    scene->setDrawLine(false);
    scene->setDrawSqure(false);
    scene->setDrawRound(false);
}

QPixmap imageitor::scanImage()
{
    /*
     *
     * The layout was removed and added to store the displayed image in its best condition.
     *
     */
    int widthTemp, heightTemp;
    widthTemp = ui->graphicsView->geometry().width();
    heightTemp = ui->graphicsView->geometry().height();

    ScrollPos[0] = ui->graphicsView->horizontalScrollBar()->value();
    ScrollPos[1] = ui->graphicsView->verticalScrollBar()->value();

    ui->graphicsView->setGeometry(ui->graphicsView->geometry().x(),ui->graphicsView->geometry().y(),scene->width(),scene->height());
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->resetMatrix();
    ui->graphicsView->scroll(0,0);

    QPixmap temp = ui->graphicsView->grab();

    ui->graphicsView->setGeometry(ui->graphicsView->geometry().x(),ui->graphicsView->geometry().y(),widthTemp,heightTemp);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    ui->graphicsView->horizontalScrollBar()->setValue(ScrollPos[0]);
    ui->graphicsView->verticalScrollBar()->setValue(ScrollPos[1]);

    if(scaleCount > 0)
    {
        for(int i=0;i<scaleCount;i++)
            ui->graphicsView->scale(1.25,1.25);
    }
    else if(scaleCount == 0)
    {
        ui->graphicsView->resetMatrix();
    }
    else
    {
        scaleCount *= -1;
        for(int i=0;i<scaleCount;i++)
            ui->graphicsView->scale(0.8,0.8);
        scaleCount *= -1;
    }
    return temp;
}

/*
 *
 * <!-- Relating to save and load -->
 *
 */

void imageitor::on_actionSave_triggered()
{
    if(OpenImage) {
        if(config.SaveAsk) {
            int Answer = QMessageBox::question(this,
                                               "Really?",
                                               "Existing images will be erased and overwritten!",
                                               QMessageBox::Yes | QMessageBox::No);
            switch (Answer) {
            case QMessageBox::Yes :
                break;
            case QMessageBox::No :
                return;
            }
        }
        QPixmap savePixmap = scanImage();
        savePixmap.save(fileLink,nullptr,config.SaveCompress);
        QMessageBox::information(this, "Notify", "Successfully saved.");
    } else {
        QMessageBox::information(this, "Please", "The image must be opened first.");
    }
}

void imageitor::on_actionSave_as_triggered()
{
    if(OpenImage) {
        QPixmap savePixmap = scanImage();
        savePixmap.save(QFileDialog::getSaveFileName(this,"SAVE FILE","","PNG(*.png) ;; JPEG (*.jpg)"),nullptr,config.SaveCompress);
    } else {
        QMessageBox::information(this,"Notice","Please the image must be opened first.");
    }
}

void imageitor::on_actionOpen_triggered()
{
    if(OpenImage && scene->runEdit)
    {
        int Answer = QMessageBox::question(this,
                                           "Really?",
                                           "Are you sure you want to erase this image and open a new file?",
                                           QMessageBox::Yes | QMessageBox::No);
        switch (Answer) {
        case QMessageBox::Yes :
            break;
        case QMessageBox::No :
            return;
        }
    }

    fileLink = QFileDialog::getOpenFileName(this,"",config.lastWorkDirectory);

    if(fileLink!="") {
        OpenImage = true;

        config.lastWorkDirectory = fileLink;
        config.SAVE();

        pixmap.load(fileLink);

        scene = new paintScene(this);
        ui->graphicsView->setScene(scene);

        scene->setPenSize(ui->penSize->value());
        scene->setColor(penRed, penGreen, penBlue);
        allCheckFalse();

        imgRed = 1;
        imgGreen = 1;
        imgBlue = 1;

        item = new QGraphicsPixmapItem(pixmap);
        scene->addItem(item);

        scene->runEdit = false;

        ImageBackup();
    }
}

/*
 *
 * <!-- End Relating to save and load -->
 *
 */

/*
 *
 * <!-- Relating to zoom -->
 *
 */

void imageitor::on_zoomoutBtn_clicked(){
    ui->graphicsView->scale(0.8,0.8);
    scaleCount--;
}

void imageitor::on_zoominBtn_clicked(){
    ui->graphicsView->scale(1.25,1.25);
    scaleCount++;
}

/*
 *
 * <!-- End Relating to zoom -->
 *
 */

/*
 *
 * <!-- Related to Image RGB control -->
 *
 */

void imageitor::on_actionRGB_triggered()
{
    if(OpenImage) {
        rgb_changer rgb(*this, 0, imgRed, imgGreen, imgBlue, this);
        rgb.exec();
    } else {
        QMessageBox::information(this,"Notice","Please the image must be opened first.");
    }
}

void imageitor::Image_RGB_Change(int slider_r, int slider_g, int slider_b)
{
    scene->runEdit = true;
    imgRed = slider_r; imgGreen = slider_g; imgBlue = slider_b;
    QImage image = pixmap.toImage();
    int r,g,b;
    QRgb rgb;

    for(int y = 0; y < image.height(); y++)
    {
        for(int x = 0 ; x < image.width(); x++)
        {
            rgb = image.pixel(x,y);
            r = qRed(rgb) + slider_r;
            g = qGreen(rgb) + slider_g;
            b = qBlue(rgb) + slider_b;
            if(r < 0) r = 0;
            else if(r > 255) r = 255;
            if(g < 0) g = 0;
            else if(g > 255) g = 255;
            if(b < 0) b = 0;
            else if(b > 255) b = 255;
            image.setPixel(x,y,qRgb(r,g,b));
        }
    }
    preview = QPixmap::fromImage(image);
    item->setPixmap(preview);

    ImageBackup();
}

void imageitor::Image_RGB_Preview_Change(int slider_r, int slider_g, int slider_b)
{
    int pre = 1;
    if(config.PreviewSize < pixmap.size().height()) pre = pixmap.size().height()/config.PreviewSize;
    QImage image = pixmap.scaled(pixmap.size().width()/pre,pixmap.size().height()/pre).toImage();
    int r,g,b;
    QRgb rgb;
    for(int y = 0; y < image.height(); y++)
    {
        for(int x = 0 ; x < image.width(); x++)
        {
            rgb = image.pixel(x,y);
            r = qRed(rgb) + slider_r;
            g = qGreen(rgb) + slider_g;
            b = qBlue(rgb) + slider_b;
            if(r < 0) r = 0;
            else if(r > 255) r = 255;
            if(g < 0) g = 0;
            else if(g > 255) g = 255;
            if(b < 0) b = 0;
            else if(b > 255) b = 255;
            image.setPixel(x,y,qRgb(r,g,b));
        }
    }
    preview = QPixmap::fromImage(image.scaled(image.size().width()*pre,image.size().height()*pre));
    item->setPixmap(preview);
}

/*
 *
 * <!-- End Related to Image RGB control -->
 *
 */

/*
 *
 * <!-- Related to Image HIS control -->
 *
 */

void imageitor::on_actionHIS_triggered()
{
    if(OpenImage) {
        rgb_changer his(*this, 2, imgRed, imgGreen, imgBlue, this);
        his.exec();
    } else {
        QMessageBox::information(this,"Notice","Please the image must be opened first.");
    }
}

void imageitor::Image_Hue_Change(int slider)
{
    scene->runEdit = true;
    QImage image = pixmap.toImage();
    double RGB[3],hue;
    QRgb rgb;

    for(int y = 0; y < image.height(); y++)
    {
        for(int x = 0 ; x < image.width(); x++)
        {
            rgb = image.pixel(x,y);
            RGB[0] = qRed(rgb) / 255;
            RGB[1] = qGreen(rgb) / 255;
            RGB[2] = qBlue(rgb) / 255;

            double min = 999, max = 0;
            int max_index = 0;
            for(int i=0;i<3;i++) {
                if(min > RGB[i]) min = RGB[i];
                if(max < RGB[i]) { max = RGB[i]; max_index = i; }
            }

            switch(max_index) {
            case 0: // Red is max
                hue = (RGB[1]-RGB[2])/(max-min);
                break;
            case 1: // Green is max
                hue = 2.0 + (RGB[2]-RGB[1])/(max-min);
                break;
            case 2: // Blue is max
                hue = 4.0 + (RGB[0]-RGB[1])/(max-min);
                break;
            }

            // image.setPixel(x,y,qRgb(r,g,b));
        }
    }
    preview = QPixmap::fromImage(image);
    item->setPixmap(preview);

    ImageBackup();
}

void imageitor::Image_Intensity_Change(int slider)
{
    scene->runEdit = true;
    QImage image = pixmap.toImage();
    double r,g,b;
    double intensity;
    QRgb rgb;

    for(int y = 0; y < image.height(); y++)
    {
        for(int x = 0 ; x < image.width(); x++)
        {
            rgb = image.pixel(x,y);
            r = qRed(rgb);
            g = qGreen(rgb);
            b = qBlue(rgb);
            intensity = (r+g+b)/3;
            image.setPixel(x,y,qRgb(r,g,b));
        }
    }
    preview = QPixmap::fromImage(image);
    item->setPixmap(preview);

    ImageBackup();
}

void imageitor::Image_Saturation_Change(int slider)
{
    scene->runEdit = true;
    QImage image = pixmap.toImage();
    double RGB[3];
    double saturation;
    QRgb rgb;

    for(int y = 0; y < image.height(); y++)
    {
        for(int x = 0 ; x < image.width(); x++)
        {
            rgb = image.pixel(x,y);
            RGB[0] = qRed(rgb);
            RGB[1] = qGreen(rgb);
            RGB[2] = qBlue(rgb);
            double min = 999;
            for(int i=0;i<3;i++) {
                if(min > RGB[i]) min = RGB[i];
            }
            saturation = 1 - 3/(RGB[0]+RGB[1]+RGB[2]) * min;
        }
    }
    preview = QPixmap::fromImage(image);
    item->setPixmap(preview);

    ImageBackup();
}

/*
 *
 * <!-- End Related to Image HIS control -->
 *
 */

/*
 *
 * <!-- Related to pen settings -->
 *
 */

void imageitor::on_dotBtn_clicked()
{
    if(OpenImage) {
        allCheckFalse();
        ui->dotBtn->setChecked(true);
        scene->setDrawDot(true);
    } else {
        QMessageBox::information(this,"Notice","Please the image must be opened first.");
    }
}

void imageitor::on_lineBtn_clicked()
{
    if(OpenImage) {
        allCheckFalse();
        ui->lineBtn->setChecked(true);
        scene->setDrawLine(true);
    } else {
        QMessageBox::information(this,"Notice","Please the image must be opened first.");
    }
}

void imageitor::on_squreBtn_clicked()
{
    if(OpenImage) {
        allCheckFalse();
        ui->squreBtn->setChecked(true);
        scene->setDrawSqure(true);
    } else {
        QMessageBox::information(this,"Notice","Please the image must be opened first.");
    }
}

void imageitor::on_roundBtn_clicked()
{
    if(OpenImage) {
        allCheckFalse();
        ui->roundBtn->setChecked(true);
        scene->setDrawRound(true);
    } else {
        QMessageBox::information(this,"Notice","Please the image must be opened first.");
    }
}

void imageitor::setColorStyle(int slider_r, int slider_g, int slider_b)
{
    penRed = slider_r; penGreen = slider_g; penBlue = slider_b;
    ui->penColor->setStyleSheet("background:rgb("+QString::number(slider_r)+","+QString::number(slider_g)+","+QString::number(slider_b)+")");
    scene->setColor(slider_r,slider_g,slider_b);
}

void imageitor::on_penSize_valueChanged(int arg1)
{
    scene->setPenSize(arg1);
}

void imageitor::on_penColor_clicked()
{
    rgb_changer rgb(*this, 1, penRed, penGreen, penBlue, this);
    rgb.exec();
}

/*
 *
 * <!-- End Related to pen settings -->
 *
 */

/*
 *
 * <!-- Related to image resize -->
 *
 */

void imageitor::on_actionResizing_triggered()
{
    if(OpenImage) {
        image_resizer resizer(*this, pixmap.width(), pixmap.height(), this);
        resizer.exec();
    } else {
        QMessageBox::information(this,"Notice","Please the image must be opened first.");
    }
}

void imageitor::Image_Size_Change(int w, int h)
{
    pixmap = scanImage();
    pixmap = pixmap.scaled(w,h);

    scene = new paintScene(this);
    ui->graphicsView->setScene(scene);

    scene->setPenSize(ui->penSize->value());
    scene->setColor(penRed, penGreen, penBlue);
    FindPen();

    item = new QGraphicsPixmapItem(pixmap);
    scene->addItem(item);

    ImageBackup();
}

/*
 *
 * <!-- End Related to image resize -->
 *
 */

/*
 *
 * <!-- Relevant to image crop -->
 *
 */

void imageitor::on_cropBtn_clicked()
{
    if(OpenImage) {
        allCheckFalse();
        if(!Crop) {
            Crop = true;
            ui->maskLabel->setStyleSheet("background:rgba(255,255,255,0.5)");
            ui->maskLabel->setGeometry(ui->graphicsView->geometry().x(),ui->graphicsView->geometry().y(),this->geometry().width()-ui->graphicsView->geometry().x(),this->geometry().height()-ui->graphicsView->geometry().y()-20);
        }
        else {
            Crop = false;
            ui->maskLabel->setStyleSheet("background:rgba(255,255,255,0)");
            ui->cropLabel->setStyleSheet("border: 0px solid red;");
            ui->maskLabel->setGeometry(0,0,0,0);
            ui->cropLabel->setGeometry(0,0,0,0);
        }
        ui->cropBtn->setChecked(Crop);
    } else {
        QMessageBox::information(this,"Notice","Please the image must be opened first.");
    }
}

void imageitor::mousePressEvent(QMouseEvent *event) {
    if(Crop) {
        previousPoint = event->localPos();
        previousPoint.setY(previousPoint.y()-ui->menuBar->height());

        ui->cropLabel->setStyleSheet("border: 3px solid red;");
        ui->cropLabel->setGeometry(previousPoint.x(),previousPoint.y(),event->localPos().x()-previousPoint.x(),event->localPos().y()-previousPoint.y());
    }
}

void imageitor::mouseMoveEvent(QMouseEvent *event) {
    if(Crop) {
        lastPoint = event->localPos()-previousPoint;
        lastPoint.setY(lastPoint.y()-ui->menuBar->height());

        ui->cropLabel->setGeometry(previousPoint.x(), previousPoint.y(), lastPoint.x(), lastPoint.y());
    }
}

void imageitor::mouseReleaseEvent(QMouseEvent *event) {
    if(Crop) {
        ui->maskLabel->setStyleSheet("background:rgba(255,255,255,0)");
        ui->cropLabel->setStyleSheet("border: 0px solid red;");
        ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        pixmap = ui->graphicsView->grab(QRect(previousPoint.x()-ui->graphicsView->x(),previousPoint.y()-ui->graphicsView->y(),event->localPos().x()-previousPoint.x(),event->localPos().y()-ui->menuBar->height()-previousPoint.y()));

        ui->graphicsView->resetMatrix();
        scaleCount = 0;
        ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        ui->maskLabel->setGeometry(0,0,0,0);
        ui->cropLabel->setGeometry(0,0,0,0);

        scene = new paintScene(this);
        ui->graphicsView->setScene(scene);

        scene->setPenSize(ui->penSize->value());
        scene->setColor(penRed, penGreen, penBlue);

        item = new QGraphicsPixmapItem(pixmap);
        scene->addItem(item);

        ui->cropBtn->setChecked(false);
        Crop = false;

        ImageBackup();
    }
}

/*
 *
 * <!-- End Relevant to image crop -->
 *
 */

/*
 *
 * <!-- Relevant to reactive layouts -->
 *
 */

void imageitor::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    ui->graphicsView->setGeometry(ui->graphicsView->geometry().x(),ui->graphicsView->geometry().y(),this->geometry().width()-ui->graphicsView->geometry().x(),this->geometry().height()-ui->graphicsView->geometry().y()-20);

    if(Crop)
    {
        ui->maskLabel->setGeometry(ui->graphicsView->geometry().x(),ui->graphicsView->geometry().y(),this->geometry().width()-ui->graphicsView->geometry().x(),this->geometry().height()-ui->graphicsView->geometry().y()-20);
    }

    ui->zoominBtn->move(this->geometry().width()-60,0);
    ui->zoomoutBtn->move(this->geometry().width()-30,0);
    ui->cropBtn->move(0,this->geometry().height()-50);

    ui->separator->setGeometry(0,0,this->geometry().width(),1);
}

/*
 *
 * <!-- End Relevant to reactive layouts -->
 *
 */

/*
 *
 * <!-- Relating to other forms -->
 *
 */

void imageitor::on_actionOption_triggered()
{
    Option op(*this, this);
    op.exec();
}
