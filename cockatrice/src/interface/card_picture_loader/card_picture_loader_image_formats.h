#ifndef COCKATRICE_CARD_PICTURE_LOADER_IMAGE_FORMATS_H
#define COCKATRICE_CARD_PICTURE_LOADER_IMAGE_FORMATS_H

#include <QBuffer>
#include <QByteArray>
#include <QByteArrayList>
#include <QImage>
#include <QImageReader>
#include <QIODevice>
#include <QMovie>
#include <QString>

namespace CardPictureLoaderImageFormats
{
inline QByteArray acceptHeader()
{
    // Decoder plugins differ across distributions. Prefer only formats this
    // runtime can read, then retain valid wildcards for other image endpoints.
    static const QByteArray header = [] {
        const auto supported = QImageReader::supportedMimeTypes();
        QByteArrayList types;
        for (const QByteArray &preferred :
             {QByteArray("image/avif"), QByteArray("image/webp"), QByteArray("image/apng"), QByteArray("image/png"),
              QByteArray("image/jpeg")}) {
            if (supported.contains(preferred)) {
                types.append(preferred);
            }
        }
        for (const QByteArray &type : supported) {
            if (type.startsWith("image/") && !types.contains(type)) {
                types.append(type);
            }
        }
        types.append("image/*;q=0.5");
        types.append("*/*;q=0.1");
        return types.join(',');
    }();
    return header;
}

struct DecodedImage
{
    QImage image;
    QString error;
};

inline DecodedImage readImage(QIODevice *device)
{
    static constexpr int riffHeaderSize = 12; // RIFF_HEADER_SIZE from webp/format_constants.h
    const auto header = device->peek(riffHeaderSize);
    if (header.startsWith("RIFF") && header.endsWith("WEBP")) {
        // Preserve the existing first-frame behavior for animated WebP files.
        QBuffer buffer;
        buffer.setData(device->readAll());
        QMovie movie(&buffer);
        movie.start();
        movie.stop();
        return {movie.currentImage(), movie.lastErrorString()};
    }

    QImageReader reader;
    reader.setDecideFormatFromContent(true);
    reader.setDevice(device);
    QImage image = reader.read();
    return {image, image.isNull() ? reader.errorString() : QString()};
}
} // namespace CardPictureLoaderImageFormats

#endif // COCKATRICE_CARD_PICTURE_LOADER_IMAGE_FORMATS_H
