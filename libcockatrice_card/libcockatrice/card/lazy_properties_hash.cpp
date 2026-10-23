#include "lazy_properties_hash.h"

#include <QDataStream>
#include <QIODevice>
#include <QMutexLocker>

namespace
{
constexpr quint32 NULL_STRING_LENGTH = 0xFFFFFFFFu;

/**
 * @brief Reads a QDataStream-serialized QString length at @p pos, advancing past the length field.
 *
 * QDataStream writes a QString as a big-endian quint32 byte length followed by UTF-16 code units (also big-endian),
 * or 0xFFFFFFFF for a null string. Parsing at the byte level lets us skip and compare properties without
 * constructing a QString for every entry.
 */
bool readBlobLength(const QByteArray &blob, int &pos, quint32 &length)
{
    if (pos < 0 || static_cast<qint64>(pos) + 4 > blob.size()) {
        return false;
    }

    const auto *bytes = reinterpret_cast<const uchar *>(blob.constData()) + pos;
    length = (quint32(bytes[0]) << 24) | (quint32(bytes[1]) << 16) | (quint32(bytes[2]) << 8) | quint32(bytes[3]);
    pos += 4;

    if (length == NULL_STRING_LENGTH) {
        length = 0;
        return true;
    }

    if (length > static_cast<quint32>(blob.size() - pos)) {
        return false;
    }

    return true;
}

/**
 * @brief Compares the serialized bytes at @p pos with @p key without allocating a QString.
 */
bool blobKeyEquals(const QByteArray &blob, int pos, quint32 length, const QString &key)
{
    if (length != static_cast<quint32>(key.size()) * 2) {
        return false;
    }

    const auto *bytes = reinterpret_cast<const uchar *>(blob.constData()) + pos;
    for (int i = 0; i < key.size(); ++i) {
        const ushort codeUnit = static_cast<ushort>((bytes[2 * i] << 8) | bytes[2 * i + 1]);
        if (key.at(i).unicode() != codeUnit) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Builds a QString from the serialized UTF-16BE bytes at @p pos.
 */
QString blobString(const QByteArray &blob, int pos, quint32 length)
{
    const int codeUnits = static_cast<int>(length / 2);
    QString result;
    result.resize(codeUnits);

    const auto *bytes = reinterpret_cast<const uchar *>(blob.constData()) + pos;
    for (int i = 0; i < codeUnits; ++i) {
        result[i] = QChar(static_cast<ushort>((bytes[2 * i] << 8) | bytes[2 * i + 1]));
    }

    return result;
}

/**
 * @brief Iterates over the key/value pairs of a serialized QHash blob without materializing it.
 */
class BlobReader
{
public:
    explicit BlobReader(const QByteArray &_blob) : blob(_blob)
    {
        if (blob.size() < 4) {
            return;
        }

        const auto *bytes = reinterpret_cast<const uchar *>(blob.constData());
        remaining =
            (quint32(bytes[0]) << 24) | (quint32(bytes[1]) << 16) | (quint32(bytes[2]) << 8) | quint32(bytes[3]);
        pos = 4;
        valid = true;
    }

    /**
     * @brief Reads the next pair's offsets.
     * @return False when there are no more pairs or the blob is malformed.
     */
    bool next(int &keyPos, quint32 &keyLength, int &valuePos, quint32 &valueLength)
    {
        if (!valid || remaining == 0) {
            return false;
        }

        if (!readBlobLength(blob, pos, keyLength)) {
            valid = false;
            return false;
        }
        keyPos = pos;
        pos += static_cast<int>(keyLength);

        if (!readBlobLength(blob, pos, valueLength)) {
            valid = false;
            return false;
        }
        valuePos = pos;
        pos += static_cast<int>(valueLength);

        --remaining;
        return true;
    }

    [[nodiscard]] quint32 entryCount() const
    {
        return valid ? remaining : 0;
    }

private:
    const QByteArray &blob;
    int pos = 4;
    quint32 remaining = 0;
    bool valid = false;
};

/**
 * @brief Looks up @p key in a serialized blob without materializing it.
 */
QString blobValue(const QByteArray &blob, const QString &key)
{
    BlobReader reader(blob);
    int keyPos = 0;
    int valuePos = 0;
    quint32 keyLength = 0;
    quint32 valueLength = 0;
    while (reader.next(keyPos, keyLength, valuePos, valueLength)) {
        if (blobKeyEquals(blob, keyPos, keyLength, key)) {
            return blobString(blob, valuePos, valueLength);
        }
    }

    return {};
}

/**
 * @brief Checks whether @p key is present in a serialized blob without materializing it.
 */
bool blobContains(const QByteArray &blob, const QString &key)
{
    BlobReader reader(blob);
    int keyPos = 0;
    int valuePos = 0;
    quint32 keyLength = 0;
    quint32 valueLength = 0;
    while (reader.next(keyPos, keyLength, valuePos, valueLength)) {
        if (blobKeyEquals(blob, keyPos, keyLength, key)) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Lists the keys of a serialized blob without materializing it.
 */
QStringList blobKeys(const QByteArray &blob)
{
    BlobReader reader(blob);
    QStringList result;
    result.reserve(static_cast<int>(reader.entryCount()));

    int keyPos = 0;
    int valuePos = 0;
    quint32 keyLength = 0;
    quint32 valueLength = 0;
    while (reader.next(keyPos, keyLength, valuePos, valueLength)) {
        result.append(blobString(blob, keyPos, keyLength));
    }

    return result;
}
} // namespace

LazyPropertiesHash::LazyPropertiesHash() : isMaterialized(true)
{
}

LazyPropertiesHash::LazyPropertiesHash(const QByteArray &blob) : blob(blob)
{
}

LazyPropertiesHash::LazyPropertiesHash(const QHash<QString, QString> &properties)
    : properties(properties), isMaterialized(true)
{
}

LazyPropertiesHash::LazyPropertiesHash(const LazyPropertiesHash &other)
{
    // since we do not allow dematerialization, we only need to lock if not materialized yet
    if (other.isMaterialized.load(std::memory_order_acquire)) {
        blob = other.blob;
        properties = other.properties;
        isMaterialized.store(true, std::memory_order_relaxed);
    } else {
        QMutexLocker lock(&other.propertiesMutex);
        blob = other.blob;
        properties = other.properties;
        isMaterialized.store(other.isMaterialized.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }
}

LazyPropertiesHash &LazyPropertiesHash::operator=(const LazyPropertiesHash &other)
{
    if (this == &other) {
        return *this;
    }

    // since we do not allow dematerialization, we only need to lock if not materialized yet
    if (other.isMaterialized.load(std::memory_order_acquire)) {
        blob = other.blob;
        properties = other.properties;
        isMaterialized.store(true, std::memory_order_relaxed);
    } else {
        QMutexLocker lock(&other.propertiesMutex);
        blob = other.blob;
        properties = other.properties;
        isMaterialized.store(false, std::memory_order_relaxed);
    }

    return *this;
}

void LazyPropertiesHash::ensureMaterialized() const
{
    // Materialization happens once and is never undone, so the common case can
    // skip the mutex entirely once it has happened.
    if (isMaterialized.load(std::memory_order_acquire)) {
        return;
    }

    QMutexLocker lock(&propertiesMutex);

    if (isMaterialized.load(std::memory_order_relaxed)) {
        return;
    }

    if (!blob.isEmpty()) {
        QDataStream in(blob);
        in.setVersion(QDataStream::Qt_6_4);
        in >> properties;
    }

    blob.clear();

    isMaterialized.store(true, std::memory_order_release);
}

QByteArray LazyPropertiesHash::serialize(const QHash<QString, QString> &properties)
{
    QByteArray result;
    QDataStream out(&result, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_4);
    out << properties;
    return result;
}

QString LazyPropertiesHash::value(const QString &key) const
{
    return read([&key](const QHash<QString, QString> &materialized) { return materialized.value(key); },
                [&key](const QByteArray &blob) { return blobValue(blob, key); });
}

bool LazyPropertiesHash::contains(const QString &key) const
{
    return read([&key](const QHash<QString, QString> &materialized) { return materialized.contains(key); },
                [&key](const QByteArray &blob) { return blobContains(blob, key); });
}

int LazyPropertiesHash::size() const
{
    return read([](const QHash<QString, QString> &materialized) { return static_cast<int>(materialized.size()); },
                [](const QByteArray &blob) { return static_cast<int>(BlobReader(blob).entryCount()); });
}

bool LazyPropertiesHash::isEmpty() const
{
    return size() == 0;
}

QStringList LazyPropertiesHash::keys() const
{
    return read([](const QHash<QString, QString> &materialized) { return QStringList(materialized.keys()); },
                [](const QByteArray &blob) { return blobKeys(blob); });
}

QByteArray LazyPropertiesHash::serialized() const
{
    return read([](const QHash<QString, QString> &materialized) { return serialize(materialized); },
                [](const QByteArray &blob) {
                    // An empty blob is the "unknown/absent" form used by failed cache reads; give callers a
                    // well-formed serialization of the (empty) property map instead.
                    return blob.isEmpty() ? serialize({}) : blob;
                });
}

bool LazyPropertiesHash::insert(const QString &key, const QString &value)
{
    ensureMaterialized();

    if (value == properties.value(key)) {
        return false;
    }

    properties.insert(key, value);
    return true;
}

const QHash<QString, QString> &LazyPropertiesHash::getProperties() const
{
    ensureMaterialized();
    return properties;
}
