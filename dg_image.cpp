#include "dg_image.h"

DG_image::DG_image(const uchar *data, quint32 fileOffset, quint32 imageOffset, int width, int height, quint16 type, bool palette) :
    QImage(data+fileOffset+imageOffset,width,height,width,QImage::Format_Indexed8),
    _type(type),
    _fileOffset(fileOffset),
    _imageOffset(imageOffset),
    _palette(palette)
{
    QVector<QRgb> colors;
    for (int i=0;i<256;i++) {
        QRgb color = 0xff000000 | (i<<16) | (i<<8) | i;
        colors.push_back(color);
    }
    _altColorTable=colors;
    if ((imageOffset == 4) || (imageOffset == (4 + 3*256))) {
        quint16 x = *reinterpret_cast<const quint16*>(data + fileOffset);
        quint16 y = *reinterpret_cast<const quint16*>(data + fileOffset + 2);
        _coordinates = QPoint(x, y);
    }
    if (_palette) {
        colors.clear();
        data+=fileOffset+(imageOffset-3*256);
        for (int i=0;i<256;i++) {
            // red, green, blue
            uchar r=*data++;
            uchar g=*data++;
            uchar b=*data++;
            // Convert 6bit to 8bit
            r = (r<<2) | (r>>4);
            g = (g<<2) | (g>>4);
            b = (b<<2) | (b>>4);
            QRgb color = 0xff000000 | (r<<16) | (g<<8) | b;
            colors.push_back(color);
        }
    }
    setColorTable(colors);
}

void DG_image::switchColorTable()
{
    QVector<QRgb> tmp=_altColorTable;
    _altColorTable=colorTable();
    setColorTable(tmp);
}
