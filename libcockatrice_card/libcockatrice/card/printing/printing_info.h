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
 * Info relating to a specific printing for a card.
 */
class PrintingInfo
{
public:
    explicit PrintingInfo(const CardSetPtr &_set = nullptr);
    ~PrintingInfo() = default;

    bool operator==(const PrintingInfo &other) const
    {
        return this->set == other.set && this->properties == other.properties;
    }

private:
    CardSetPtr set;
    // per-printing card properties;
    QVariantHash properties;

public:
    CardSetPtr getSet() const
    {
        return set;
    }

    QStringList getProperties() const
    {
        return properties.keys();
    }
    QString getProperty(const QString &propertyName) const
    {
        return properties.value(propertyName).toString();
    }
    void setProperty(const QString &_name, const QString &_value)
    {
        properties.insert(_name, _value);
    }

    QString getUuid() const;
};

#endif // COCKATRICE_PRINTING_INFO_H
