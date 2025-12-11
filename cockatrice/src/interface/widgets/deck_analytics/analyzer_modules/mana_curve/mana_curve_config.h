
#ifndef COCKATRICE_MANA_CURVE_CONFIG_H
#define COCKATRICE_MANA_CURVE_CONFIG_H

#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

struct ManaCurveConfig
{
    QString groupBy = "type"; // "type", "color", "subtype", etc.
    QStringList filters;      // empty = all
    bool showMain = true;
    bool showCategoryRows = true;

    QJsonObject toJson() const;

    static ManaCurveConfig fromJson(const QJsonObject &o);
};

#endif // COCKATRICE_MANA_CURVE_CONFIG_H
