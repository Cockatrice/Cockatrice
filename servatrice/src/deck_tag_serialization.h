#ifndef DECK_TAG_SERIALIZATION_H
#define DECK_TAG_SERIALIZATION_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QString>
#include <QStringList>

/**
 * @brief Encodes deck tags as a compact JSON array for storage in a text column.
 *
 * Deck tags are stored as JSON (rather than a delimited string) so tag names may
 * contain any character, and decoded uniformly everywhere they are read.
 */
inline QString serializeDeckTags(const QStringList &tags)
{
    QJsonArray array;
    for (const QString &tag : tags) {
        array.append(tag);
    }
    return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
}

/** @brief Decodes deck tags previously written by serializeDeckTags. */
inline QStringList deserializeDeckTags(const QString &serialized)
{
    QStringList tags;
    if (serialized.isEmpty()) {
        return tags;
    }
    const QJsonArray array = QJsonDocument::fromJson(serialized.toUtf8()).array();
    for (const QJsonValue &tag : array) {
        tags.append(tag.toString());
    }
    return tags;
}

#endif // DECK_TAG_SERIALIZATION_H
