/**
 * @file visual_deck_storage_quick_settings_widget.h
 * @ingroup VisualDeckStorageWidgets
 * @brief The VDS's quick settings menu.
 * Manages the widgets in the quick settings menu dropdown, as well as syncing their values with SettingsCache.
 * The current values of the settings are exposed through getters and signals.
 */

#ifndef VISUAL_DECK_STORAGE_QUICK_SETTINGS_WIDGET_H
#define VISUAL_DECK_STORAGE_QUICK_SETTINGS_WIDGET_H

#include "../quick_settings/settings_button_widget.h"

class CardSizeWidget;
class QLabel;
class QSpinBox;
class QCheckBox;
class QComboBox;

class VisualDeckStorageQuickSettingsWidget : public SettingsButtonWidget
{
    Q_OBJECT

    QCheckBox *showFoldersCheckBox;
    QCheckBox *showColorIdentityCheckBox;
    QCheckBox *drawUnusedColorIdentitiesCheckBox;
    QCheckBox *showBannerCardComboBoxCheckBox;
    QCheckBox *showTagFilterCheckBox;
    QCheckBox *showTagsOnDeckPreviewsCheckBox;
    QCheckBox *showUploadTimeCheckBox;
    QLabel *unusedColorIdentitiesOpacityLabel;
    QSpinBox *unusedColorIdentitiesOpacitySpinBox;
    QLabel *deckPreviewTooltipLabel;
    QComboBox *deckPreviewTooltipComboBox;
    QWidget *deckPreviewTooltipWidget;
    CardSizeWidget *cardSizeWidget;

public:
    /**
     * The info to display in the deck preview's banner card tooltip.
     */
    enum TooltipType
    {
        None,
        Filepath
    };
    Q_ENUM(TooltipType)

    explicit VisualDeckStorageQuickSettingsWidget(QWidget *parent = nullptr);

    /**
     * @brief Hides the controls that do not apply to the public decks tab.
     *
     * The public decks tab reuses this widget for its quick settings menu but
     * has no folders, banner selection or per-deck tooltip, so those controls
     * are hidden while every shared key keeps syncing with SettingsCache.
     */
    void setPublicDecksMode(bool enabled);

    void retranslateUi();

    [[nodiscard]] bool getShowFolders() const;
    [[nodiscard]] bool getDrawUnusedColorIdentities() const;
    [[nodiscard]] bool getShowColorIdentity() const;
    [[nodiscard]] bool getShowBannerCardComboBox() const;
    [[nodiscard]] bool getShowTagFilter() const;
    [[nodiscard]] bool getShowTagsOnDeckPreviews() const;
    [[nodiscard]] bool getShowUploadTime() const;
    [[nodiscard]] int getUnusedColorIdentitiesOpacity() const;
    [[nodiscard]] TooltipType getDeckPreviewTooltip() const;
    [[nodiscard]] int getCardSize() const;

    /**
     * @return The card size widget, so card display hosts can resize the cards on
     * Ctrl + scroll even though the slider itself lives inside this menu.
     */
    [[nodiscard]] CardSizeWidget *getCardSizeWidget() const
    {
        return cardSizeWidget;
    }

signals:
    void showFoldersChanged(bool enabled);
    void drawUnusedColorIdentitiesChanged(bool enabled);
    void showColorIdentityChanged(bool enabled);
    void showBannerCardComboBoxChanged(bool enabled);
    void showTagFilterChanged(bool enabled);
    void showTagsOnDeckPreviewsChanged(bool enabled);
    void showUploadTimeChanged(bool enabled);
    void unusedColorIdentitiesOpacityChanged(int opacity);
    void deckPreviewTooltipChanged(TooltipType tooltip);
    void cardSizeChanged(int scale);
};

#endif // VISUAL_DECK_STORAGE_QUICK_SETTINGS_WIDGET_H