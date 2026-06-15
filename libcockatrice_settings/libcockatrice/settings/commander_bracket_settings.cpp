#include "commander_bracket_settings.h"

#include <QSettings>

QVariantList CommanderBracketSettings::defaultDefinitions()
{
    return {
        QVariantMap{{"tag", "R"},
                    {"officialName", "[5] cEDH"},
                    {"displayName", "Ruthless"},
                    {"explanation",
                     "Top-tier competitive decks with maximum optimization, fast combos, and minimal variance."}},
        QVariantMap{{"tag", "S"},
                    {"officialName", "[4] Optimized"},
                    {"displayName", "Spicy"},
                    {"explanation", "Highly tuned decks with strong synergy and occasional combo finishes."}},
        QVariantMap{{"tag", "P"},
                    {"officialName", "[3] Upgraded"},
                    {"displayName", "Powerful"},
                    {"explanation", "Focused decks with clear win conditions and solid consistency."}},
        QVariantMap{{"tag", "O"},
                    {"officialName", "[2] Core"},
                    {"displayName", "Oddball"},
                    {"explanation", "Unconventional or thematic decks with some structure but non-standard choices."}},
        QVariantMap{{"tag", "PA"},
                    {"officialName", "[1] Exhibition"},
                    {"displayName", "Precon Appropriate"},
                    {"explanation", "Lightly upgraded preconstructed decks or very casual builds."}},
        QVariantMap{{"tag", "C"},
                    {"officialName", "[1] Casual"},
                    {"displayName", "Casual"},
                    {"explanation", "Relaxed decks with no strict optimization goals."}},
        QVariantMap{{"tag", "U"},
                    {"officialName", "Unknown"},
                    {"displayName", "Unknown"},
                    {"explanation", "Unclassified or missing bracket definition."}},
    };
}

CommanderBracketSettings::CommanderBracketSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "commander_brackets.ini", "commander_brackets", QString(), parent)
{
}

void CommanderBracketSettings::setSchemaVersion(int version)
{
    setValue(version, "schemaVersion");
}

int CommanderBracketSettings::getSchemaVersion() const
{
    QVariant value = getValue("schemaVersion");
    return value.isValid() ? value.toInt() : 0;
}

void CommanderBracketSettings::clearDefinitions()
{
    auto settings = getSettings();

    settings.beginGroup("commander_brackets");
    settings.remove("");
    settings.endGroup();

    settings.sync();
}

void CommanderBracketSettings::saveDefinitions(const QVariantList &definitions)
{
    auto settings = getSettings();

    settings.beginGroup("commander_brackets");

    settings.remove("");

    settings.setValue("schemaVersion", CurrentSchemaVersion);

    for (const auto &entry : definitions) {
        QVariantMap map = entry.toMap();

        QString tag = map.value("tag").toString();

        if (tag.isEmpty()) {
            continue;
        }

        settings.beginGroup(tag);

        settings.setValue("officialName", map.value("officialName"));

        settings.setValue("displayName", map.value("displayName"));

        settings.setValue("explanation", map.value("explanation"));

        settings.endGroup();
    }

    settings.endGroup();

    settings.sync();
}

QVariantList CommanderBracketSettings::loadDefinitions() const
{
    QVariantList result;

    auto settings = getSettings();

    settings.beginGroup("commander_brackets");

    int version = settings.value("schemaVersion", 0).toInt();

    if (version != CurrentSchemaVersion) {
        settings.endGroup();
        return result;
    }

    QStringList groups = settings.childGroups();

    for (const QString &tag : groups) {
        settings.beginGroup(tag);

        QVariantMap map;

        map["tag"] = tag;
        map["officialName"] = settings.value("officialName");
        map["displayName"] = settings.value("displayName");
        map["explanation"] = settings.value("explanation");

        result.append(map);

        settings.endGroup();
    }

    settings.endGroup();

    return result;
}