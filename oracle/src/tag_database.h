#ifndef TAG_DATABASE_H
#define TAG_DATABASE_H

#include <QByteArray>
#include <QCoreApplication>
#include <QHash>
#include <QString>
#include <QStringList>
#include <memory>

/**
 * @brief Tag slugs keyed by Scryfall oracle id and/or card name.
 *
 * Loaded from Scryfall's Oracle Tags bulk data or from a user-supplied tag
 * database. The data is import-time only: Oracle applies the slugs to the
 * `tags` card property before writing cards.xml, so the client needs no
 * tag-specific code.
 *
 * Accepted inputs, auto-detected:
 *  - Scryfall tag objects, either a JSON array or one object per JSONL line;
 *  - a flat map of `oracle_id` (or card name) to an array of slugs;
 *  - a manifest object `{"key": "oracle_id"|"name", "tags": { ... }}`;
 *  - JSONL rows `{"oracle_id": "...", "tags": [...]}` or the `name` equivalent.
 *
 * Gzip, xz and zip inputs are decompressed transparently when the build has
 * the matching library. Scryfall serves its bulk data as gzipped JSONL.
 */
class TagDatabase
{
    Q_DECLARE_TR_FUNCTIONS(TagDatabase)
public:
    struct LoadOptions
    {
        bool includeParentTags = false;
    };

    /**
     * @brief Parses and decompresses a tag database.
     * @return The database, or nullptr with @p errorMessage set on failure.
     */
    static std::shared_ptr<TagDatabase>
    loadFromData(const QByteArray &data, const LoadOptions &options, QString *errorMessage = nullptr);

    /**
     * @brief Returns the sorted slugs for one card.
     *
     * Oracle ids are the primary key. A name-keyed database is consulted as a
     * fallback, case-insensitively. An empty list means the card has no tags.
     */
    [[nodiscard]] QStringList tagsForCard(const QString &oracleId, const QString &name) const;
    [[nodiscard]] bool isEmpty() const;
    /** @brief Number of distinct slugs in the database. */
    [[nodiscard]] int tagCount() const
    {
        return distinctTagCount;
    }
    /** @brief Number of cards that received at least one tag. */
    [[nodiscard]] int taggedCardCount() const
    {
        return tagsByOracleId.size() + tagsByName.size();
    }

private:
    QHash<QString, QStringList> tagsByOracleId;
    QHash<QString, QStringList> tagsByName; ///< keys are lowercased
    int distinctTagCount = 0;
};

#endif // TAG_DATABASE_H
