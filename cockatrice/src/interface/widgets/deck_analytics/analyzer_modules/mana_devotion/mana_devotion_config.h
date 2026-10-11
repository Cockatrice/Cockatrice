#ifndef COCKATRICE_MANA_DEVOTION_CONFIG_H
#define COCKATRICE_MANA_DEVOTION_CONFIG_H

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

struct ManaDevotionConfig
{
    QString displayType; // "pie" or "bar" or "combinedBar"
    QStringList filters; // which colors to show, empty = all

    QJsonObject toJson() const;

    static ManaDevotionConfig fromJson(const QJsonObject &o);
};

#endif // COCKATRICE_MANA_DEVOTION_CONFIG_H
