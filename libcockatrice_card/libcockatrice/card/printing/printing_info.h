#ifndef COCKATRICE_PRINTING_INFO_H
#define COCKATRICE_PRINTING_INFO_H

#include "../set/card_set.h"

#include <QList>
#include <QMap>
#include <QStringList>
#include <QVariant>

class PrintingInfo;

using SetToPrintingsMap = QMap<QString, QList<PrintingInfo>>;

/**
 * @class PrintingInfo
 * @ingroup CardPrintings
 *
 * @brief Represents metadata for a specific variation of a card within a set.
 *
 * A card can have multiple variations across sets. PrintingInfo associates
 * a card with one such variation, and provides per-printing attributes
 * such as identifiers or additional properties.
 *
 * Equality is defined as both the set and the property values being equal.
 */
class PrintingInfo
{
public:
    /**
     * @brief Constructs a PrintingInfo associated with a specific set.
     *
     * @param _set The set this printing belongs to (defaults to null).
     */
    explicit PrintingInfo(const CardSetPtr &_set = nullptr);

    /**
     * @brief Destroys the PrintingInfo.
     *
     * Defaulted since no special cleanup is required.
     */
    ~PrintingInfo() = default;

    /**
     * @brief Equality operator.
     *
     * Two PrintingInfo objects are equal if they refer to the same set
     * and contain the exact same property key/value pairs.
     *
     * @param other Another PrintingInfo to compare against.
     * @return True if both set and properties are equal, otherwise false.
     */
    bool operator==(const PrintingInfo &other) const
    {
        return this->set == other.set && this->properties == other.properties;
    }

private:
    CardSetPtr set;          ///< The set this variation belongs to.
    QVariantHash properties; ///< Key-value store for variation-specific attributes.

public:
    /**
     * @brief Returns the set this printing belongs to.
     *
     * @return Pointer to the associated CardSet.
     */
    CardSetPtr getSet() const
    {
        return set;
    }

    /**
     * @brief Returns the list of property names defined for this printing.
     *
     * @return List of keys stored in the properties map.
     */
    QStringList getProperties() const
    {
        return properties.keys();
    }

    /**
     * @brief Retrieves the value of a specific property.
     *
     * @param propertyName The key name of the property to query.
     * @return The property value as a string, or an empty string if not set.
     */
    QString getProperty(const QString &propertyName) const
    {
        return properties.value(propertyName).toString();
    }

    /**
     * @brief Sets or updates the value of a specific property.
     *
     * If the property already exists, its value is replaced.
     *
     * @param _name The name of the property.
     * @param _value The string value to assign.
     */
    void setProperty(const QString &_name, const QString &_value)
    {
        properties.insert(_name, _value);
    }

    /**
     * @brief Returns the providerID for this printing.
     *
     * @return A string representing the providerID.
     */
    QString getUuid() const;

    /**
     * @brief Returns the flavorName for this printing.
     *
     * @return The flavorName, or empty if it isn't present.
     */
    QString getFlavorName() const;
};

#endif // COCKATRICE_PRINTING_INFO_H
