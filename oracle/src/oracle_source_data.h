#ifndef ORACLE_SOURCE_DATA_H
#define ORACLE_SOURCE_DATA_H

#include <QByteArray>
#include <QTemporaryFile>

/**
 * An owned snapshot of an Oracle input. Downloads and decompression write to
 * disk; the JSON scanner and importer share a read-only mapping of the result.
 * The mapping remains valid until this object is destroyed. Access is serial:
 * the download finishes before a worker takes ownership of the input.
 */
class OracleSourceData
{
public:
    bool open(QString &error);
    bool append(QIODevice &input, QString &error);
    bool rewind(QString &error);
    bool map(QString &error);
    bool copyTo(QIODevice &output, QString &error);

    QTemporaryFile &device()
    {
        return file;
    }
    qint64 size() const
    {
        return file.size();
    }
    QByteArray bytes() const;

private:
    QTemporaryFile file;
    QFile mappedFile;
    uchar *mapping = nullptr;
};

#endif
