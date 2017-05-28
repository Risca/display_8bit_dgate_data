#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define IMG_COLORS 256

#include "dg_image.h"

#include <QMainWindow>
#include <QImage>
#include <QVector>
#include <QFile>
#include <QLabel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void fileOpened(const QString &filename);

private slots:
    void openFile();
    void processFile(const QString &filename);
    void printError(QString);
    void setImage(int);
    void changeOffset(int);
    void changePalette();
    void scaleImage(int);
    
private:
    Ui::MainWindow   * ui;
    uchar            * _fileData;
    QVector<DG_image*> _images;
    QFile            * _file;
    QLabel           * _imageLabel;
    int                _scaleFactor;

    void makeCheckImage(void);
    void deleteImages();
    bool eventFilter(QObject *obj, QEvent *ev);
};

#endif // MAINWINDOW_H
