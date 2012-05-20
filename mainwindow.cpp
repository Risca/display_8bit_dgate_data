#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "vga_color_palette.h"

#include <QVector>
#include <QColor>
#include <QPixmap>
#include <QDir>
#include <QFileDialog>
#include <QFile>
#include <iostream>
#include <new>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _image(NULL),
    _imageData(NULL)
{
    ui->setupUi(this);

    updateImage();

    connect(ui->widthSpinBox,SIGNAL(valueChanged(int)),this,SLOT(updateImage()));
    connect(ui->heightSpinBox,SIGNAL(valueChanged(int)),this,SLOT(updateImage()));
    connect(ui->offsetSpinBox,SIGNAL(valueChanged(int)),this,SLOT(updateImage()));
    connect(ui->offsetSpinBox,SIGNAL(valueChanged(int)),ui->lcdNumber,SLOT(display(int)));

    connect(ui->updateButton,SIGNAL(clicked()),this,SLOT(updateImage()));
    connect(ui->actionAboutQt,SIGNAL(triggered()),qApp,SLOT(aboutQt()));
    connect(ui->actionOpen,SIGNAL(triggered()),this,SLOT(openFile()));
}

MainWindow::~MainWindow()
{
    delete _image;
    delete _imageData;
    delete ui;
}

void MainWindow::updateImage()
{
    unsigned int width=ui->widthSpinBox->value();
    unsigned int height=ui->heightSpinBox->value();
    unsigned int offset=ui->offsetSpinBox->value();

    delete _image; _image=NULL;
    delete []_imageData; _imageData=NULL;
    _imageData=new (std::nothrow) uchar[width*height];
    if (_imageData==NULL) {
        printError("Image too big to fit in memory");
        return;
    }

    if (_fileData.isEmpty()) {
        makeCheckImage();
    } else if (width*height+offset+1 >= _fileData.size()) {
        printError("Read past end of file");
        height=_fileData.size()/width - offset/width;
        // setValue() will emit valueChanged()
        // and this function gets called again
        ui->heightSpinBox->setValue(height);
        return;
    } else {
        for (unsigned int i=0;i < width*height;i++) {
            _imageData[i]=_fileData.at(i+offset);
        }
    }
    _image = new DG_image(_imageData,width,height);
    ui->imageLabel->setPixmap(QPixmap::fromImage(*_image));
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open file"));
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        printError("Failed to open file");
        return;
    }

    _fileData.clear();
    _fileData.append(file.readAll());
    updateImage();
}

void MainWindow::printError(QString str)
{
    std::cerr << str.toStdString() << std::endl;
    ui->statusBar->showMessage(str,1000);
}

void MainWindow::makeCheckImage(void)
{
    unsigned int i, j, c;
    unsigned int checkImageWidth=ui->widthSpinBox->value();
    unsigned int checkImageHeight=ui->heightSpinBox->value();

    for (i = 0; i < checkImageHeight; i++) {
        for (j = 0; j < checkImageWidth; j++) {
            //0 or 255 --> black or white
            c = ((((i&0x10)==0))^(((j&0x10))==0))*(j);
            _imageData[i*checkImageWidth+j] = (uchar) c;
        }
    }
}
