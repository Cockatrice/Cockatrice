#include "mana_devotion_config.h"

QJsonObject ManaDevotionConfig::toJson() const
{
    QJsonObject jsonObject;
    QJsonArray jsonArray;
    jsonObject["displayType"] = displayType;
    for (auto &filter : filters) {
        jsonArray.append(filter);
    }
    jsonObject["filters"] = jsonArray;
    return jsonObject;
}

ManaDevotionConfig ManaDevotionConfig::fromJson(const QJsonObject &o)
{
    ManaDevotionConfig config;

    if (o.contains("displayType")) {
        config.displayType = o["displayType"].toString();
    }

    if (o.contains("filters")) {
        config.filters.clear();
        for (auto v : o["filters"].toArray()) {
            config.filters << v.toString();
        }
    }

    return config;
}
