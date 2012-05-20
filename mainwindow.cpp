#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "vga_color_palette.h"

#include <QVector>
#include <QColor>
#include <QPixmap>
#include <QDir>
#include <QFileDialog>
#include <QFile>
#include <QDataStream>
#include <QGraphicsPixmapItem>
#include <iostream>
#include <ios>
#include <new>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _fileData(NULL)
{
    ui->setupUi(this);

    connect(ui->actionAboutQt,SIGNAL(triggered()),qApp,SLOT(aboutQt()));
    connect(ui->actionOpen,SIGNAL(triggered()),this,SLOT(openFile()));
    connect(ui->imageSpinBox,SIGNAL(valueChanged(int)),this,SLOT(setImage(int)));
}

MainWindow::~MainWindow()
{
    deleteImages();
    // segfaults (invalid pointer)
    //delete _fileData;
    delete ui;
}

void MainWindow::openFile()
{
    quint32 offset;
    quint16 tmp;
    quint16 width, height;
    DG_image * image;
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open file"));
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        printError("Failed to open file");
        return;
    }
    setWindowTitle(fileName);
    _fileData = file.map(0,file.size());
    deleteImages();
    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);
    in >> offset;
    while (offset != 0 && !in.atEnd()) {
        in >> tmp; // type
        if (tmp==0x1500) { // Image
            in >> width >> height;
            image = new DG_image(_fileData+offset,width,height);
            _images.push_back(image);
            std::cout << "Added image (" << width << "x" << height << ") at offset " << offset << std::endl;
        } else {
            std::cerr << "Not image type: " << std::hex << tmp << std::endl << std::dec;
            // Read past unknown data
            in >> tmp >> tmp;
        }
        // Align to next offset
        in >> tmp;
        in >> offset;
    }
    if (!_images.isEmpty()) {
        ui->imageLabel->setPixmap(QPixmap::fromImage(*_images.first()));
        ui->imageSpinBox->setEnabled(true);
        ui->imageSpinBox->setMaximum(_images.size());
    } else {
        ui->imageLabel->clear();
        ui->imageSpinBox->setEnabled(false);
    }
    ui->statusBar->showMessage("Read "+QString::number(_images.size())+" images");
    std::cout << "Read " << _images.size() << " images" << std::endl;
}

void MainWindow::printError(QString str)
{
    std::cerr << str.toStdString() << std::endl;
    ui->statusBar->showMessage(str,1000);
}

void MainWindow::setImage(int n)
{
    if (n>_images.size())
        return;
    ui->imageLabel->setPixmap(QPixmap::fromImage(*_images.at(n-1)));
}

void MainWindow::makeCheckImage(void)
{
    unsigned int i, j, c;
    unsigned int checkImageWidth=256;
    unsigned int checkImageHeight=256;

    for (i = 0; i < checkImageHeight; i++) {
        for (j = 0; j < checkImageWidth; j++) {
            //0 or 255 --> black or white
            c = ((((i&0x10)==0))^(((j&0x10))==0))*(j);
            _fileData[i*checkImageWidth+j] = (uchar) c;
        }
    }
}

void MainWindow::deleteImages()
{
    DG_image * image;
    int size=_images.size();
    for (int i=0; i<size; i++) {
        image=_images.back();
        _images.pop_back();
        delete image;
    }
}
