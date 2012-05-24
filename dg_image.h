#ifndef DG_IMAGE_H
#define DG_IMAGE_H

#include <QImage>

#define DG_ClosedBook           0x0300 // No header
#define DG_SpellAnimation       0x0301 // 4 byte header
#define DG_NavigationAndItems   0x0500 // No header
#define DG_Animation            0x0501 // 4 byte header
#define DG_NoteOrOpenBook       0x1300 // 3*256 palette header
#define DG_BookAnimation        0x1301 // 4 byte (coordinates?)+ 3*256 palette header
#define DG_BigImage             0x1500 // 3*256 palette header
#define DG_BigAnimation         0x4501 // 4 byte header (coordinates?)

class DG_image : public QImage
{
public:
    DG_image(const uchar * data, quint32 fileOffset, quint32 imageOffset, int width, int height, quint16 type=true, bool palette=true);

    quint16 type() { return _type; }
    quint32 fileOffset() { return _fileOffset; }
    quint32 imageOffset() { return _imageOffset; }
private:
    quint16 _type;
    quint32 _fileOffset;
    quint32 _imageOffset;
};

#endif // DG_IMAGE_H
