#include "dg_image.h"

DG_image::DG_image(const uchar *data, int width, int height) :
    QImage(data+3*256,width,height,width,QImage::Format_Indexed8)
{
    QVector<QRgb> _colors;

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
    setColorTable(_colors);
}
