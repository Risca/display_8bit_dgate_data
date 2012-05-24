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
    _fileData(NULL),
    _file(NULL)
{
    ui->setupUi(this);

    connect(ui->actionAboutQt,SIGNAL(triggered()),qApp,SLOT(aboutQt()));
    connect(ui->actionOpen,SIGNAL(triggered()),this,SLOT(openFile()));
    connect(ui->imageCountSpinBox,SIGNAL(valueChanged(int)),this,SLOT(setImage(int)));
    connect(ui->offsetSpinBox,SIGNAL(valueChanged(int)),this,SLOT(changeOffset(int)));
}

MainWindow::~MainWindow()
{
    deleteImages();
    // segfaults (invalid pointer)
    //delete _fileData;
    delete _file;
    delete ui;
}

void MainWindow::openFile()
{
    quint32 offset;
    quint16 tmp;
    quint16 width, height;
    DG_image * image;
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open file"));
    if (_file != NULL) delete _file;
    _file=new QFile(fileName);
    if (!_file->open(QIODevice::ReadOnly)) {
        printError("Failed to open file");
        return;
    }
    setWindowTitle(fileName);
    _fileData = _file->map(0,_file->size());
    deleteImages();
    QDataStream in(_file);
    in.setByteOrder(QDataStream::LittleEndian);
    in >> offset;
    while (offset != 0 && !in.atEnd()) {
        in >> tmp; // type
        in >> width >> height;
        switch (tmp) {
        case DG_SpellAnimation:
        case DG_Animation:
        case DG_BigAnimation:
            // 4 byte header
            image = new DG_image(_fileData,offset,4,width,height,tmp,false);
//            image->setColorTable(_images.back()->colorTable());
            break;

        case DG_BookAnimation:
            image = new DG_image(_fileData,offset,3*256+4,width,height,tmp);
            break;
        case DG_NoteOrOpenBook:
        case DG_BigImage:
            image = new DG_image(_fileData,offset,3*256,width,height,tmp);
            break;
        case DG_ClosedBook:
        case DG_NavigationAndItems:
        default:
            image = new DG_image(_fileData,offset,0,width,height,tmp,false);
            std::cerr << "Not known image type: 0x" << std::hex << tmp << std::endl << std::dec;
        }
        _images.push_back(image);
        std::cout << "Added image (" << width << "x" << height << ") at offset " << offset << std::endl;
        // Align to next offset
        in >> tmp;
        in >> offset;
    }
    if (!_images.isEmpty()) {
        ui->imageCountSpinBox->setEnabled(true);
        ui->imageCountSpinBox->setMaximum(_images.size());
        setImage(1);
    } else {
        ui->imageLabel->clear();
        ui->imageCountSpinBox->setEnabled(false);
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

    DG_image * image=_images.at(n-1);
    ui->imageLabel->setPixmap(QPixmap::fromImage(*image));
    switch (image->type()) {
    case DG_BigImage:
    case DG_SpellAnimation:
    case DG_Animation:
    case DG_BigAnimation:
    case DG_NoteOrOpenBook:
    case DG_BookAnimation:
    case DG_NavigationAndItems:
    case DG_ClosedBook:
        ui->offsetSpinBox->setEnabled(false);
        break;
    default:
        ui->offsetSpinBox->setEnabled(true);
        ui->offsetSpinBox->setValue(image->imageOffset());
    }
    ui->imageHeaderLabel->setText("image #"+QString::number(ui->imageCountSpinBox->value())+
                                  " (0x"+QString::number(image->type(),16)+
                                  ", 0x"+QString::number(image->fileOffset(),16)+
                                  ", "+QString::number(image->width())+
                                  "x"+QString::number(image->height())+
                                  "==0x"+QString::number(image->height()*image->width(),16)+
                                  ", 0x"+QString::number(image->fileOffset()+image->imageOffset()+image->height()*image->width(),16)+
                                  ")");
}

void MainWindow::changeOffset(int offset)
{
    int idx=ui->imageCountSpinBox->value()-1;
    DG_image * image, * old=_images.at(idx);
    image=new DG_image(_fileData,old->fileOffset(),offset,
                       old->width(),old->height(),old->type(),false);
    _images.replace(idx,image);
    delete old;
    setImage(idx+1);
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
