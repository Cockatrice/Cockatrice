#ifndef COCKATRICE_INTERFACE_DECK_EDITOR_SETTINGS_PROVIDER_H
#define COCKATRICE_INTERFACE_DECK_EDITOR_SETTINGS_PROVIDER_H

class IDeckEditorSettingsProvider
{
public:
    virtual ~IDeckEditorSettingsProvider() = default;

    [[nodiscard]] virtual bool getOpenDeckInNewTab() const = 0;
    [[nodiscard]] virtual bool getBannerCardComboBoxVisible() const = 0;
    [[nodiscard]] virtual bool getTagsWidgetVisible() const = 0;
    [[nodiscard]] virtual int getDefaultDeckEditorType() const = 0;
};

#endif // COCKATRICE_INTERFACE_DECK_EDITOR_SETTINGS_PROVIDER_H
