#include "user_interface_settings_page.h"

#include "../../../client/settings/cache_settings.h"
#include "../interface/widgets/tabs/tab_supervisor.h"

#include <QGridLayout>
#include <libcockatrice/settings/cards_display_settings.h>
#include <libcockatrice/settings/interface_settings.h>
#include <libcockatrice/settings/personal_settings.h>
#include <libcockatrice/settings/visual_deck_storage_settings.h>

enum visualDeckStoragePromptForConversionIndex
{
    visualDeckStoragePromptForConversionIndexNone,
    visualDeckStoragePromptForConversionIndexPrompt,
    visualDeckStoragePromptForConversionIndexAlways
};

UserInterfaceSettingsPage::UserInterfaceSettingsPage()
{
    // general settings and notification settings
    notificationsEnabledCheckBox.setChecked(SettingsCache::instance().interface().getNotificationsEnabled());
    connect(&notificationsEnabledCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().interface(),
            &InterfaceSettings::setNotificationsEnabled);
    connect(&notificationsEnabledCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &UserInterfaceSettingsPage::setNotificationEnabled);

    specNotificationsEnabledCheckBox.setChecked(
        SettingsCache::instance().interface().getSpectatorNotificationsEnabled());
    specNotificationsEnabledCheckBox.setEnabled(SettingsCache::instance().interface().getNotificationsEnabled());
    connect(&specNotificationsEnabledCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().interface(),
            &InterfaceSettings::setSpectatorNotificationsEnabled);

    buddyConnectNotificationsEnabledCheckBox.setChecked(
        SettingsCache::instance().interface().getBuddyConnectNotificationsEnabled());
    buddyConnectNotificationsEnabledCheckBox.setEnabled(
        SettingsCache::instance().interface().getNotificationsEnabled());
    connect(&buddyConnectNotificationsEnabledCheckBox, &QCheckBox::QT_STATE_CHANGED,
            &SettingsCache::instance().interface(), &InterfaceSettings::setBuddyConnectNotificationsEnabled);

    doubleClickToPlayCheckBox.setChecked(SettingsCache::instance().interface().getDoubleClickToPlay());
    connect(&doubleClickToPlayCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().interface(),
            &InterfaceSettings::setDoubleClickToPlay);

    clickPlaysAllSelectedCheckBox.setChecked(SettingsCache::instance().interface().getClickPlaysAllSelected());
    connect(&clickPlaysAllSelectedCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().interface(),
            &InterfaceSettings::setClickPlaysAllSelected);

    playToStackCheckBox.setChecked(SettingsCache::instance().interface().getPlayToStack());
    connect(&playToStackCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().interface(),
            &InterfaceSettings::setPlayToStack);

    doNotDeleteArrowsInSubPhasesCheckBox.setChecked(
        SettingsCache::instance().interface().getDoNotDeleteArrowsInSubPhases());
    connect(&doNotDeleteArrowsInSubPhasesCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().interface(),
            &InterfaceSettings::setDoNotDeleteArrowsInSubPhases);

    closeEmptyCardViewCheckBox.setChecked(SettingsCache::instance().interface().getCloseEmptyCardView());
    connect(&closeEmptyCardViewCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().interface(),
            &InterfaceSettings::setCloseEmptyCardView);

    focusCardViewSearchBarCheckBox.setChecked(SettingsCache::instance().interface().getFocusCardViewSearchBar());
    connect(&focusCardViewSearchBarCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().interface(),
            &InterfaceSettings::setFocusCardViewSearchBar);

    annotateTokensCheckBox.setChecked(SettingsCache::instance().interface().getAnnotateTokens());
    connect(&annotateTokensCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().interface(),
            &InterfaceSettings::setAnnotateTokens);

    showDragSelectionCountCheckBox.setChecked(SettingsCache::instance().interface().getShowDragSelectionCount());
    connect(&showDragSelectionCountCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().interface(),
            &InterfaceSettings::setShowDragSelectionCount);

    showTotalSelectionCountCheckBox.setChecked(SettingsCache::instance().interface().getShowTotalSelectionCount());
    connect(&showTotalSelectionCountCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().interface(),
            &InterfaceSettings::setShowTotalSelectionCount);

    useTearOffMenusCheckBox.setChecked(SettingsCache::instance().interface().getUseTearOffMenus());
    connect(&useTearOffMenusCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().interface(),
            [](const QT_STATE_CHANGED_T state) {
                SettingsCache::instance().interface().setUseTearOffMenus(state == Qt::Checked);
            });

    keepGameChatFocusCheckBox.setChecked(SettingsCache::instance().interface().getKeepGameChatFocus());
    connect(&keepGameChatFocusCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().interface(),
            &InterfaceSettings::setKeepGameChatFocus);

    auto *generalGrid = new QGridLayout;
    generalGrid->addWidget(&doubleClickToPlayCheckBox, 0, 0);
    generalGrid->addWidget(&clickPlaysAllSelectedCheckBox, 1, 0);
    generalGrid->addWidget(&playToStackCheckBox, 2, 0);
    generalGrid->addWidget(&doNotDeleteArrowsInSubPhasesCheckBox, 3, 0);
    generalGrid->addWidget(&closeEmptyCardViewCheckBox, 4, 0);
    generalGrid->addWidget(&focusCardViewSearchBarCheckBox, 5, 0);
    generalGrid->addWidget(&annotateTokensCheckBox, 6, 0);
    generalGrid->addWidget(&showDragSelectionCountCheckBox, 7, 0);
    generalGrid->addWidget(&showTotalSelectionCountCheckBox, 8, 0);
    generalGrid->addWidget(&useTearOffMenusCheckBox, 9, 0);
    generalGrid->addWidget(&keepGameChatFocusCheckBox, 10, 0);

    generalGroupBox = new QGroupBox;
    generalGroupBox->setLayout(generalGrid);

    auto *notificationsGrid = new QGridLayout;
    notificationsGrid->addWidget(&notificationsEnabledCheckBox, 0, 0);
    notificationsGrid->addWidget(&specNotificationsEnabledCheckBox, 1, 0);
    notificationsGrid->addWidget(&buddyConnectNotificationsEnabledCheckBox, 2, 0);

    notificationsGroupBox = new QGroupBox;
    notificationsGroupBox->setLayout(notificationsGrid);

    // animation settings
    tapAnimationCheckBox.setChecked(SettingsCache::instance().cardsDisplay().getTapAnimation());
    connect(&tapAnimationCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().cardsDisplay(),
            &CardsDisplaySettings::setTapAnimation);

    auto *animationGrid = new QGridLayout;
    animationGrid->addWidget(&tapAnimationCheckBox, 0, 0);

    animationGroupBox = new QGroupBox;
    animationGroupBox->setLayout(animationGrid);

    // deck editor settings
    openDeckInNewTabCheckBox.setChecked(SettingsCache::instance().interface().getOpenDeckInNewTab());
    connect(&openDeckInNewTabCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().interface(),
            &InterfaceSettings::setOpenDeckInNewTab);

    visualDeckStorageInGameCheckBox.setChecked(
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageInGame());
    connect(&visualDeckStorageInGameCheckBox, &QCheckBox::QT_STATE_CHANGED,
            &SettingsCache::instance().visualDeckStorage(), &VisualDeckStorageSettings::setVisualDeckStorageInGame);

    visualDeckStorageSelectionAnimationCheckBox.setChecked(
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageSelectionAnimation());
    connect(&visualDeckStorageSelectionAnimationCheckBox, &QCheckBox::QT_STATE_CHANGED,
            &SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::setVisualDeckStorageSelectionAnimation);

    visualDeckStoragePromptForConversionSelector.addItem(""); // these will be set in retranslateUI
    visualDeckStoragePromptForConversionSelector.addItem("");
    visualDeckStoragePromptForConversionSelector.addItem("");
    if (SettingsCache::instance().visualDeckStorage().getVisualDeckStoragePromptForConversion()) {
        visualDeckStoragePromptForConversionSelector.setCurrentIndex(visualDeckStoragePromptForConversionIndexPrompt);
    } else if (SettingsCache::instance().visualDeckStorage().getVisualDeckStorageAlwaysConvert()) {
        visualDeckStoragePromptForConversionSelector.setCurrentIndex(visualDeckStoragePromptForConversionIndexAlways);
    } else {
        visualDeckStoragePromptForConversionSelector.setCurrentIndex(visualDeckStoragePromptForConversionIndexNone);
    }
    connect(&visualDeckStoragePromptForConversionSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [](int index) {
                SettingsCache::instance().visualDeckStorage().setVisualDeckStoragePromptForConversion(
                    index == visualDeckStoragePromptForConversionIndexPrompt);
                SettingsCache::instance().visualDeckStorage().setVisualDeckStorageAlwaysConvert(
                    index == visualDeckStoragePromptForConversionIndexAlways);
            });

    defaultDeckEditorTypeSelector.addItem(""); // these will be set in retranslateUI
    defaultDeckEditorTypeSelector.addItem("");
    defaultDeckEditorTypeSelector.setCurrentIndex(
        SettingsCache::instance().visualDeckStorage().getDefaultDeckEditorType());
    connect(&defaultDeckEditorTypeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            &SettingsCache::instance().visualDeckStorage(), &VisualDeckStorageSettings::setDefaultDeckEditorType);

    auto *deckEditorGrid = new QGridLayout;
    deckEditorGrid->addWidget(&openDeckInNewTabCheckBox, 0, 0);
    deckEditorGrid->addWidget(&visualDeckStorageInGameCheckBox, 1, 0);
    deckEditorGrid->addWidget(&visualDeckStorageSelectionAnimationCheckBox, 2, 0);
    deckEditorGrid->addWidget(&visualDeckStoragePromptForConversionLabel, 3, 0);
    deckEditorGrid->addWidget(&visualDeckStoragePromptForConversionSelector, 3, 1);
    deckEditorGrid->addWidget(&defaultDeckEditorTypeLabel, 4, 0);
    deckEditorGrid->addWidget(&defaultDeckEditorTypeSelector, 4, 1);

    deckEditorGroupBox = new QGroupBox;
    deckEditorGroupBox->setLayout(deckEditorGrid);

    // replay settings
    rewindBufferingMsBox.setRange(0, 9999);
    rewindBufferingMsBox.setValue(SettingsCache::instance().interface().getRewindBufferingMs());
    connect(&rewindBufferingMsBox, qOverload<int>(&QSpinBox::valueChanged), &SettingsCache::instance().interface(),
            &InterfaceSettings::setRewindBufferingMs);

    auto *replayGrid = new QGridLayout;
    replayGrid->addWidget(&rewindBufferingMsLabel, 0, 0, 1, 1);
    replayGrid->addWidget(&rewindBufferingMsBox, 0, 1, 1, 1);

    replayGroupBox = new QGroupBox;
    replayGroupBox->setLayout(replayGrid);

    // putting it all together
    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(generalGroupBox);
    mainLayout->addWidget(notificationsGroupBox);
    mainLayout->addWidget(animationGroupBox);
    mainLayout->addWidget(deckEditorGroupBox);
    mainLayout->addWidget(replayGroupBox);
    mainLayout->addStretch();

    setLayout(mainLayout);

    connect(&SettingsCache::instance().personal(), &PersonalSettings::langChanged, this,
            &UserInterfaceSettingsPage::retranslateUi);
    retranslateUi();
}

