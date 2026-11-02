#ifndef XZ_DECOMPRESS_H
#define XZ_DECOMPRESS_H

#include <lzma.h>
#include <QIODevice>

class XzDecompressor : public QObject
{
    Q_OBJECT
public:
    XzDecompressor(QObject *parent = 0);
    ~XzDecompressor() { };
    bool decompress(QIODevice *in, QIODevice *out);
private:
    bool init_decoder(lzma_stream *strm);
    bool internal_decompress(lzma_stream *strm, QIODevice *in, QIODevice *out);
};

#endif
