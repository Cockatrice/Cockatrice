#ifndef COCKATRICE_LAZY_PROPERTIES_HASH_H
#define COCKATRICE_LAZY_PROPERTIES_HASH_H

#include <QHash>
#include <QMutex>

/**
 * @brief A property map that can lazily deserialize blobs to avoid loading overhead.
 *
 * Properties are stored as a pre-serialized blob (cheap to load), and the QString is materialized on the first query,
 * so the database load avoids constructing thousands of QString per card.
 *
 * Once the properties are materialized, it cannot be unmaterialized.
 * If you want to reset the properties to an unmaterialized state, you should create a new LazyPropertiesHash.
 */
class LazyPropertiesHash
{

    mutable QByteArray blob;                    ///< Serialized properties (load form).
    mutable QHash<QString, QString> properties; ///< Materialized properties (query form).
    mutable QMutex propertiesMutex;             ///< Guards lazy materialization.
    mutable bool isMaterialized = false;        ///< Whether propertiesCache is valid.

    /**
     * @brief Materializes properties from blob if not already done. Clears blob afterward.
     * Safe to call from const getters (members are mutable).
     */
    void ensureMaterialized() const;

public:
    /**
     * @brief Default constructor.
     */
    LazyPropertiesHash();

    /**
     * @brief Creates an unmaterialized LazyPropertiesHash
     * @param blob The pre-serialized blob
     */
    explicit LazyPropertiesHash(const QByteArray &blob);

    /**
     * @brief Creates an already-materialized LazyPropertiesHash
     * @param properties The properties
     */
    explicit LazyPropertiesHash(const QHash<QString, QString> &properties);

    // Override copy constructor and copy-assignment because mutex isn't copiable
    LazyPropertiesHash(const LazyPropertiesHash &other);
    LazyPropertiesHash &operator=(const LazyPropertiesHash &other);

    /**
     * @brief Gets the value from the materialized properties hash
     * @param key The key
     * @return The value, or an empty string if the key is not present
     */
    QString value(const QString &key) const;

    /**
     * @brief Inserts a value into the materialized properties hash
     * @param key The key
     * @param value The value to insert
     */
    void insert(const QString &key, const QString &value);

    /**
     * @brief Gets a view of the materialized properties hash.
     * @return The properties hash
     */
    const QHash<QString, QString> &getProperties() const;
};

#endif // COCKATRICE_LAZY_PROPERTIES_HASH_H