void UserInterfaceSettingsPage::setNotificationEnabled(QT_STATE_CHANGED_T i)
{
    specNotificationsEnabledCheckBox.setEnabled(i != 0);
    buddyConnectNotificationsEnabledCheckBox.setEnabled(i != 0);
    if (i == 0) {
        specNotificationsEnabledCheckBox.setChecked(false);
        buddyConnectNotificationsEnabledCheckBox.setChecked(false);
    }
}

void UserInterfaceSettingsPage::retranslateUi()
{
    generalGroupBox->setTitle(tr("General interface settings"));
    doubleClickToPlayCheckBox.setText(tr("&Double-click cards to play them (instead of single-click)"));
    clickPlaysAllSelectedCheckBox.setText(tr("&Clicking plays all selected cards (instead of just the clicked card)"));
    playToStackCheckBox.setText(tr("&Play all nonlands onto the stack (not the battlefield) by default"));
    doNotDeleteArrowsInSubPhasesCheckBox.setText(tr("Do not delete &arrows inside of subphases"));
    closeEmptyCardViewCheckBox.setText(tr("Close card view window when last card is removed"));
    focusCardViewSearchBarCheckBox.setText(tr("Auto focus search bar when card view window is opened"));
    annotateTokensCheckBox.setText(tr("Annotate card text on tokens"));
    showDragSelectionCountCheckBox.setText(tr("Show selection count during drag selection"));
    showTotalSelectionCountCheckBox.setText(tr("Show total selection count"));
    useTearOffMenusCheckBox.setText(tr("Use tear-off menus, allowing right click menus to persist on screen"));
    keepGameChatFocusCheckBox.setText(
        tr("Keep game chat focused when clicking in game (Note: disables card view search bar)"));

    notificationsGroupBox->setTitle(tr("Notifications settings"));
    notificationsEnabledCheckBox.setText(tr("Enable notifications in taskbar"));
    specNotificationsEnabledCheckBox.setText(tr("Notify in the taskbar for game events while you are spectating"));
    buddyConnectNotificationsEnabledCheckBox.setText(tr("Notify in the taskbar when users in your buddy list connect"));
    animationGroupBox->setTitle(tr("Animation settings"));
    tapAnimationCheckBox.setText(tr("&Tap/untap animation"));
    deckEditorGroupBox->setTitle(tr("Deck editor/storage settings"));
    openDeckInNewTabCheckBox.setText(tr("Open deck in new tab by default"));
    visualDeckStorageInGameCheckBox.setText(tr("Use visual deck storage in game lobby"));
    visualDeckStorageSelectionAnimationCheckBox.setText(tr("Use selection animation for Visual Deck Storage"));
    visualDeckStoragePromptForConversionLabel.setText(
        tr("When adding a tag in the visual deck storage to a .txt deck:"));
    visualDeckStoragePromptForConversionSelector.setItemText(visualDeckStoragePromptForConversionIndexNone,
                                                             tr("do nothing"));
    visualDeckStoragePromptForConversionSelector.setItemText(visualDeckStoragePromptForConversionIndexPrompt,
                                                             tr("ask to convert to .cod"));
    visualDeckStoragePromptForConversionSelector.setItemText(visualDeckStoragePromptForConversionIndexAlways,
                                                             tr("always convert to .cod"));
    defaultDeckEditorTypeLabel.setText(tr("Default deck editor type"));
    defaultDeckEditorTypeSelector.setItemText(TabSupervisor::ClassicDeckEditor, tr("Classic Deck Editor"));
    defaultDeckEditorTypeSelector.setItemText(TabSupervisor::VisualDeckEditor, tr("Visual Deck Editor"));
    replayGroupBox->setTitle(tr("Replay settings"));
    rewindBufferingMsLabel.setText(tr("Buffer time for backwards skip via shortcut:"));
    rewindBufferingMsBox.setSuffix(" ms");
}