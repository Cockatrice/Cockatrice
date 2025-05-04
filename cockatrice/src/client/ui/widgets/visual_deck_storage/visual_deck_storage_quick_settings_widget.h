#ifndef VISUAL_DECK_STORAGE_QUICK_SETTINGS_WIDGET_H
#define VISUAL_DECK_STORAGE_QUICK_SETTINGS_WIDGET_H

#include "../quick_settings/settings_button_widget.h"

class CardSizeWidget;
class QLabel;
class QSpinBox;
class QCheckBox;

/**
 * The VDS's quick settings menu.
 * Manages the widgets in the quick settings menu dropdown, as well as syncing their values with SettingsCache.
 * The current values of the settings are exposed through getters and signals.
 */
class VisualDeckStorageQuickSettingsWidget : public SettingsButtonWidget
{
    Q_OBJECT

    QCheckBox *showFoldersCheckBox;
    QCheckBox *drawUnusedColorIdentitiesCheckBox;
    QCheckBox *showBannerCardComboBoxCheckBox;
    QCheckBox *showTagFilterCheckBox;
    QCheckBox *showTagsOnDeckPreviewsCheckBox;
    QCheckBox *searchFolderNamesCheckBox;
    QLabel *unusedColorIdentitiesOpacityLabel;
    QSpinBox *unusedColorIdentitiesOpacitySpinBox;
    CardSizeWidget *cardSizeWidget;

public:
    explicit VisualDeckStorageQuickSettingsWidget(QWidget *parent = nullptr);

    void retranslateUi();

    bool getShowFolders() const;
    bool getDrawUnusedColorIdentities() const;
    bool getShowBannerCardComboBox() const;
    bool getShowTagFilter() const;
    bool getShowTagsOnDeckPreviews() const;
    bool getSearchFolderNames() const;
    int getUnusedColorIdentitiesOpacity() const;
    int getCardSize() const;

signals:
    void showFoldersChanged(bool enabled);
    void drawUnusedColorIdentitiesChanged(bool enabled);
    void showBannerCardComboBoxChanged(bool enabled);
    void showTagFilterChanged(bool enabled);
    void showTagsOnDeckPreviewsChanged(bool enabled);
    void searchFolderNamesChanged(bool enabled);
    void unusedColorIdentitiesOpacityChanged(int opacity);
    void cardSizeChanged(int scale);
};

#endif // VISUAL_DECK_STORAGE_QUICK_SETTINGS_WIDGET_H