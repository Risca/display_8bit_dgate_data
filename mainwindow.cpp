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
#include <QMouseEvent>
#include <QTimer>
#include <iostream>
#include <ios>
#include <new>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _fileData(NULL),
    _file(NULL),
    _imageLabel(new QLabel),
    _scaleFactor(1)
{
    ui->setupUi(this);

    _imageLabel->setBackgroundRole(QPalette::Base);
    _imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    _imageLabel->setScaledContents(true);
    _imageLabel->setMouseTracking(true);

    ui->scrollArea->setBackgroundRole(QPalette::Dark);
    ui->scrollArea->setWidget(_imageLabel);

    connect(ui->actionAboutQt,SIGNAL(triggered()),qApp,SLOT(aboutQt()));
    connect(ui->actionOpen,SIGNAL(triggered()),this,SLOT(openFile()));
    connect(this, SIGNAL(fileOpened(QString)), this, SLOT(processFile(QString)));
    connect(ui->imageCountSlider,SIGNAL(valueChanged(int)),this,SLOT(setImage(int)));
    connect(ui->offsetSpinBox,SIGNAL(valueChanged(int)),this,SLOT(changeOffset(int)));
    connect(ui->paletteCheckBox,SIGNAL(stateChanged(int)),this,SLOT(changePalette()));
    connect(ui->zoomSlider,SIGNAL(valueChanged(int)),this,SLOT(scaleImage(int)));
    _imageLabel->installEventFilter(this);
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
    QString filename = QFileDialog::getOpenFileName(this,tr("Open file"));
    if (!filename.isEmpty()) {
        emit fileOpened(filename);
    }
}

void MainWindow::processFile(const QString &filename)
{
    quint32 offset;
    quint16 tmp;
    quint16 width, height;
    DG_image * image;

    if (_file != NULL) delete _file;
    _file=new QFile(filename);
    if (!_file->open(QIODevice::ReadOnly)) {
        printError("Failed to open file");
        return;
    }
    setWindowTitle(filename);
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
        {
            int i=_images.size();
            DG_image * tmpImage;
            do {
                i--;
                tmpImage=_images.at(i);
            } while (!tmpImage->hasPalette() && i!=0);

            if (tmpImage->hasPalette()) {
                image->setColorTable(tmpImage->colorTable());
            }
        }
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
        ui->zoomSlider->setEnabled(true);
        ui->imageCountSlider->setEnabled(true);
        ui->imageCountSlider->setMaximum(_images.size());
        setImage(1);
    } else {
        _imageLabel->clear();
        ui->zoomSlider->setEnabled(false);
        ui->imageCountSlider->setEnabled(false);
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
    _imageLabel->setPixmap(QPixmap::fromImage(*image));
    scaleImage(_scaleFactor);
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
    ui->imageHeaderLabel->setText("image #"+QString::number(ui->imageCountSlider->value())+
                                  " (type: 0x"+QString::number(image->type(),16)+
                                  ", file offset: 0x"+QString::number(image->fileOffset(),16)+
                                  ", "+QString::number(image->width())+
                                  "x"+QString::number(image->height())+
                                  "==0x"+QString::number(image->height()*image->width(),16)+
                                  ", image header size: 0x"+QString::number(image->imageOffset(),16)+
                                  ", coordinates: ("+QString::number(image->coordinates().x())+", "+
                                                     QString::number(image->coordinates().y())+
                                  ")");
}

void MainWindow::changeOffset(int offset)
{
    int idx=ui->imageCountSlider->value()-1;
    DG_image * image, * old=_images.at(idx);
    image=new DG_image(_fileData,old->fileOffset(),offset,
                       old->width(),old->height(),old->type(),false);
    _images.replace(idx,image);
    delete old;
    setImage(idx+1);
}

void MainWindow::changePalette()
{
    foreach(DG_image * img,_images) {
        if (!img->hasPalette()) {
            img->switchColorTable();
        }
    }
    setImage(ui->imageCountSlider->value());
}

void MainWindow::scaleImage(int factor)
{
    _scaleFactor=factor;
    _imageLabel->resize(factor * _imageLabel->pixmap()->size());
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

bool MainWindow::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj==_imageLabel && ev->type()==QEvent::MouseMove) {
        if (_images.isEmpty())
            return false;
        DG_image *image=_images.at(ui->imageCountSlider->value()-1);
        // Get geometry of current image
        int width=image->width();
        int height=image->height();
        // Get coordinates of mouse (inside imageLabel).
        QMouseEvent * mEv=static_cast<QMouseEvent*>(ev);
        int x = mEv->pos().x() / _scaleFactor;
        int y = mEv->pos().y()/_scaleFactor;
        // Check bounds
        if (x > width)
            x=width;
        if (x < 0)
            x=0;
        if (y > height)
            y=height;
        if (y < 0)
            y=0;
        unsigned char pixelIndex=image->pixelIndex(x,y);
        ui->statusBar->showMessage("("+QString::number(x)+","+QString::number(y)+") 0x"+QString::number(pixelIndex, 16) + "->"+QString::number(image->pixel(x,y), 16));
        return true;
    }
    return false;
}
