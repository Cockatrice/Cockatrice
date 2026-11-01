#include "printing_info.h"

#include "../set/card_set.h"

PrintingInfo::PrintingInfo(const CardSetPtr &_set, const LazyPropertiesHash &_properties)
    : set(_set), properties(_properties)
{
}

void PrintingInfo::setProperty(const QString &_name, const QString &_value)
{
    properties.insert(_name, _value);
}

bool PrintingInfo::operator==(const PrintingInfo &other) const
{
    if (set != other.set || properties.size() != other.properties.size()) {
        return false;
    }

    // Both property maps are usually still in blob form here, so compare key by key instead of
    // materializing two full hashes just to compare them.
    for (const QString &key : properties.keys()) {
        if (!other.properties.contains(key) || properties.value(key) != other.properties.value(key)) {
            return false;
        }
    }

    return true;
}

/**
 * Gets the uuid property of the printing, or an empty string if the property isn't present
 */
QString PrintingInfo::getUuid() const
{
    return getProperty("uuid");
}

QString PrintingInfo::getFlavorName() const
{
    return getProperty("flavorName");
}
