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
        QVariantMap{{"tag", "C"},
                    {"officialName", "[2] Core"},
                    {"displayName", "Core"},
                    {"explanation", "Preconstructed or precon-level decks with straightforward strategies."}},
        QVariantMap{{"tag", "E"},
                    {"officialName", "[1] Exhibition"},
                    {"displayName", "Exhibition"},
                    {"explanation", "Ultra-casual, theme-focused decks with minimal optimization."}},
        QVariantMap{{"tag", "B"},
                    {"officialName", "Banned"},
                    {"displayName", "Banned"},
                    {"explanation", "The deck contains one or more cards banned in Commander."}},
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

void CommanderBracketSettings::reloadDefinitions(const QVariantList &definitionsList)
{
    definitions.clear();

    for (const auto &entry : definitionsList) {
        const auto map = entry.toMap();

        CommanderBracketDefinition definition;

        definition.tag = map.value("tag").toString();
        definition.officialName = map.value("officialName").toString();
        definition.displayName = map.value("displayName").toString();
        definition.explanation = map.value("explanation").toString();

        if (!definition.tag.isEmpty()) {
            definitions.insert(definition.tag, definition);
        }
    }
}

QString CommanderBracketSettings::officialName(const QString &tag) const
{
    auto it = definitions.find(tag);

    if (it == definitions.end()) {
        return tag;
    }

    return it->officialName;
}

QString CommanderBracketSettings::displayName(const QString &tag) const
{
    auto it = definitions.find(tag);

    if (it == definitions.end()) {
        return tag;
    }

    return it->displayName;
}

QString CommanderBracketSettings::explanation(const QString &tag) const
{
    auto it = definitions.find(tag);

    if (it == definitions.end()) {
        return {};
    }

    return it->explanation;
}

bool CommanderBracketSettings::contains(const QString &tag) const
{
    return definitions.contains(tag);
}