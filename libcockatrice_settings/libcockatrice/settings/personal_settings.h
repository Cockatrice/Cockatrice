#ifndef PERSONAL_SETTINGS_H
#define PERSONAL_SETTINGS_H

#include "settings_manager.h"

#include <QString>
#include <libcockatrice/interfaces/interface_personal_settings_provider.h>
#include <qtmetamacros.h>

class QObject;
template <typename T> class QList;

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

    enum class CrashReportingMode
    {
        Ask = 0,        ///< Show the crash-report dialog after an unclean exit.
        AlwaysSend = 1, ///< Upload crash reports without asking.
        NeverSend = 2   ///< Never upload; stay silent.
    };

    [[nodiscard]] CrashReportingMode getCrashReportingMode() const;
    void setCrashReportingMode(CrashReportingMode _crashReportingMode);

signals:
    void langChanged();
    void crashReportingModeChanged(CrashReportingMode mode);

public:
    explicit PersonalSettings(const QString &settingPath, QObject *parent = nullptr);

private:
    PersonalSettings(const PersonalSettings & /*other*/);
};

#endif // PERSONAL_SETTINGS_H
