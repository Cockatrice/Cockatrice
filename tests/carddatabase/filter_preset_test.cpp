#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <gtest/gtest.h>
#include <libcockatrice/filters/filter_preset.h>

namespace
{
FilterPreset mixedPreset()
{
    return {{{CardFilter::AttrTag,
              false,
              {{CardFilter::TypeAnd, true, {{"favorite", false}, {QString::fromUtf8("Éowyn \"tag\"\n"), true}}},
               {CardFilter::TypeOrNot, false, {}}}},
             {CardFilter::AttrColorIdentity, true, {{CardFilter::TypeAndNot, false, {{"W", true}}}}},
             {CardFilter::AttrName, false, {}}},
            "identity"};
}

void expectSameSnapshot(const FilterPreset::Snapshot &actual, const FilterPreset::Snapshot &expected)
{
    ASSERT_EQ(actual.size(), expected.size());
    for (qsizetype a = 0; a < expected.size(); ++a) {
        EXPECT_EQ(actual[a].attr, expected[a].attr);
        EXPECT_EQ(actual[a].enabled, expected[a].enabled);
        ASSERT_EQ(actual[a].types.size(), expected[a].types.size());
        for (qsizetype t = 0; t < expected[a].types.size(); ++t) {
            const auto &actualType = actual[a].types[t];
            const auto &expectedType = expected[a].types[t];
            EXPECT_EQ(actualType.type, expectedType.type);
            EXPECT_EQ(actualType.enabled, expectedType.enabled);
            ASSERT_EQ(actualType.terms.size(), expectedType.terms.size());
            for (qsizetype i = 0; i < expectedType.terms.size(); ++i) {
                EXPECT_EQ(actualType.terms[i].term, expectedType.terms[i].term);
                EXPECT_EQ(actualType.terms[i].enabled, expectedType.terms[i].enabled);
            }
        }
    }
}

TEST(FilterPresetSerialization, RoundTripPreservesIndependentFlagsEmptyGroupsAndColorMode)
{
    const auto original = mixedPreset();
    const QByteArray bytes = QJsonDocument(original.toJson()).toJson();
    const auto restored = FilterPreset::fromJson(QJsonDocument::fromJson(bytes).object());
    ASSERT_TRUE(restored);
    expectSameSnapshot(restored->snapshot, original.snapshot);
    EXPECT_EQ(restored->colorMode, original.colorMode);
}

TEST(FilterPresetSerialization, EmptyPresetAndEveryColorModeRoundTrip)
{
    for (const QString &mode : {QString(), QString("exact"), QString("includes"), QString("excludes"),
                                QString("include_exclude"), QString("identity")}) {
        const FilterPreset original{{}, mode};
        const auto restored = FilterPreset::fromJson(original.toJson());
        ASSERT_TRUE(restored);
        EXPECT_TRUE(restored->snapshot.isEmpty());
        EXPECT_EQ(restored->colorMode, mode);
    }
}

TEST(FilterPresetSerialization, LegacyLabelsDefaultToEnabledAndKeepExplicitDisabledTerms)
{
    const QJsonObject legacy{
        {"filters",
         QJsonArray{QJsonObject{{"term", "favorite"}, {"type", "AND"}, {"attr", "Tag"}},
                    QJsonObject{{"term", "U"}, {"type", "AND NOT"}, {"attr", "Color Identity"}, {"enabled", false}}}}};
    const auto restored = FilterPreset::fromJson(legacy);
    ASSERT_TRUE(restored);
    const FilterPreset::Snapshot expected{
        {CardFilter::AttrTag, true, {{CardFilter::TypeAnd, true, {{"favorite", true}}}}},
        {CardFilter::AttrColorIdentity, true, {{CardFilter::TypeAndNot, true, {{"U", false}}}}}};
    expectSameSnapshot(restored->snapshot, expected);
    EXPECT_TRUE(restored->colorMode.isEmpty());
}

TEST(FilterPresetSerialization, CanonicalIdsDoNotRequireDisplayLabels)
{
    QJsonObject document = mixedPreset().toJson();
    QJsonArray filters;
    for (const auto &value : document["filters"].toArray()) {
        QJsonObject filter = value.toObject();
        filter.remove("type");
        filter.remove("attr");
        filters.append(filter);
    }
    document["filters"] = filters;
    const auto restored = FilterPreset::fromJson(document);
    ASSERT_TRUE(restored);
    expectSameSnapshot(restored->snapshot, mixedPreset().snapshot);
}

TEST(FilterPresetSerialization, RejectsMalformedRootAndVersion)
{
    EXPECT_FALSE(FilterPreset::fromJson(QJsonObject()));
    for (const QJsonValue &value : {QJsonValue(), QJsonValue(1), QJsonValue(true), QJsonValue("filters")}) {
        auto document = mixedPreset().toJson();
        document["filters"] = value;
        EXPECT_FALSE(FilterPreset::fromJson(document));
    }
    for (const QJsonValue &version :
         {QJsonValue(), QJsonValue("1"), QJsonValue(true), QJsonValue(1.5), QJsonValue(2)}) {
        auto document = mixedPreset().toJson();
        document["filterStateVersion"] = version;
        EXPECT_FALSE(FilterPreset::fromJson(document));
    }
    for (const QString &key : {QString("filterStateVersion"), QString("filterGroups")}) {
        auto document = mixedPreset().toJson();
        document.remove(key);
        EXPECT_FALSE(FilterPreset::fromJson(document));
    }
}

TEST(FilterPresetSerialization, RejectsUnknownAndDuplicateGroups)
{
    const auto original = mixedPreset().toJson();
    for (const QJsonValue &invalid : {QJsonValue(), QJsonValue("future-attribute"), QJsonValue(1)}) {
        auto document = original;
        auto groups = document["filterGroups"].toArray();
        auto group = groups[0].toObject();
        group["attrId"] = invalid;
        groups[0] = group;
        document["filterGroups"] = groups;
        EXPECT_FALSE(FilterPreset::fromJson(document));
    }
    auto document = original;
    auto groups = document["filterGroups"].toArray();
    groups.append(groups[0]);
    document["filterGroups"] = groups;
    EXPECT_FALSE(FilterPreset::fromJson(document));

    document = original;
    groups = document["filterGroups"].toArray();
    auto group = groups[0].toObject();
    auto types = group["types"].toArray();
    types.append(types[0]);
    group["types"] = types;
    groups[0] = group;
    document["filterGroups"] = groups;
    EXPECT_FALSE(FilterPreset::fromJson(document));
}

TEST(FilterPresetSerialization, RejectsMalformedGroupAndTermState)
{
    for (const QString &key : {QString("enabled"), QString("types")}) {
        auto document = mixedPreset().toJson();
        auto groups = document["filterGroups"].toArray();
        auto group = groups[0].toObject();
        group.remove(key);
        groups[0] = group;
        document["filterGroups"] = groups;
        EXPECT_FALSE(FilterPreset::fromJson(document));
    }
    for (const QString &key : {QString("typeId"), QString("attrId"), QString("term"), QString("enabled")}) {
        auto document = mixedPreset().toJson();
        auto filters = document["filters"].toArray();
        auto filter = filters[0].toObject();
        filter[key] = 42;
        filters[0] = filter;
        document["filters"] = filters;
        EXPECT_FALSE(FilterPreset::fromJson(document));
    }
}

TEST(FilterPresetSerialization, RejectsTermsOutsideDeclaredGroups)
{
    for (const QString &key : {QString("typeId"), QString("attrId")}) {
        auto document = mixedPreset().toJson();
        auto filters = document["filters"].toArray();
        auto filter = filters[0].toObject();
        filter[key] = key == QLatin1String("typeId") ? "or" : "color";
        filters[0] = filter;
        document["filters"] = filters;
        EXPECT_FALSE(FilterPreset::fromJson(document));
    }
}

TEST(FilterPresetSerialization, DuplicateTermsMustAgreeOnEnabledState)
{
    auto document = mixedPreset().toJson();
    auto filters = document["filters"].toArray();
    filters.append(filters[0]);
    document["filters"] = filters;
    auto restored = FilterPreset::fromJson(document);
    ASSERT_TRUE(restored);
    expectSameSnapshot(restored->snapshot, mixedPreset().snapshot);

    auto duplicate = filters[0].toObject();
    duplicate["enabled"] = !duplicate["enabled"].toBool();
    filters.append(duplicate);
    document["filters"] = filters;
    EXPECT_FALSE(FilterPreset::fromJson(document));
}

TEST(FilterPresetSerialization, RejectsInvalidColorModeBeforeAcceptingFilters)
{
    for (const QJsonValue &mode : {QJsonValue(), QJsonValue(1), QJsonValue(true), QJsonValue("future-mode")}) {
        auto document = mixedPreset().toJson();
        document["colorMode"] = mode;
        EXPECT_FALSE(FilterPreset::fromJson(document));
    }
}
} // namespace

int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
