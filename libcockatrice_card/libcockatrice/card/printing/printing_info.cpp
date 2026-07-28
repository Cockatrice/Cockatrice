#include "printing_info.h"

#include "../set/card_set.h"

#include <QDataStream>
#include <QIODevice>

PrintingInfo::PrintingInfo(const CardSetPtr &_set, const LazyPropertiesHash &_properties)
    : set(_set), properties(_properties)
{
}

void PrintingInfo::setProperty(const QString &_name, const QString &_value)
{
    properties.insert(_name, _value);
}

/**
 * Gets the uuid property of the printing, or an empty string if the property isn't present
 */
QString PrintingInfo::getUuid() const
{
    return getPropertiesHash().value("uuid");
}

QString PrintingInfo::getFlavorName() const
{
    return getPropertiesHash().value("flavorName");
}
