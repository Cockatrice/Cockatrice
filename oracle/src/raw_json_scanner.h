#ifndef RAW_JSON_SCANNER_H
#define RAW_JSON_SCANNER_H

#include <QByteArray>
#include <QList>
#include <QString>

namespace RawJson
{

/**
 * @brief The byte extent of a set's object inside the scanned document, plus
 * the size of its cards array. This is the slice SetToDownload needs for lazy
 * per-set parsing; the metadata strings live in SetRange alongside it.
 */
struct SetDataRange
{
    /** @brief Byte offset of the set's object within the scanned buffer. */
    qsizetype start = -1;
    /** @brief Byte length of the set's object, including the surrounding braces. */
    qsizetype length = 0;
    /** @brief Number of entries in the set's "cards" array. */
    int cardCount = 0;
};

struct SetRange
{
    /** @brief The byte slice of this set within the document. */
    SetDataRange dataRange;
    QString code;
    QString name;
    QString type;
    QString releaseDate;
};

struct ScanError
{
    bool isError() const
    {
        return !message.isEmpty();
    }
    QString message;
};

/**
 * @brief Scans a full MTGJSON document without materializing the JSON tree.
 *
 * Splits the top-level "data" object into per-set byte ranges and reads each
 * set's metadata directly from the raw bytes. The oracle importer can then
 * parse one set at a time during import, keeping peak memory far below a single
 * QJsonDocument::fromJson() over the whole file.
 *
 * The whole document is structurally validated while scanning (strings,
 * escapes, braces, and a trailing-content check) and nesting depth is capped at
 * 1024 to match QJsonDocument, so pathologically deep documents fail shallowly
 * instead of exhausting the stack. Verdicts agree with QJsonDocument::fromJson
 * on structurally malformed input; unlike Qt, string metadata fields
 * ("name", "type", "releaseDate", "code") tolerate null / non-string values by
 * defaulting to empty rather than rejecting the whole document, so one broken
 * set cannot abort the import of the rest.
 *
 * Following QJsonDocument::fromJson's convention, the parsed ranges are
 * returned by value and any failure is reported through the @p error out
 * parameter.
 *
 * @param json The raw MTGJSON document bytes.
 * @param error Out parameter. Set to an error ScanError when the document
 *              cannot be parsed, otherwise left empty. Passing a null
 *              pointer disables error reporting.
 * @return The detected per-set ranges, or an empty list on failure.
 */
QList<SetRange> scanSetRanges(const QByteArray &json, ScanError *error = nullptr);

} // namespace RawJson

#endif // RAW_JSON_SCANNER_H