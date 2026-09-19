#ifndef COMMANDER_BRACKET_SETTINGS_H
#define COMMANDER_BRACKET_SETTINGS_H

#include "settings_manager.h"

#include <QHash>
#include <QObject>
#include <QString>
#include <QVariantList>

struct CommanderBracketDefinition
{
    QString tag;

    QString officialName;
    QString displayName;

    QString explanation;
};

class CommanderBracketSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

public:
    static constexpr int CurrentSchemaVersion = 1;

    static QVariantList defaultDefinitions();

    void clearDefinitions();

    void saveDefinitions(const QVariantList &definitions);

    QVariantList loadDefinitions() const;

    void reloadDefinitions(const QVariantList &definitions);

    QString officialName(const QString &tag) const;
    QString displayName(const QString &tag) const;
    QString explanation(const QString &tag) const;
    bool contains(const QString &tag) const;

    void setSchemaVersion(int version);
    int getSchemaVersion() const;

private:
    explicit CommanderBracketSettings(const QString &settingPath, QObject *parent = nullptr);

    CommanderBracketSettings(const CommanderBracketSettings &) = delete;
    CommanderBracketSettings &operator=(const CommanderBracketSettings &) = delete;

    QHash<QString, CommanderBracketDefinition> definitions;
};

#endif // COMMANDER_BRACKET_SETTINGS_H
