#include "lazy_properties_hash.h"

#include <QDataStream>
#include <QMutexLocker>
#include <QSharedPointer>

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
    if (other.isMaterialized.load(std::memory_order_acquire)) {
        blob = other.blob;
        properties = other.properties;
        isMaterialized.store(true, std::memory_order_relaxed);
    } else {
        QMutexLocker lock(&other.propertiesMutex);
        blob = other.blob;
        properties = other.properties;
        isMaterialized.store(other.isMaterialized.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }
}

LazyPropertiesHash &LazyPropertiesHash::operator=(const LazyPropertiesHash &other)
{
    if (this == &other) {
        return *this;
    }

    // since we do not allow dematerialization, we only need to lock if not materialized yet
    if (other.isMaterialized.load(std::memory_order_acquire)) {
        blob = other.blob;
        properties = other.properties;
        isMaterialized.store(true, std::memory_order_relaxed);
    } else {
        QMutexLocker lock(&other.propertiesMutex);
        blob = other.blob;
        properties = other.properties;
        isMaterialized.store(false, std::memory_order_relaxed);
    }

    return *this;
}

void LazyPropertiesHash::ensureMaterialized() const
{
    // Materialization happens once and is never undone, so the common case can
    // skip the mutex entirely once it has happened.
    if (isMaterialized.load(std::memory_order_acquire)) {
        return;
    }

    QMutexLocker lock(&propertiesMutex);

    if (isMaterialized.load(std::memory_order_relaxed)) {
        return;
    }

    if (!blob.isEmpty()) {
        QDataStream in(blob);
        in.setVersion(QDataStream::Qt_6_4);
        in >> properties;
    }

    blob.clear();

    isMaterialized.store(true, std::memory_order_release);
}

QString LazyPropertiesHash::value(const QString &key) const
{
    ensureMaterialized();
    return properties.value(key);
}

bool LazyPropertiesHash::insert(const QString &key, const QString &value)
{
    ensureMaterialized();

    if (value == properties.value(key)) {
        return false;
    }

    properties.insert(key, value);
    return true;
}

const QHash<QString, QString> &LazyPropertiesHash::getProperties() const
{
    ensureMaterialized();
    return properties;
}
