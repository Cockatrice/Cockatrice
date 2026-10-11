#include "mana_base_config.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QJsonValueRef>

QJsonObject ManaBaseConfig::toJson() const
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

ManaBaseConfig ManaBaseConfig::fromJson(const QJsonObject &o)

{
    ManaBaseConfig config;

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
