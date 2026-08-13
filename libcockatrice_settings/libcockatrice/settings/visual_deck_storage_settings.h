#ifndef VISUAL_DECK_STORAGE_SETTINGS_H
#define VISUAL_DECK_STORAGE_SETTINGS_H

#include "settings_manager.h"

#include <QStringList>
#include <libcockatrice/interfaces/interface_visual_deck_storage_settings_provider.h>

class VisualDeckStorageSettings : public SettingsManager, public IVisualDeckStorageSettingsProvider
{
    Q_OBJECT
    friend class SettingsCache;

public:
    [[nodiscard]] int getVisualDeckStorageSortingOrder() const override;
    [[nodiscard]] bool getVisualDeckStorageShowFolders() const override;
    [[nodiscard]] bool getVisualDeckStorageShowTagFilter() const override;
    [[nodiscard]] QStringList getVisualDeckStorageDefaultTagsList() const override;
    [[nodiscard]] bool getVisualDeckStorageSearchFolderNames() const override;
    [[nodiscard]] bool getVisualDeckStorageShowColorIdentity() const override;
    [[nodiscard]] bool getVisualDeckStorageShowBannerCardComboBox() const override;
    [[nodiscard]] bool getVisualDeckStorageShowTagsOnDeckPreviews() const override;
    [[nodiscard]] bool getVisualDeckStorageDrawUnusedColorIdentities() const override;
    [[nodiscard]] int getVisualDeckStorageUnusedColorIdentitiesOpacity() const override;
    [[nodiscard]] int getVisualDeckStorageTooltipType() const override;
    [[nodiscard]] bool getVisualDeckStoragePromptForConversion() const override;
    [[nodiscard]] bool getVisualDeckStorageAlwaysConvert() const override;
    [[nodiscard]] bool getVisualDeckStorageInGame() const override;
    [[nodiscard]] bool getVisualDeckStorageSelectionAnimation() const override;
    [[nodiscard]] bool getVisualDatabaseDisplayFilterToMostRecentSetsEnabled() const override;
    [[nodiscard]] int getVisualDatabaseDisplayFilterToMostRecentSetsAmount() const override;

    void setVisualDeckStorageSortingOrder(int _sortingOrder);
    void setVisualDeckStorageShowFolders(bool value);
    void setVisualDeckStorageShowTagFilter(bool _showTags);
    void setVisualDeckStorageDefaultTagsList(QStringList _defaultTagsList);
    void setVisualDeckStorageSearchFolderNames(bool value);
    void setVisualDeckStorageShowColorIdentity(bool value);
    void setVisualDeckStorageShowBannerCardComboBox(bool _showBannerCardComboBox);
    void setVisualDeckStorageShowTagsOnDeckPreviews(bool _showTags);
    void setVisualDeckStorageDrawUnusedColorIdentities(bool _draw);
    void setVisualDeckStorageUnusedColorIdentitiesOpacity(int _opacity);
    void setVisualDeckStorageTooltipType(int value);
    void setVisualDeckStoragePromptForConversion(bool _prompt);
    void setVisualDeckStorageAlwaysConvert(bool _always);
    void setVisualDeckStorageInGame(bool enabled);
    void setVisualDeckStorageSelectionAnimation(bool enabled);
    void setVisualDatabaseDisplayFilterToMostRecentSetsEnabled(bool _enabled);
    void setVisualDatabaseDisplayFilterToMostRecentSetsAmount(int _amount);

signals:
    void visualDeckStorageShowTagFilterChanged(bool _visible);
    void visualDeckStorageDefaultTagsListChanged();
    void visualDeckStorageShowColorIdentityChanged(bool _visible);
    void visualDeckStorageShowBannerCardComboBoxChanged(bool _visible);
    void visualDeckStorageShowTagsOnDeckPreviewsChanged(bool _visible);
    void visualDeckStorageDrawUnusedColorIdentitiesChanged(bool _visible);
    void visualDeckStorageUnusedColorIdentitiesOpacityChanged(bool value);
    void visualDeckStorageInGameChanged(bool enabled);
    void visualDeckStorageSelectionAnimationChanged(bool enabled);
    void visualDatabaseDisplayFilterToMostRecentSetsEnabledChanged(bool enabled);
    void visualDatabaseDisplayFilterToMostRecentSetsAmountChanged(int amount);

public:
    explicit VisualDeckStorageSettings(const QString &settingPath, QObject *parent = nullptr);

private:
    VisualDeckStorageSettings(const VisualDeckStorageSettings & /*other*/);
};

#endif // VISUAL_DECK_STORAGE_SETTINGS_H
