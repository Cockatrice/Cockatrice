#include "mana_curve_config.h"

QJsonObject ManaCurveConfig::toJson() const
{
    QJsonObject jsonObject;
    jsonObject["groupBy"] = groupBy;
    QJsonArray jsonArray;
    for (auto &filter : filters) {
        jsonArray.append(filter);
    }
    jsonObject["filters"] = jsonArray;
    jsonObject["showMain"] = showMain;
    jsonObject["showCategoryRows"] = showCategoryRows;
    return jsonObject;
}

ManaCurveConfig ManaCurveConfig::fromJson(const QJsonObject &o)
{
    ManaCurveConfig config;

    if (o.contains("groupBy")) {
        config.groupBy = o["groupBy"].toString();
    }

    if (o.contains("filters")) {
        config.filters.clear();
        for (auto v : o["filters"].toArray()) {
            config.filters << v.toString();
        }
    }

    if (o.contains("showMain")) {
        config.showMain = o["showMain"].toBool(true);
    }

    if (o.contains("showCategoryRows")) {
        config.showCategoryRows = o["showCategoryRows"].toBool(true);
    }

    return config;
}
