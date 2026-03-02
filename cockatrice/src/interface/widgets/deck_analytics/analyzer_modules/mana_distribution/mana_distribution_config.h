#ifndef COCKATRICE_MANA_DISTRIBUTION_CONFIG_H
#define COCKATRICE_MANA_DISTRIBUTION_CONFIG_H

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>

struct ManaDistributionConfig
{
    QString displayType = "pie"; // "pie" or "bar"
    QStringList filters;         // empty = all colors
    bool showColorRows = true;

    QJsonObject toJson() const;

    static ManaDistributionConfig fromJson(const QJsonObject &o);
};

#endif // COCKATRICE_MANA_DISTRIBUTION_CONFIG_H
