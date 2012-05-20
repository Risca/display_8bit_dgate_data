#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define IMG_COLORS 256

#include "dg_image.h"

#include <QMainWindow>
#include <QImage>
#include <QVector>

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
    void openFile();
    void printError(QString);
    void setImage(int);
    
private:
    Ui::MainWindow   * ui;
    uchar            * _fileData;
    QVector<DG_image*> _images;

    void makeCheckImage(void);
    void deleteImages();
};

#endif // MAINWINDOW_H
