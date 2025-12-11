#include "mana_distribution_config.h"

QJsonObject ManaDistributionConfig::toJson() const
{
    QJsonObject o;
    o["displayType"] = displayType;

    QJsonArray jsonArray;
    for (auto &s : filters) {
        jsonArray.append(s);
    }
    o["filters"] = jsonArray;

    o["showColorRows"] = showColorRows;
    return o;
}

ManaDistributionConfig ManaDistributionConfig::fromJson(const QJsonObject &o)
{
    ManaDistributionConfig config;
    if (o.contains("displayType")) {
        config.displayType = o["displayType"].toString();
    }

    if (o.contains("filters")) {
        config.filters.clear();
        for (auto v : o["filters"].toArray())
            config.filters << v.toString();
    }

    if (o.contains("showColorRows")) {
        config.showColorRows = o["showColorRows"].toBool(true);
    }

    return config;
}
