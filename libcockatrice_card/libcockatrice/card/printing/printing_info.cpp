#include "printing_info.h"

#include "../set/card_set.h"

#include <QDataStream>

PrintingInfo::PrintingInfo(const CardSetPtr &_set) : set(_set)
{
}

void PrintingInfo::ensurePropertiesLoaded() const
{
    QMutexLocker lock(propertiesMutex.data());
    if (propertiesLoaded) {
        return;
    }
    propertiesCache.clear();
    if (!propertiesBlob.isEmpty()) {
        QDataStream in(propertiesBlob);
        in.setVersion(QDataStream::Qt_6_3);
        in >> propertiesCache;
    }
    propertiesLoaded = true;
}

/**
 * Gets the uuid property of the printing, or an empty string if the property isn't present
 */
QString PrintingInfo::getUuid() const
{
    return getPropertiesHash().value("uuid").toString();
}

QString PrintingInfo::getFlavorName() const
{
    return getPropertiesHash().value("flavorName").toString();
}