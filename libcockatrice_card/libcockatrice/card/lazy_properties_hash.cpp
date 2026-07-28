#include "lazy_properties_hash.h"

#include <QIODevice>

LazyPropertiesHash::LazyPropertiesHash() : isMaterialized(true)
{
}

LazyPropertiesHash::LazyPropertiesHash(const QByteArray &blob) : blob(blob)
{
}

LazyPropertiesHash::LazyPropertiesHash(const QHash<QString, QString> &properties)
    : properties(properties), isMaterialized(true)
{
}

LazyPropertiesHash::LazyPropertiesHash(const LazyPropertiesHash &other)
{
    // since we do not allow dematerialization, we only need to lock if not materialized yet
    if (other.isMaterialized) {
        blob = other.blob;
        properties = other.properties;
        isMaterialized = true;
    } else {
        QMutexLocker lock(&other.propertiesMutex);
        blob = other.blob;
        properties = other.properties;
        isMaterialized = false;
    }
}

LazyPropertiesHash &LazyPropertiesHash::operator=(const LazyPropertiesHash &other)
{
    if (this == &other) {
        return *this;
    }

    // since we do not allow dematerialization, we only need to lock if not materialized yet
    if (other.isMaterialized) {
        blob = other.blob;
        properties = other.properties;
        isMaterialized = true;
    } else {
        QMutexLocker lock(&other.propertiesMutex);
        blob = other.blob;
        properties = other.properties;
        isMaterialized = false;
    }

    return *this;
}

void LazyPropertiesHash::ensureMaterialized() const
{
    QMutexLocker lock(&propertiesMutex);

    if (isMaterialized) {
        return;
    }

    if (!blob.isEmpty()) {
        QDataStream in(blob);
        in.setVersion(QDataStream::Qt_6_4);
        in >> properties;
    }

    blob.clear();

    isMaterialized = true;
}

QString LazyPropertiesHash::value(const QString &key) const
{
    ensureMaterialized();
    return properties.value(key);
}

void LazyPropertiesHash::insert(const QString &key, const QString &value)
{
    ensureMaterialized();
    properties.insert(key, value);
}

const QHash<QString, QString> &LazyPropertiesHash::getProperties() const
{
    ensureMaterialized();
    return properties;
}
