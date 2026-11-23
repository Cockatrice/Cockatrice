#include "tag_database.h"

#include <QBuffer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QList>
#include <QSet>
#include <QUuid>

#ifdef HAS_LZMA
#include "lzma/decompress.h"
#endif

#ifdef HAS_ZLIB
#include "zip/unzip.h"

#include <zlib.h>
#endif

#include <utility>

namespace
{
using TagSets = QHash<QString, QSet<QString>>;

struct ParsedTag
{
    QString slug;
    QStringList parentIds;
    QList<QString> oracleIds;
};

struct ParseContext
{
    bool includeParentTags = false;
    QHash<QString, ParsedTag> tagsById;
    TagSets oracleTags;
    TagSets nameTags;
};

const QByteArray &gzipMagic()
{
    static const QByteArray magic = QByteArray::fromHex("1f8b");
    return magic;
}

const QByteArray &xzMagic()
{
    static const QByteArray magic = QByteArray::fromHex("fd377a585a00");
    return magic;
}

const QByteArray &zipMagic()
{
    static const QByteArray magic = QByteArray::fromHex("504b0304");
    return magic;
}

QString normalizedSlug(const QString &slug)
{
    return slug.trimmed().toLower();
}

/**
 * @brief Rejects slugs that cannot survive the space-separated `tags` property.
 */
bool isValidSlug(const QString &slug)
{
    if (slug.isEmpty()) {
        return false;
    }
    for (const QChar &character : slug) {
        if (character.isSpace()) {
            return false;
        }
    }
    return true;
}

QString normalizedName(const QString &name)
{
    return name.trimmed().toLower();
}

/**
 * @brief True when a map key has the shape of a Scryfall oracle id.
 *
 * A manifest with an explicit `key` removes the ambiguity for card names that
 * happen to look like UUIDs.
 */
bool looksLikeOracleId(const QString &key)
{
    return !QUuid::fromString(key.trimmed()).isNull();
}

void addTags(TagSets &target, const QString &key, const QStringList &slugs)
{
    // Both key spaces are case-insensitive: Scryfall UUIDs are hex, and card
    // names must match regardless of the source's capitalization.
    const QString normalizedKey = key.trimmed().toLower();
    if (normalizedKey.isEmpty()) {
        return;
    }

    QSet<QString> &tags = target[normalizedKey];
    for (const QString &rawSlug : slugs) {
        const QString slug = normalizedSlug(rawSlug);
        if (isValidSlug(slug)) {
            tags.insert(slug);
        }
    }
    if (tags.isEmpty()) {
        target.remove(normalizedKey);
    }
}

#ifdef HAS_ZLIB
bool decompressGzip(const QByteArray &input, QByteArray &output, QString *errorMessage)
{
    z_stream stream{};
    if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
        if (errorMessage) {
            *errorMessage = TagDatabase::tr("Gzip support could not be initialized.");
        }
        return false;
    }

    stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(input.constData()));
    stream.avail_in = static_cast<uInt>(input.size());

    int result = Z_OK;
    char buffer[64 * 1024];
    while (result == Z_OK) {
        stream.next_out = reinterpret_cast<Bytef *>(buffer);
        stream.avail_out = sizeof(buffer);
        result = inflate(&stream, Z_NO_FLUSH);

        const uInt produced = static_cast<uInt>(sizeof(buffer)) - stream.avail_out;
        if (produced > 0) {
            output.append(buffer, static_cast<int>(produced));
        }

        if (result != Z_OK && result != Z_STREAM_END) {
            inflateEnd(&stream);
            if (errorMessage) {
                *errorMessage = TagDatabase::tr("Gzip decompression failed.");
            }
            return false;
        }
        if (produced == 0 && stream.avail_in == 0 && result == Z_OK) {
            inflateEnd(&stream);
            if (errorMessage) {
                *errorMessage = TagDatabase::tr("The gzip stream is truncated.");
            }
            return false;
        }
    }

    inflateEnd(&stream);
    return result == Z_STREAM_END;
}
#endif

#ifdef HAS_LZMA
bool decompressXz(const QByteArray &input, QByteArray &output, QString *errorMessage)
{
    QByteArray inputCopy = input;
    QBuffer inputBuffer(&inputCopy);
    QBuffer outputBuffer(&output);
    inputBuffer.open(QBuffer::ReadOnly);
    outputBuffer.open(QBuffer::WriteOnly);

    XzDecompressor decompressor;
    if (!decompressor.decompress(&inputBuffer, &outputBuffer)) {
        if (errorMessage) {
            *errorMessage = TagDatabase::tr("Xz decompression failed.");
        }
        return false;
    }
    return true;
}
#endif

