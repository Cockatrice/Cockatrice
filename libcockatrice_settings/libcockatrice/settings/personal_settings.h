#ifndef PERSONAL_SETTINGS_H
#define PERSONAL_SETTINGS_H

#include "settings_manager.h"

#include <QDate>
#include <QList>
#include <libcockatrice/interfaces/interface_personal_settings_provider.h>

class PersonalSettings : public SettingsManager, public IPersonalSettingsProvider
{
    Q_OBJECT
    friend class SettingsCache;

public:
    [[nodiscard]] QString getLang() const override;
    [[nodiscard]] bool getShowTipsOnStartup() const override;
    [[nodiscard]] QList<int> getSeenTips() const override;

    void setLang(const QString &_lang);
    void setShowTipsOnStartup(bool _showTipsOnStartup);
    void setSeenTips(const QList<int> &_seenTips);

signals:
    void langChanged();

public:
    explicit PersonalSettings(const QString &settingPath, QObject *parent = nullptr);

private:
    PersonalSettings(const PersonalSettings & /*other*/);
};

#endif // PERSONAL_SETTINGS_H
