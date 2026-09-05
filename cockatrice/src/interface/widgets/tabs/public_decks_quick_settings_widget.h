/**
 * @file public_decks_quick_settings_widget.h
 * @ingroup Tabs
 * @brief The quick settings menu for the public decks tab.
 * Manages the widgets in the quick settings menu dropdown of the public decks
 * tab, and syncs their values with the same SettingsCache keys the Visual Deck
 * Storage uses, so shared preview widgets (color identity, tags) behave the
 * same way in both places.
 */

#ifndef PUBLIC_DECKS_QUICK_SETTINGS_WIDGET_H
#define PUBLIC_DECKS_QUICK_SETTINGS_WIDGET_H

#include "../quick_settings/settings_button_widget.h"

class CardSizeWidget;
class QCheckBox;
class QLabel;
class QSpinBox;

class PublicDecksQuickSettingsWidget : public SettingsButtonWidget
{
    Q_OBJECT

    QCheckBox *showColorIdentityCheckBox;
    QCheckBox *drawUnusedColorIdentitiesCheckBox;
    QCheckBox *showTagFilterCheckBox;
    QCheckBox *showTagsOnDeckPreviewsCheckBox;
    QCheckBox *showUploadTimeCheckBox;
    QLabel *unusedColorIdentitiesOpacityLabel;
    QSpinBox *unusedColorIdentitiesOpacitySpinBox;
    CardSizeWidget *cardSizeWidget;

public:
    explicit PublicDecksQuickSettingsWidget(QWidget *parent = nullptr);

    void retranslateUi();

    [[nodiscard]] bool getDrawUnusedColorIdentities() const;
    [[nodiscard]] bool getShowColorIdentity() const;
    [[nodiscard]] bool getShowTagFilter() const;
    [[nodiscard]] bool getShowTagsOnDeckPreviews() const;
    [[nodiscard]] bool getShowUploadTime() const;
    [[nodiscard]] int getUnusedColorIdentitiesOpacity() const;
    [[nodiscard]] CardSizeWidget *getCardSizeWidget() const;

signals:
    void drawUnusedColorIdentitiesChanged(bool enabled);
    void showColorIdentityChanged(bool enabled);
    void showTagFilterChanged(bool enabled);
    void showTagsOnDeckPreviewsChanged(bool enabled);
    void showUploadTimeChanged(bool enabled);
    void unusedColorIdentitiesOpacityChanged(int opacity);
    void cardSizeChanged(int scale);
};

#endif // PUBLIC_DECKS_QUICK_SETTINGS_WIDGET_H