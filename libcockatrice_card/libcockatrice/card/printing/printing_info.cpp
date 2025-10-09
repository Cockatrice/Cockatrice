#include "printing_info.h"

#include "../set/card_set.h"

PrintingInfo::PrintingInfo(const CardSetPtr &_set) : set(_set)
{
}

/**
 * Gets the uuid property of the printing, or an empty string if the property isn't present
 */
QString PrintingInfo::getUuid() const
{
    return properties.value("uuid").toString();
}