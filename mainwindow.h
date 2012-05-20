#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define IMG_COLORS 256

#include "dg_image.h"

#include <QMainWindow>
#include <QImage>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void updateImage();
    void openFile();
    void printError(QString);
    
private:
    Ui::MainWindow * ui;
    DG_image       * _image;
    uchar          * _imageData;
    QByteArray       _fileData;
    QVector<QRgb>    _colors;

    void makeCheckImage(void);
};

#endif // MAINWINDOW_H