#ifdef HAS_ZLIB
bool decompressZip(const QByteArray &input, QByteArray &output, QString *errorMessage)
{
    QByteArray inputCopy = input;
    QBuffer inputBuffer(&inputCopy);
    UnZip archive;
    const UnZip::ErrorCode openError = archive.openArchive(&inputBuffer);
    if (openError != UnZip::Ok) {
        if (errorMessage) {
            *errorMessage = TagDatabase::tr("Failed to open the Zip archive: %1.").arg(archive.formatError(openError));
        }
        return false;
    }

    const QStringList files = archive.fileList();
    if (files.size() != 1) {
        archive.closeArchive();
        if (errorMessage) {
            *errorMessage = TagDatabase::tr("The Zip archive must contain exactly one tag database file.");
        }
        return false;
    }

    QBuffer outputBuffer(&output);
    outputBuffer.open(QBuffer::ReadWrite);
    const UnZip::ErrorCode extractError = archive.extractFile(files.first(), &outputBuffer);
    archive.closeArchive();
    if (extractError != UnZip::Ok) {
        if (errorMessage) {
            *errorMessage = TagDatabase::tr("Zip extraction failed: %1.").arg(archive.formatError(extractError));
        }
        return false;
    }
    return true;
}
#endif

/**
 * @brief Unwraps gzip/xz/zip payloads, including nested archives.
 */
bool decompressData(QByteArray &data, QString *errorMessage)
{
    // Nested compression is legitimate (a zip around a gzip), but a handful of
    // layers is enough; the cap keeps a hostile file from looping forever.
    for (int layer = 0; layer < 4; ++layer) {
        if (data.startsWith(gzipMagic())) {
#ifdef HAS_ZLIB
            QByteArray decompressed;
            if (!decompressGzip(data, decompressed, errorMessage)) {
                return false;
            }
            data = std::move(decompressed);
            continue;
#else
            if (errorMessage) {
                *errorMessage = TagDatabase::tr("This version of Oracle does not support gzip compressed files.");
            }
            return false;
#endif
        }

        if (data.startsWith(xzMagic())) {
#ifdef HAS_LZMA
            QByteArray decompressed;
            if (!decompressXz(data, decompressed, errorMessage)) {
                return false;
            }
            data = std::move(decompressed);
            continue;
#else
            if (errorMessage) {
                *errorMessage = TagDatabase::tr("This version of Oracle does not support xz compressed files.");
            }
            return false;
#endif
        }

        if (data.startsWith(zipMagic())) {
#ifdef HAS_ZLIB
            QByteArray decompressed;
            if (!decompressZip(data, decompressed, errorMessage)) {
                return false;
            }
            data = std::move(decompressed);
            continue;
#else
            if (errorMessage) {
                *errorMessage = TagDatabase::tr("This version of Oracle does not support zipped files.");
            }
            return false;
#endif
        }

        return true;
    }

    if (errorMessage) {
        *errorMessage = TagDatabase::tr("The tag database is compressed in too many layers.");
    }
    return false;
}

bool parseKeyedMap(const QJsonObject &map, const QString &explicitKey, ParseContext &context, QString *errorMessage)
{
    if (map.isEmpty()) {
        if (errorMessage) {
            *errorMessage = TagDatabase::tr("The tag database contains no tag entries.");
        }
        return false;
    }

    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        if (!it.value().isArray()) {
            if (errorMessage) {
                *errorMessage = TagDatabase::tr("The tag list for \"%1\" must be an array of slugs.").arg(it.key());
            }
            return false;
        }

        QStringList slugs;
        for (const QJsonValue &value : it.value().toArray()) {
            if (!value.isString()) {
                if (errorMessage) {
                    *errorMessage =
                        TagDatabase::tr("The tag list for \"%1\" must contain only slug strings.").arg(it.key());
                }
                return false;
            }
            slugs.append(value.toString());
        }

        const bool oracleKey = explicitKey.isEmpty() ? looksLikeOracleId(it.key())
                                                     : (explicitKey == "oracle_id" || explicitKey == "oracleid");
        if (oracleKey) {
            addTags(context.oracleTags, it.key().trimmed(), slugs);
        } else {
            addTags(context.nameTags, normalizedName(it.key()), slugs);
        }
    }
    return true;
}

