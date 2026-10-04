
#ifndef COCKATRICE_MANA_BASE_CONFIG_H
#define COCKATRICE_MANA_BASE_CONFIG_H

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

struct ManaBaseConfig
{
    QString displayType; // "pie" or "bar" or "combinedBar"
    QStringList filters; // which colors to show, empty = all

    QJsonObject toJson() const;

    static ManaBaseConfig fromJson(const QJsonObject &o);
};

#endif // COCKATRICE_MANA_BASE_CONFIG_H
