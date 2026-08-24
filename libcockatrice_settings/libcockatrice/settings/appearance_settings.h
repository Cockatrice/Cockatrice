/**
 * @file appearance_settings.h
 * @ingroup CoreSettings
 */
//! \todo Document this file.

#ifndef APPEARANCE_SETTINGS_H
#define APPEARANCE_SETTINGS_H

#include "settings_manager.h"

class AppearanceSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

public:
    [[nodiscard]] QString getThemeName() const;
    void setThemeName(const QString &_themeName);
    [[nodiscard]] bool getStyleUserList() const;
    void setStyleUserList(bool _styleUserList);
    [[nodiscard]] int getMaxFontSize() const;
    void setMaxFontSize(int _max);
    [[nodiscard]] QString getHomeTabBackgroundSource() const;
    void setHomeTabBackgroundSource(const QString &_backgroundSource);
    [[nodiscard]] int getHomeTabBackgroundShuffleFrequency() const;
    void setHomeTabBackgroundShuffleFrequency(int _frequency);
    [[nodiscard]] bool getHomeTabDisplayCardName() const;
    void setHomeTabDisplayCardName(bool _displayCardName);
    [[nodiscard]] int getHomeTabButtonColorSourceIndex() const;
    void setHomeTabButtonColorSourceIndex(int index);

signals:
    void themeNameChanged();
    void styleUserListChanged();
    void homeTabBackgroundSourceChanged();
    void homeTabBackgroundShuffleFrequencyChanged();
    void homeTabDisplayCardNameChanged();
    void homeTabButtonColorChanged();

public:
    explicit AppearanceSettings(const QString &settingPath, QObject *parent = nullptr);

private:
    AppearanceSettings(const AppearanceSettings & /*other*/);
};

#endif // APPEARANCE_SETTINGS_H