bool parseTagRow(const QJsonObject &object, ParseContext &context)
{
    QStringList slugs;
    for (const QJsonValue &value : object.value("tags").toArray()) {
        if (value.isString()) {
            slugs.append(value.toString());
        }
    }

    const QString oracleId = object.value("oracle_id").toString().trimmed();
    if (!oracleId.isEmpty()) {
        addTags(context.oracleTags, oracleId, slugs);
    }

    const QString name = object.value("name").toString().trimmed();
    if (!name.isEmpty()) {
        addTags(context.nameTags, normalizedName(name), slugs);
    }
    return true;
}

bool parseTagObject(const QJsonObject &object, ParseContext &context)
{
    // Art and illustration tags describe a printing's artwork, not the oracle
    // card, so they never belong in the card-level tag property.
    const QString type = object.value("type").toString().trimmed().toLower();
    if (!type.isEmpty() && type != "oracle") {
        return true;
    }

    const QString slug = normalizedSlug(object.value("slug").toString());
    if (!isValidSlug(slug)) {
        return true;
    }

    QList<QString> oracleIds;
    for (const QJsonValue &value : object.value("taggings").toArray()) {
        const QString oracleId = value.toObject().value("oracle_id").toString().trimmed();
        if (!oracleId.isEmpty() && !oracleIds.contains(oracleId)) {
            oracleIds.append(oracleId);
        }
    }

    const QString id = object.value("id").toString().trimmed();
    if (!context.includeParentTags || id.isEmpty()) {
        for (const QString &oracleId : oracleIds) {
            addTags(context.oracleTags, oracleId, QStringList{slug});
        }
        return true;
    }

    ParsedTag &tag = context.tagsById[id];
    if (tag.slug.isEmpty()) {
        tag.slug = slug;
    }
    for (const QJsonValue &parent : object.value("parent_ids").toArray()) {
        const QString parentId = parent.toString().trimmed();
        if (!parentId.isEmpty() && !tag.parentIds.contains(parentId)) {
            tag.parentIds.append(parentId);
        }
    }
    for (const QString &oracleId : oracleIds) {
        if (!tag.oracleIds.contains(oracleId)) {
            tag.oracleIds.append(oracleId);
        }
    }
    return true;
}

bool handleObject(const QJsonObject &object, ParseContext &context, QString *errorMessage)
{
    if (object.contains("slug")) {
        return parseTagObject(object, context);
    }

    const QJsonValue tagsValue = object.value("tags");
    if (tagsValue.isArray() && (object.contains("oracle_id") || object.contains("name"))) {
        return parseTagRow(object, context);
    }
    if (tagsValue.isObject()) {
        const QString key = object.value("key").toString().trimmed().toLower();
        return parseKeyedMap(tagsValue.toObject(), key, context, errorMessage);
    }

    bool allArrays = !object.isEmpty();
    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        if (!it.value().isArray()) {
            allArrays = false;
            break;
        }
    }
    if (allArrays) {
        return parseKeyedMap(object, QString(), context, errorMessage);
    }

    if (errorMessage) {
        *errorMessage = TagDatabase::tr("Unrecognized tag database format.");
    }
    return false;
}

bool parseJsonArray(const QJsonArray &array, ParseContext &context, QString *errorMessage)
{
    if (array.isEmpty()) {
        if (errorMessage) {
            *errorMessage = TagDatabase::tr("The tag database contains no tag entries.");
        }
        return false;
    }

    for (const QJsonValue &value : array) {
        if (!value.isObject()) {
            if (errorMessage) {
                *errorMessage = TagDatabase::tr("The tag database array must contain only objects.");
            }
            return false;
        }
        if (!handleObject(value.toObject(), context, errorMessage)) {
            return false;
        }
    }
    return true;
}

bool parseJsonLines(const QByteArray &payload, ParseContext &context, QString *errorMessage)
{
    const QList<QByteArray> lines = payload.split('\n');
    bool parsedAny = false;

    for (int index = 0; index < lines.size(); ++index) {
        const QByteArray line = lines.at(index).trimmed();
        if (line.isEmpty()) {
            continue;
        }

        QJsonParseError lineError{};
        const QJsonDocument lineDocument = QJsonDocument::fromJson(line, &lineError);
        if (lineError.error != QJsonParseError::NoError || !lineDocument.isObject()) {
            if (errorMessage) {
                *errorMessage = TagDatabase::tr("Malformed tag database at line %1: %2")
                                    .arg(index + 1)
                                    .arg(lineError.errorString());
            }
            return false;
        }
        if (!handleObject(lineDocument.object(), context, errorMessage)) {
            return false;
        }
        parsedAny = true;
    }

    if (!parsedAny) {
        if (errorMessage) {
            *errorMessage = TagDatabase::tr("The tag database contains no tag entries.");
        }
        return false;
    }
    return true;
}

