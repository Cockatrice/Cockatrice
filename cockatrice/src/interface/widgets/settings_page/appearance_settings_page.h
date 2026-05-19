#ifndef COCKATRICE_APPEARANCE_SETTINGS_PAGE_H
#define COCKATRICE_APPEARANCE_SETTINGS_PAGE_H

#include "abstract_settings_page.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <libcockatrice/utility/macros.h>

class AppearanceSettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
private slots:
    void themeBoxChanged(int index);
    void openThemeLocation();
    void editPalette();
    void updateHomeTabSettingsVisibility();
    void showShortcutsChanged(QT_STATE_CHANGED_T enabled);
    void overrideAllCardArtWithPersonalPreferenceToggled(QT_STATE_CHANGED_T enabled);

    void cardViewInitialRowsMaxChanged(int value);
    void cardViewExpandedRowsMaxChanged(int value);

private:
    QLabel themeLabel;
    QComboBox themeBox;
    QPushButton openThemeButton;
    QLabel schemeComboLabel;
    QComboBox schemeCombo;
    QPushButton editPaletteButton;
    QLabel homeTabBackgroundSourceLabel;
    QComboBox homeTabBackgroundSourceBox;
    QLabel homeTabBackgroundShuffleFrequencyLabel;
    QSpinBox homeTabBackgroundShuffleFrequencySpinBox;
    QCheckBox homeTabDisplayCardNameCheckBox;
    QLabel minPlayersForMultiColumnLayoutLabel;
    QLabel maxFontSizeForCardsLabel;
    QCheckBox showShortcutsCheckBox;
    QCheckBox showGameSelectorFilterToolbarCheckBox;
    QCheckBox overrideAllCardArtWithPersonalPreferenceCheckBox;
    QCheckBox bumpSetsWithCardsInDeckToTopCheckBox;
    QCheckBox displayCardNamesCheckBox;
    QCheckBox autoRotateSidewaysLayoutCardsCheckBox;
    QCheckBox cardScalingCheckBox;
    QCheckBox roundCardCornersCheckBox;
    QLabel verticalCardOverlapPercentLabel;
    QSpinBox verticalCardOverlapPercentBox;
    QLabel cardViewInitialRowsMaxLabel;
    QSpinBox cardViewInitialRowsMaxBox;
    QLabel cardViewExpandedRowsMaxLabel;
    QSpinBox cardViewExpandedRowsMaxBox;
    QCheckBox horizontalHandCheckBox;
    QCheckBox leftJustifiedHandCheckBox;
    QCheckBox invertVerticalCoordinateCheckBox;
    QGroupBox *themeGroupBox;
    QGroupBox *homeTabGroupBox;
    QGroupBox *menuGroupBox;
    QGroupBox *printingsGroupBox;
    QGroupBox *cardsGroupBox;
    QGroupBox *handGroupBox;
    QGroupBox *tableGroupBox;
    QGroupBox *cardCountersGroupBox;
    QList<QLabel *> cardCounterNames;
    QSpinBox minPlayersForMultiColumnLayoutEdit;
    QSpinBox maxFontSizeForCardsEdit;

public:
    AppearanceSettingsPage();
    void retranslateUi() override;
};

#endif // COCKATRICE_APPEARANCE_SETTINGS_PAGE_H
