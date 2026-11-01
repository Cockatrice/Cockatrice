#include "filter_preset.h"

#include <QJsonArray>
#include <QMap>
#include <memory>
#include <utility>

QJsonObject FilterPreset::toJson() const
{
    QJsonArray filters;
    QJsonArray groups;
    for (const auto &attribute : snapshot) {
        const CardFilter attrIdentity(QString(), CardFilter::TypeAnd, attribute.attr);
        QJsonArray types;
        for (const auto &group : attribute.types) {
            const CardFilter typeIdentity(QString(), group.type, attribute.attr);
            types.append(QJsonObject{{"typeId", typeIdentity.toJson()["typeId"]}, {"enabled", group.enabled}});
            for (const auto &term : group.terms) {
                const CardFilter filter(term.term, group.type, attribute.attr);
                QJsonObject object = filter.toJson();
                object["enabled"] = term.enabled;
                filters.append(object);
            }
        }
        groups.append(
            QJsonObject{{"attrId", attrIdentity.toJson()["attrId"]}, {"enabled", attribute.enabled}, {"types", types}});
    }
    // Keep the original flat array and display fields for legacy readers. Group
    // metadata also represents empty groups, which have no entry in that array.
    QJsonObject root{{"filters", filters}, {"filterStateVersion", 1}, {"filterGroups", groups}};
    if (!colorMode.isEmpty()) {
        root["colorMode"] = colorMode;
    }
    return root;
}

std::optional<FilterPreset> FilterPreset::fromJson(const QJsonObject &root)
{
    if (!root.value("filters").isArray()) {
        return std::nullopt;
    }
    FilterPreset preset;
    if (root.contains("colorMode")) {
        const QJsonValue mode = root.value("colorMode");
        if (!mode.isString()) {
            return std::nullopt;
        }
        preset.colorMode = mode.toString();
        if (!preset.colorMode.isEmpty() && preset.colorMode != QLatin1String("exact") &&
            preset.colorMode != QLatin1String("includes") && preset.colorMode != QLatin1String("excludes") &&
            preset.colorMode != QLatin1String("include_exclude") && preset.colorMode != QLatin1String("identity")) {
            return std::nullopt;
        }
    }
    auto &snapshot = preset.snapshot;
    QMap<CardFilter::Attr, qsizetype> attributes;
    QMap<QPair<CardFilter::Attr, CardFilter::Type>, qsizetype> types;
    const bool hasGroups = root.contains("filterGroups") || root.contains("filterStateVersion");
    if (hasGroups) {
        if (!root.value("filterStateVersion").isDouble() || root.value("filterStateVersion").toDouble() != 1 ||
            !root.value("filterGroups").isArray()) {
            return std::nullopt;
        }
        for (const auto &value : root.value("filterGroups").toArray()) {
            const QJsonObject group = value.toObject();
            if (!value.isObject() || !group.value("enabled").isBool() || !group.value("types").isArray()) {
                return std::nullopt;
            }
            std::unique_ptr<CardFilter> identity(
                CardFilter::fromJson(QJsonObject{{"term", ""}, {"typeId", "and"}, {"attrId", group.value("attrId")}}));
            if (!identity || attributes.contains(identity->attr())) {
                return std::nullopt;
            }
            const auto attr = identity->attr();
            attributes.insert(attr, snapshot.size());
            AttributeSnapshot attribute{attr, group.value("enabled").toBool(), {}};
            for (const auto &typeValue : group.value("types").toArray()) {
                const QJsonObject type = typeValue.toObject();
                if (!typeValue.isObject() || !type.value("enabled").isBool()) {
                    return std::nullopt;
                }
                identity.reset(CardFilter::fromJson(
                    QJsonObject{{"term", ""}, {"typeId", type.value("typeId")}, {"attrId", group.value("attrId")}}));
                if (!identity) {
                    return std::nullopt;
                }
                const auto key = qMakePair(attr, identity->type());
                if (types.contains(key)) {
                    return std::nullopt;
                }
                types.insert(key, attribute.types.size());
                attribute.types.append({identity->type(), type.value("enabled").toBool(), {}});
            }
            snapshot.append(std::move(attribute));
        }
    }

    for (const auto &value : root.value("filters").toArray()) {
        const QJsonObject object = value.toObject();
        if (!value.isObject() || ((hasGroups || object.contains("enabled")) && !object.value("enabled").isBool())) {
            return std::nullopt;
        }
        std::unique_ptr<CardFilter> filter(CardFilter::fromJson(object));
        if (!filter) {
            return std::nullopt;
        }
        const auto attr = filter->attr();
        const auto key = qMakePair(attr, filter->type());
        if (hasGroups && (!attributes.contains(attr) || !types.contains(key))) {
            return std::nullopt;
        }
        // Files predating enabled-state metadata retain their all-enabled behavior.
        if (!attributes.contains(attr)) {
            attributes.insert(attr, snapshot.size());
            snapshot.append({attr, true, {}});
        }
        auto &attribute = snapshot[attributes.value(attr)];
        if (!types.contains(key)) {
            types.insert(key, attribute.types.size());
            attribute.types.append({filter->type(), true, {}});
        }
        auto &terms = attribute.types[types.value(key)].terms;
        const bool enabled = object.value("enabled").toBool(true);
        bool duplicate = false;
        for (const auto &term : terms) {
            if (term.term == filter->term()) {
                if (term.enabled != enabled) {
                    return std::nullopt;
                }
                duplicate = true;
                break;
            }
        }
        if (!duplicate) {
            terms.append({filter->term(), enabled});
        }
    }
    return preset;
}
