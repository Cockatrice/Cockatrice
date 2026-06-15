#ifndef COMMANDER_BRACKET_SETTINGS_H
#define COMMANDER_BRACKET_SETTINGS_H

#include "settings_manager.h"

#include <QObject>
#include <QVariantList>

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

    void setSchemaVersion(int version);
    int getSchemaVersion() const;

private:
    explicit CommanderBracketSettings(const QString &settingPath, QObject *parent = nullptr);

    CommanderBracketSettings(const CommanderBracketSettings &) = delete;
    CommanderBracketSettings &operator=(const CommanderBracketSettings &) = delete;
};

#endif // COMMANDER_BRACKET_SETTINGS_H