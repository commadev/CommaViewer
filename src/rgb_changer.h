#ifndef RGB_CHANGER_H
#define RGB_CHANGER_H

#include "imageitor.h"
#include <QDialog>

namespace Ui {
class rgb_changer;
}

class rgb_changer : public QDialog
{
    Q_OBJECT

public:
    explicit rgb_changer(imageitor &ref, int Mode, int r, int g, int b, QWidget *parent = nullptr);
    ~rgb_changer();

private slots:
    void on_redSlider_valueChanged(int value);
    void on_greenSlider_valueChanged(int value);
    void on_blueSlider_valueChanged(int value);
    void on_redSlider_sliderReleased();
    void on_greenSlider_sliderReleased();
    void on_blueSlider_sliderReleased();

private:
    Ui::rgb_changer *ui;
    imageitor &mRef;
    int ModeSelect;
    int rColor;
    int gColor;
    int bColor;
};

#endif // RGB_CHANGER_H
