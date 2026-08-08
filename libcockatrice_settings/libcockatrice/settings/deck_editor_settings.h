#ifndef DECK_EDITOR_SETTINGS_H
#define DECK_EDITOR_SETTINGS_H

#include "settings_manager.h"

#include <libcockatrice/interfaces/interface_deck_editor_settings_provider.h>

class DeckEditorSettings : public SettingsManager, public IDeckEditorSettingsProvider
{
    Q_OBJECT
    friend class SettingsCache;

public:
    [[nodiscard]] bool getOpenDeckInNewTab() const override;
    [[nodiscard]] bool getBannerCardComboBoxVisible() const override;
    [[nodiscard]] bool getTagsWidgetVisible() const override;
    [[nodiscard]] int getDefaultDeckEditorType() const override;

    void setOpenDeckInNewTab(bool _openDeckInNewTab);
    void setBannerCardComboBoxVisible(bool _bannerCardComboBoxVisible);
    void setTagsWidgetVisible(bool _tagsWidgetVisible);
    void setDefaultDeckEditorType(int _defaultDeckEditorType);

signals:
    void bannerCardComboBoxVisibleChanged(bool visible);
    void tagsWidgetVisibleChanged(bool visible);

public:
    explicit DeckEditorSettings(const QString &settingPath, QObject *parent = nullptr);

private:
    DeckEditorSettings(const DeckEditorSettings & /*other*/);
};

#endif // DECK_EDITOR_SETTINGS_H
