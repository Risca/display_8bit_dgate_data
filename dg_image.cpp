#include "dg_image.h"

DG_image::DG_image(const uchar *data, quint32 fileOffset, quint32 imageOffset, int width, int height, quint16 type, bool palette) :
    QImage(data+fileOffset+imageOffset,width,height,width,QImage::Format_Indexed8),
    _type(type),
    _fileOffset(fileOffset),
    _imageOffset(imageOffset)
{
    QVector<QRgb> _colors;
    if (palette) {
        data+=fileOffset+((imageOffset==3*256) ? 0 : imageOffset-3*256);
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
            _colors.push_back(color);
        }
    } else {
        for (int i=0;i<256;i++) {
            QRgb color = 0xff000000 | (i<<16) | (i<<8) | i;
            _colors.push_back(color);
        }
    }
    setColorTable(_colors);
}
