#include "printing_info.h"

#include "../set/card_set.h"

#include <QDataStream>
#include <QIODevice>

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
        in.setVersion(QDataStream::Qt_6_4);
        in >> propertiesCache;
    }
    propertiesLoaded = true;
}

void PrintingInfo::setProperty(const QString &_name, const QString &_value)
{
    ensurePropertiesLoaded();
    if (propertiesCache.value(_name) == _value) {
        return;
    }
    propertiesCache.insert(_name, _value);
    setProperties(propertiesCache);
}

void PrintingInfo::setProperties(const QHash<QString, QString> &_props)
{
    ensurePropertiesLoaded();
    propertiesCache = _props;
    QDataStream out(&propertiesBlob, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_4);
    out << propertiesCache;
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