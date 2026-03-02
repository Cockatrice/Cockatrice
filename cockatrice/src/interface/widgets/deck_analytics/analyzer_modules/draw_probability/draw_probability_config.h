#ifndef COCKATRICE_DRAW_PROBABILITY_CONFIG_H
#define COCKATRICE_DRAW_PROBABILITY_CONFIG_H

#include <QJsonObject>
#include <QString>

struct DrawProbabilityConfig
{
    QString criteria = "name"; // name, type, subtype, cmc
    bool atLeast = true;       // true = at least, false = exactly
    int quantity = 1;          // N
    int drawn = 7;             // M

    QJsonObject toJson() const;

    static DrawProbabilityConfig fromJson(const QJsonObject &o);
};

#endif
