#include "draw_probability_config.h"

QJsonObject DrawProbabilityConfig::toJson() const
{
    QJsonObject o;
    o["criteria"] = criteria;
    o["atLeast"] = atLeast;
    o["quantity"] = quantity;
    o["drawn"] = drawn;
    return o;
}
DrawProbabilityConfig DrawProbabilityConfig::fromJson(const QJsonObject &o)
{
    DrawProbabilityConfig cfg;
    if (o.contains("criteria")) {
        cfg.criteria = o["criteria"].toString();
    }
    if (o.contains("atLeast")) {
        cfg.atLeast = o["atLeast"].toBool(true);
    }
    if (o.contains("quantity")) {
        cfg.quantity = o["quantity"].toInt(1);
    }
    if (o.contains("drawn")) {
        cfg.drawn = o["drawn"].toInt(7);
    }
    return cfg;
}
