#include "draw_probability_config.h"

#include <QJsonValue>
#include <QJsonValueRef>
#include <algorithm>

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
        const QString criteria = o["criteria"].toString();
        if (criteria == "name" || criteria == "type" || criteria == "subtype" || criteria == "cmc") {
            cfg.criteria = criteria;
        }
    }
    if (o.contains("atLeast")) {
        cfg.atLeast = o["atLeast"].toBool(true);
    }
    if (o.contains("quantity")) {
        cfg.quantity = std::clamp(o["quantity"].toInt(1), 0, 60);
    }
    if (o.contains("drawn")) {
        cfg.drawn = std::clamp(o["drawn"].toInt(7), 0, 60);
    }
    return cfg;
}