QHash<QString, QStringList> sortedLists(const TagSets &sets)
{
    QHash<QString, QStringList> result;
    result.reserve(sets.size());
    for (auto it = sets.constBegin(); it != sets.constEnd(); ++it) {
        if (it.value().isEmpty()) {
            continue;
        }
        QStringList slugs = it.value().values();
        slugs.sort();
        result.insert(it.key(), slugs);
    }
    return result;
}

} // namespace

std::shared_ptr<TagDatabase>
TagDatabase::loadFromData(const QByteArray &data, const LoadOptions &options, QString *errorMessage)
{
    if (errorMessage) {
        errorMessage->clear();
    }

    if (data.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = tr("The tag database is empty.");
        }
        return nullptr;
    }

    QByteArray payload = data;
    if (!decompressData(payload, errorMessage)) {
        return nullptr;
    }
    if (payload.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = tr("The tag database is empty.");
        }
        return nullptr;
    }

    ParseContext context;
    context.includeParentTags = options.includeParentTags;

    QJsonParseError parseError{};
    const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error == QJsonParseError::NoError) {
        if (document.isArray()) {
            if (!parseJsonArray(document.array(), context, errorMessage)) {
                return nullptr;
            }
        } else if (document.isObject()) {
            if (!handleObject(document.object(), context, errorMessage)) {
                return nullptr;
            }
        } else {
            if (errorMessage) {
                *errorMessage = tr("The tag database is not a JSON object or array.");
            }
            return nullptr;
        }
    } else if (payload.trimmed().startsWith('[')) {
        // A broken array is not JSONL; report the parser error instead of
        // blaming an arbitrary line.
        if (errorMessage) {
            *errorMessage = tr("Malformed tag database: %1").arg(parseError.errorString());
        }
        return nullptr;
    } else if (!parseJsonLines(payload, context, errorMessage)) {
        return nullptr;
    }

    if (context.includeParentTags) {
        // Expand upward only after every tag is known: a parent object may
        // appear after its children in the file.
        for (auto it = context.tagsById.constBegin(); it != context.tagsById.constEnd(); ++it) {
            QStringList slugs{it->slug};
            QSet<QString> visited;
            QList<QString> pending = it->parentIds;

            while (!pending.isEmpty()) {
                const QString parentId = pending.takeLast();
                if (visited.contains(parentId)) {
                    continue;
                }
                visited.insert(parentId);

                const auto parent = context.tagsById.constFind(parentId);
                if (parent == context.tagsById.constEnd()) {
                    continue;
                }
                slugs.append(parent->slug);
                pending.append(parent->parentIds);
            }

            for (const QString &oracleId : it->oracleIds) {
                addTags(context.oracleTags, oracleId, slugs);
            }
        }
    }

    auto database = std::make_shared<TagDatabase>();
    database->tagsByOracleId = sortedLists(context.oracleTags);
    database->tagsByName = sortedLists(context.nameTags);

    QSet<QString> distinctSlugs;
    for (const QStringList &slugs : database->tagsByOracleId) {
        for (const QString &slug : slugs) {
            distinctSlugs.insert(slug);
        }
    }
    for (const QStringList &slugs : database->tagsByName) {
        for (const QString &slug : slugs) {
            distinctSlugs.insert(slug);
        }
    }
    database->distinctTagCount = distinctSlugs.size();

    if (database->isEmpty()) {
        if (errorMessage) {
            *errorMessage = tr("The tag database contains no usable tags.");
        }
        return nullptr;
    }
    return database;
}

QStringList TagDatabase::tagsForCard(const QString &oracleId, const QString &name) const
{
    QStringList result;
    if (!oracleId.isEmpty()) {
        result = tagsByOracleId.value(oracleId.trimmed().toLower());
    }

    const QString normalizedCardName = normalizedName(name);
    if (!normalizedCardName.isEmpty()) {
        const QStringList byName = tagsByName.value(normalizedCardName);
        for (const QString &slug : byName) {
            if (!result.contains(slug)) {
                result.append(slug);
            }
        }
    }

    if (!result.isEmpty()) {
        result.sort();
    }
    return result;
}

bool TagDatabase::isEmpty() const
{
    return tagsByOracleId.isEmpty() && tagsByName.isEmpty();
}
