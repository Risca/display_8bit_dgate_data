#ifndef DG_IMAGE_H
#define DG_IMAGE_H

#include <QImage>

class DG_image : public QImage
{
public:
    DG_image(const uchar * data,int width, int height);

};

#endif // DG_IMAGE_H
