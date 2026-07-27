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
    [[nodiscard]] QString getClientID() override;
    [[nodiscard]] QString getClientVersion() override;
    [[nodiscard]] int getKeepAlive() const override;
    [[nodiscard]] int getTimeOut() const override;
    [[nodiscard]] bool getPicDownload() const override;
    [[nodiscard]] bool getShowStatusBar() const override;
    [[nodiscard]] int getMaxFontSize() const override;
    [[nodiscard]] QString getHighlightWords() const override;
    [[nodiscard]] QString getHomeTabBackgroundSource() const override;
    [[nodiscard]] int getHomeTabBackgroundShuffleFrequency() const override;
    [[nodiscard]] QString getThemeName() const;
    void setThemeName(const QString &_themeName);
    [[nodiscard]] bool getHomeTabDisplayCardName() const override;
    [[nodiscard]] bool getShowTipsOnStartup() const override;
    [[nodiscard]] QList<int> getSeenTips() const override;
    [[nodiscard]] bool getDownloadSpoilersStatus() const override;

    void setLang(const QString &_lang);
    void setClientID(const QString &_clientID);
    void setClientVersion(const QString &_clientVersion);
    void setPicDownload(bool _picDownload);
    void setShowStatusBar(bool value);
    void setMaxFontSize(int _max);
    void setHighlightWords(const QString &_highlightWords);
    void setHomeTabBackgroundSource(const QString &_backgroundSource);
    void setHomeTabBackgroundShuffleFrequency(int _frequency);
    void setHomeTabDisplayCardName(bool _displayCardName);
    void setShowTipsOnStartup(bool _showTipsOnStartup);
    void setSeenTips(const QList<int> &_seenTips);
    void setDownloadSpoilerStatus(bool _spoilerStatus);

signals:
    void langChanged();
    void themeNameChanged();
    void picDownloadChanged();
    void showStatusBarChanged(bool state);
    void homeTabBackgroundSourceChanged();
    void homeTabBackgroundShuffleFrequencyChanged();
    void homeTabDisplayCardNameChanged();
    void downloadSpoilerStatusChanged();

public:
    explicit PersonalSettings(const QString &settingPath, QObject *parent = nullptr);

private:
    PersonalSettings(const PersonalSettings & /*other*/);
};

#endif // PERSONAL_SETTINGS_H
