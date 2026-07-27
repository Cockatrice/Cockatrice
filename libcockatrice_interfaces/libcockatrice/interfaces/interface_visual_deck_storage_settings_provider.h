#ifndef COCKATRICE_INTERFACE_VISUAL_DECK_STORAGE_SETTINGS_PROVIDER_H
#define COCKATRICE_INTERFACE_VISUAL_DECK_STORAGE_SETTINGS_PROVIDER_H

#include <QStringList>

class IVisualDeckStorageSettingsProvider
{
public:
    virtual ~IVisualDeckStorageSettingsProvider() = default;

    [[nodiscard]] virtual int getVisualDeckStorageSortingOrder() const = 0;
    [[nodiscard]] virtual bool getVisualDeckStorageShowFolders() const = 0;
    [[nodiscard]] virtual bool getVisualDeckStorageShowTagFilter() const = 0;
    [[nodiscard]] virtual QStringList getVisualDeckStorageDefaultTagsList() const = 0;
    [[nodiscard]] virtual bool getVisualDeckStorageSearchFolderNames() const = 0;
    [[nodiscard]] virtual bool getVisualDeckStorageShowColorIdentity() const = 0;
    [[nodiscard]] virtual bool getVisualDeckStorageShowBannerCardComboBox() const = 0;
    [[nodiscard]] virtual bool getVisualDeckStorageShowTagsOnDeckPreviews() const = 0;
    [[nodiscard]] virtual int getVisualDeckStorageCardSize() const = 0;
    [[nodiscard]] virtual bool getVisualDeckStorageDrawUnusedColorIdentities() const = 0;
    [[nodiscard]] virtual int getVisualDeckStorageUnusedColorIdentitiesOpacity() const = 0;
    [[nodiscard]] virtual int getVisualDeckStorageTooltipType() const = 0;
    [[nodiscard]] virtual bool getVisualDeckStoragePromptForConversion() const = 0;
    [[nodiscard]] virtual bool getVisualDeckStorageAlwaysConvert() const = 0;
    [[nodiscard]] virtual bool getVisualDeckStorageInGame() const = 0;
    [[nodiscard]] virtual bool getVisualDeckStorageSelectionAnimation() const = 0;
    [[nodiscard]] virtual int getVisualDeckEditorCardSize() const = 0;
    [[nodiscard]] virtual int getVisualDeckEditorSampleHandSize() const = 0;
    [[nodiscard]] virtual int getVisualDatabaseDisplayCardSize() const = 0;
    [[nodiscard]] virtual bool getVisualDatabaseDisplayFilterToMostRecentSetsEnabled() const = 0;
    [[nodiscard]] virtual int getVisualDatabaseDisplayFilterToMostRecentSetsAmount() const = 0;
    [[nodiscard]] virtual int getEDHRecCardSize() const = 0;
    [[nodiscard]] virtual int getArchidektPreviewSize() const = 0;
    [[nodiscard]] virtual int getDefaultDeckEditorType() const = 0;
};

#endif // COCKATRICE_INTERFACE_VISUAL_DECK_STORAGE_SETTINGS_PROVIDER_H
