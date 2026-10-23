#ifndef COCKATRICE_LAZY_PROPERTIES_HASH_H
#define COCKATRICE_LAZY_PROPERTIES_HASH_H

#include <QByteArray>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QStringList>
#include <atomic>

/**
 * @brief A property map that can lazily deserialize blobs to avoid loading overhead.
 *
 * Properties are stored as a pre-serialized blob (cheap to load), and individual values are read straight out of that
 * blob, so the database load and ordinary queries avoid constructing thousands of QString per card.
 *
 * A full QHash<QString, QString> is only built on the first insert() or getProperties() call. Once materialized, it
 * cannot be unmaterialized; create a new LazyPropertiesHash to return to the blob form.
 */
class LazyPropertiesHash
{

    mutable QByteArray blob;                         ///< Serialized properties (load form).
    mutable QHash<QString, QString> properties;      ///< Materialized properties (query form).
    mutable QMutex propertiesMutex;                  ///< Guards lazy materialization.
    mutable std::atomic<bool> isMaterialized{false}; ///< Whether properties is valid.

    /**
     * @brief Materializes properties from blob if not already done. Clears blob afterward.
     * Safe to call from const getters (members are mutable).
     */
    void ensureMaterialized() const;

    /**
     * @brief Runs the accessor for the current storage form.
     *
     * The common case skips the mutex once the properties are materialized. While they are
     * still in blob form, the mutex is taken and @p onBlob reads the values straight out of
     * the blob without building the hash, so ordinary queries stay allocation-light.
     *
     * @param onMaterialized Called with the materialized hash when it is available.
     * @param onBlob Called with the blob when it is still the source of truth.
     * @return Whatever the invoked accessor returns; both accessors must return the same type.
     */
    template <typename MaterializedFn, typename BlobFn>
    auto read(MaterializedFn &&onMaterialized, BlobFn &&onBlob) const
    {
        if (isMaterialized.load(std::memory_order_acquire)) {
            return onMaterialized(properties);
        }

        QMutexLocker lock(&propertiesMutex);
        if (isMaterialized.load(std::memory_order_relaxed)) {
            return onMaterialized(properties);
        }

        return onBlob(blob);
    }

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
     * @brief Serializes a property map into the blob form understood by this class.
     * @param properties The properties to serialize
     * @return The serialized blob
     */
    static QByteArray serialize(const QHash<QString, QString> &properties);

    /**
     * @brief Gets the value for a key, reading it from the blob when possible.
     * @param key The key
     * @return The value, or an empty string if the key is not present
     */
    QString value(const QString &key) const;

    /**
     * @brief Checks whether a key is present, reading the blob when possible.
     * @param key The key
     * @return True if the key is present
     */
    bool contains(const QString &key) const;

    /**
     * @brief Returns the number of properties.
     */
    int size() const;

    /**
     * @brief Returns whether there are no properties.
     */
    bool isEmpty() const;

    /**
     * @brief Returns the property names, reading them from the blob when possible.
     */
    QStringList keys() const;

    /**
     * @brief Returns the properties in serialized blob form.
     *
     * While the object is unmaterialized this is a cheap copy of the stored blob; after materialization the hash is
     * serialized on demand.
     */
    QByteArray serialized() const;

    /**
     * @brief Inserts a value into the materialized properties hash
     * @param key The key
     * @param value The value to insert
     * @return True if a new value was inserted; false if the new value is the same as the existing value
     */
    bool insert(const QString &key, const QString &value);

    /**
     * @brief Gets a view of the materialized properties hash, materializing it if needed.
     * @return The properties hash
     */
    const QHash<QString, QString> &getProperties() const;
};

#endif // COCKATRICE_LAZY_PROPERTIES_HASH_H
