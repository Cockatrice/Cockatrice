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
    QLabel *unusedColorIdentitiesOpacityLabel;
    QSpinBox *unusedColorIdentitiesOpacitySpinBox;
    QLabel *deckPreviewTooltipLabel;
    QComboBox *deckPreviewTooltipComboBox;
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

    void retranslateUi();

    [[nodiscard]] bool getShowFolders() const;
    [[nodiscard]] bool getDrawUnusedColorIdentities() const;
    [[nodiscard]] bool getShowColorIdentity() const;
    [[nodiscard]] bool getShowBannerCardComboBox() const;
    [[nodiscard]] bool getShowTagFilter() const;
    [[nodiscard]] bool getShowTagsOnDeckPreviews() const;
    [[nodiscard]] int getUnusedColorIdentitiesOpacity() const;
    [[nodiscard]] TooltipType getDeckPreviewTooltip() const;
    [[nodiscard]] int getCardSize() const;

signals:
    void showFoldersChanged(bool enabled);
    void drawUnusedColorIdentitiesChanged(bool enabled);
    void showColorIdentityChanged(bool enabled);
    void showBannerCardComboBoxChanged(bool enabled);
    void showTagFilterChanged(bool enabled);
    void showTagsOnDeckPreviewsChanged(bool enabled);
    void unusedColorIdentitiesOpacityChanged(int opacity);
    void deckPreviewTooltipChanged(TooltipType tooltip);
    void cardSizeChanged(int scale);
};

#endif // VISUAL_DECK_STORAGE_QUICK_SETTINGS_WIDGET_H