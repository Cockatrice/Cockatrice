#ifndef XZ_DECOMPRESS_H
#define XZ_DECOMPRESS_H

#include <lzma.h>
#include <QBuffer>

class XzDecompressor : public QObject
{
    Q_OBJECT
public:
    XzDecompressor(QObject *parent = 0);
    ~XzDecompressor() { };
    bool decompress(QBuffer *in, QBuffer *out);
private:
    bool init_decoder(lzma_stream *strm);
    bool internal_decompress(lzma_stream *strm, QBuffer *in, QBuffer *out);
};

#endif
