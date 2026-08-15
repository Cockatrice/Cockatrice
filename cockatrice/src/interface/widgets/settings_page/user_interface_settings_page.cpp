#include "user_interface_settings_page.h"

#include "../../../client/settings/cache_settings.h"
#include "../interface/widgets/tabs/tab_supervisor.h"
#include "../tabs/api/commander_spellbook/commander_spellbook_bracket_explainer.h"

#include <QGridLayout>
#include <libcockatrice/settings/cards_display_settings.h>
#include <libcockatrice/settings/deck_editor_settings.h>
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
    notificationsEnabledCheckBox.setChecked(SettingsCache::instance().userInterface().getNotificationsEnabled());
    connect(&notificationsEnabledCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().userInterface(),
            &InterfaceSettings::setNotificationsEnabled);
    connect(&notificationsEnabledCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &UserInterfaceSettingsPage::setNotificationEnabled);

    specNotificationsEnabledCheckBox.setChecked(
        SettingsCache::instance().userInterface().getSpectatorNotificationsEnabled());
    specNotificationsEnabledCheckBox.setEnabled(SettingsCache::instance().userInterface().getNotificationsEnabled());
    connect(&specNotificationsEnabledCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().userInterface(),
            &InterfaceSettings::setSpectatorNotificationsEnabled);

    buddyConnectNotificationsEnabledCheckBox.setChecked(
        SettingsCache::instance().userInterface().getBuddyConnectNotificationsEnabled());
    buddyConnectNotificationsEnabledCheckBox.setEnabled(
        SettingsCache::instance().userInterface().getNotificationsEnabled());
    connect(&buddyConnectNotificationsEnabledCheckBox, &QCheckBox::QT_STATE_CHANGED,
            &SettingsCache::instance().userInterface(), &InterfaceSettings::setBuddyConnectNotificationsEnabled);

    doubleClickToPlayCheckBox.setChecked(SettingsCache::instance().userInterface().getDoubleClickToPlay());
    connect(&doubleClickToPlayCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().userInterface(),
            &InterfaceSettings::setDoubleClickToPlay);

    clickPlaysAllSelectedCheckBox.setChecked(SettingsCache::instance().userInterface().getClickPlaysAllSelected());
    connect(&clickPlaysAllSelectedCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().userInterface(),
            &InterfaceSettings::setClickPlaysAllSelected);

    playToStackCheckBox.setChecked(SettingsCache::instance().userInterface().getPlayToStack());
    connect(&playToStackCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().userInterface(),
            &InterfaceSettings::setPlayToStack);

    doNotDeleteArrowsInSubPhasesCheckBox.setChecked(
        SettingsCache::instance().userInterface().getDoNotDeleteArrowsInSubPhases());
    connect(&doNotDeleteArrowsInSubPhasesCheckBox, &QCheckBox::QT_STATE_CHANGED,
            &SettingsCache::instance().userInterface(), &InterfaceSettings::setDoNotDeleteArrowsInSubPhases);

    closeEmptyCardViewCheckBox.setChecked(SettingsCache::instance().userInterface().getCloseEmptyCardView());
    connect(&closeEmptyCardViewCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().userInterface(),
            &InterfaceSettings::setCloseEmptyCardView);

    focusCardViewSearchBarCheckBox.setChecked(SettingsCache::instance().userInterface().getFocusCardViewSearchBar());
    connect(&focusCardViewSearchBarCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().userInterface(),
            &InterfaceSettings::setFocusCardViewSearchBar);

    annotateTokensCheckBox.setChecked(SettingsCache::instance().userInterface().getAnnotateTokens());
    connect(&annotateTokensCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().userInterface(),
            &InterfaceSettings::setAnnotateTokens);

    showDragSelectionCountCheckBox.setChecked(SettingsCache::instance().userInterface().getShowDragSelectionCount());
    connect(&showDragSelectionCountCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().userInterface(),
            &InterfaceSettings::setShowDragSelectionCount);

    showTotalSelectionCountCheckBox.setChecked(SettingsCache::instance().userInterface().getShowTotalSelectionCount());
    connect(&showTotalSelectionCountCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().userInterface(),
            &InterfaceSettings::setShowTotalSelectionCount);

    useTearOffMenusCheckBox.setChecked(SettingsCache::instance().userInterface().getUseTearOffMenus());
    connect(&useTearOffMenusCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().userInterface(),
            [](const QT_STATE_CHANGED_T state) {
                SettingsCache::instance().userInterface().setUseTearOffMenus(state == Qt::Checked);
            });

    keepGameChatFocusCheckBox.setChecked(SettingsCache::instance().userInterface().getKeepGameChatFocus());
    connect(&keepGameChatFocusCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().userInterface(),
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

    arrowDrawAnimationCheckBox.setChecked(SettingsCache::instance().cardsDisplay().getArrowDrawAnimation());
    connect(&arrowDrawAnimationCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().cardsDisplay(),
            &CardsDisplaySettings::setArrowDrawAnimation);

    lifeCounterAnimationsCheckBox.setChecked(
        SettingsCache::instance().userInterface().getLifeCounterAnimationsEnabled());
    connect(&lifeCounterAnimationsCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().userInterface(),
            &InterfaceSettings::setLifeCounterAnimationsEnabled);

    battlefieldFlashCheckBox.setChecked(SettingsCache::instance().userInterface().getBattlefieldFlashEnabled());
    connect(&battlefieldFlashCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().userInterface(),
            &InterfaceSettings::setBattlefieldFlashEnabled);

    connect(&enableAllAnimationsButton, &QPushButton::clicked, this, &UserInterfaceSettingsPage::enableAllAnimations);
    connect(&disableAllAnimationsButton, &QPushButton::clicked, this, &UserInterfaceSettingsPage::disableAllAnimations);

    auto *animationGrid = new QGridLayout;
    animationGrid->addWidget(&enableAllAnimationsButton, 0, 0);
    animationGrid->addWidget(&disableAllAnimationsButton, 0, 1);
    animationGrid->addWidget(&tapAnimationCheckBox, 1, 0);
    animationGrid->addWidget(&arrowDrawAnimationCheckBox, 2, 0);
    animationGrid->addWidget(&lifeCounterAnimationsCheckBox, 3, 0);
    animationGrid->addWidget(&battlefieldFlashCheckBox, 4, 0);

    animationGroupBox = new QGroupBox;
    animationGroupBox->setLayout(animationGrid);

    // deck editor settings
    openDeckInNewTabCheckBox.setChecked(SettingsCache::instance().deckEditor().getOpenDeckInNewTab());
    connect(&openDeckInNewTabCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().deckEditor(),
            &DeckEditorSettings::setOpenDeckInNewTab);

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
    defaultDeckEditorTypeSelector.setCurrentIndex(SettingsCache::instance().deckEditor().getDefaultDeckEditorType());
    connect(&defaultDeckEditorTypeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            &SettingsCache::instance().deckEditor(), &DeckEditorSettings::setDefaultDeckEditorType);

    vdeStartupTabSelector.addItem(""); // these will be set in retranslateUI
    vdeStartupTabSelector.addItem("");
    vdeStartupTabSelector.addItem("");
    vdeStartupTabSelector.setCurrentIndex(SettingsCache::instance().deckEditor().getVdeStartupTab());
    connect(&vdeStartupTabSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            &SettingsCache::instance().deckEditor(), &DeckEditorSettings::setVdeStartupTab);

    commanderSpellbookIntegrationUseOfficialBracketNamesExplainer.setText("?");
    commanderSpellbookIntegrationUseOfficialBracketNamesExplainer.setAutoRaise(true);
    commanderSpellbookIntegrationUseOfficialBracketNamesExplainer.setEnabled(false);

    // Add items with userData = internal enum
    commanderSpellbookIntegrationEnabledSelector.addItem(tr("Disabled"),
                                                         commanderSpellbookIntegrationEnabledIndexDisabled);
    commanderSpellbookIntegrationEnabledSelector.addItem(tr("Enabled"),
                                                         commanderSpellbookIntegrationEnabledIndexEnabled);
    commanderSpellbookIntegrationEnabledSelector.addItem(tr("Automatic"),
                                                         commanderSpellbookIntegrationEnabledIndexAutomatic);

    int storedMode = SettingsCache::instance().deckEditor().getCommanderSpellbookIntegrationEnabled();
    for (int i = 0; i < commanderSpellbookIntegrationEnabledSelector.count(); ++i) {
        if (commanderSpellbookIntegrationEnabledSelector.itemData(i).toInt() == storedMode) {
            commanderSpellbookIntegrationEnabledSelector.setCurrentIndex(i);
            break;
        }
    }

    connect(&commanderSpellbookIntegrationEnabledSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                int mode = commanderSpellbookIntegrationEnabledSelector.itemData(index).toInt();
                SettingsCache::instance().deckEditor().setCommanderSpellbookIntegrationEnabled(mode);
                updateCommanderSpellbookUiState();
            });

    commanderSpellbookIntegrationBracketNamingSelector.addItem(
        tr("CommanderSpellbook bracket names")); // index 0 = false
    commanderSpellbookIntegrationBracketNamingSelector.addItem(
        tr("Official Commander bracket names (approximate)")); // index 1 = true

    commanderSpellbookIntegrationBracketNamingSelector.setCurrentIndex(
        SettingsCache::instance().deckEditor().getCommanderSpellbookIntegrationUseOfficialBracketNames() ? 1 : 0);

    connect(&commanderSpellbookIntegrationBracketNamingSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            &SettingsCache::instance(), [](int index) {
                SettingsCache::instance().deckEditor().setCommanderSpellbookIntegrationUseOfficialBracketNames(index ==
                                                                                                               1);
            });

    updateCommanderSpellbookUiState();

    auto *labelLayout = new QHBoxLayout;
    labelLayout->setContentsMargins(0, 0, 0, 0);
    labelLayout->addWidget(&commanderSpellbookIntegrationUseOfficialBracketNamesLabel);
    labelLayout->addWidget(&commanderSpellbookIntegrationUseOfficialBracketNamesExplainer);

    auto *labelWidget = new QWidget;
    labelWidget->setLayout(labelLayout);

    auto *deckEditorGrid = new QGridLayout;
    deckEditorGrid->addWidget(&openDeckInNewTabCheckBox, 0, 0);
    deckEditorGrid->addWidget(&visualDeckStorageInGameCheckBox, 1, 0);
    deckEditorGrid->addWidget(&visualDeckStorageSelectionAnimationCheckBox, 2, 0);
    deckEditorGrid->addWidget(&visualDeckStoragePromptForConversionLabel, 3, 0);
    deckEditorGrid->addWidget(&visualDeckStoragePromptForConversionSelector, 3, 1);
    deckEditorGrid->addWidget(&defaultDeckEditorTypeLabel, 4, 0);
    deckEditorGrid->addWidget(&defaultDeckEditorTypeSelector, 4, 1);
    deckEditorGrid->addWidget(&vdeStartupTabLabel, 5, 0);
    deckEditorGrid->addWidget(&vdeStartupTabSelector, 5, 1);
    deckEditorGrid->addWidget(&commanderSpellbookIntegrationEnabledLabel, 6, 0);
    deckEditorGrid->addWidget(&commanderSpellbookIntegrationEnabledSelector, 6, 1);
    deckEditorGrid->addWidget(labelWidget, 7, 0);
    deckEditorGrid->addWidget(&commanderSpellbookIntegrationBracketNamingSelector, 7, 1);

    deckEditorGroupBox = new QGroupBox;
    deckEditorGroupBox->setLayout(deckEditorGrid);

    // replay settings
    rewindBufferingMsBox.setRange(0, 9999);
    rewindBufferingMsBox.setValue(SettingsCache::instance().userInterface().getRewindBufferingMs());
    connect(&rewindBufferingMsBox, qOverload<int>(&QSpinBox::valueChanged), &SettingsCache::instance().userInterface(),
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

void UserInterfaceSettingsPage::enableAllAnimations()
{
    tapAnimationCheckBox.setChecked(true);
    arrowDrawAnimationCheckBox.setChecked(true);
    lifeCounterAnimationsCheckBox.setChecked(true);
    battlefieldFlashCheckBox.setChecked(true);
}

void UserInterfaceSettingsPage::disableAllAnimations()
{
    tapAnimationCheckBox.setChecked(false);
    arrowDrawAnimationCheckBox.setChecked(false);
    lifeCounterAnimationsCheckBox.setChecked(false);
    battlefieldFlashCheckBox.setChecked(false);
}

void UserInterfaceSettingsPage::updateCommanderSpellbookUiState()
{
    const int mode = SettingsCache::instance().deckEditor().getCommanderSpellbookIntegrationEnabled();

    const bool enabled = mode != commanderSpellbookIntegrationEnabledIndexDisabled &&
                         mode != commanderSpellbookIntegrationEnabledIndexUnprompted;

    commanderSpellbookIntegrationBracketNamingSelector.setEnabled(enabled);
    commanderSpellbookIntegrationUseOfficialBracketNamesExplainer.setEnabled(enabled);
    commanderSpellbookIntegrationUseOfficialBracketNamesLabel.setVisible(enabled);
    commanderSpellbookIntegrationUseOfficialBracketNamesExplainer.setVisible(enabled);
    commanderSpellbookIntegrationBracketNamingSelector.setVisible(enabled);

    if (enabled) {
        // Sync selector with the current stored bool
        const bool useOfficial =
            SettingsCache::instance().deckEditor().getCommanderSpellbookIntegrationUseOfficialBracketNames();
        commanderSpellbookIntegrationBracketNamingSelector.setCurrentIndex(useOfficial ? 1 : 0);
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
    enableAllAnimationsButton.setText(tr("&Enable all animations"));
    disableAllAnimationsButton.setText(tr("&Disable all animations"));
    tapAnimationCheckBox.setText(tr("&Tap/untap animation"));
    arrowDrawAnimationCheckBox.setText(tr("&Arrow draw animation"));
    lifeCounterAnimationsCheckBox.setText(tr("Life counter flash"));
    battlefieldFlashCheckBox.setText(tr("Battlefield flash on damage"));
    deckEditorGroupBox->setTitle(tr("Deck editor/storage settings"));
    openDeckInNewTabCheckBox.setText(tr("Open deck in new tab by default"));
    visualDeckStorageInGameCheckBox.setText(tr("Use visual deck storage in game lobby"));
    visualDeckStorageSelectionAnimationCheckBox.setText(tr("Use selection animation for Visual Deck Storage"));
    visualDeckStoragePromptForConversionLabel.setText(
        tr("When adding a tag in the visual deck storage to a .txt deck:"));
    visualDeckStoragePromptForConversionSelector.setItemText(visualDeckStoragePromptForConversionIndexNone,
                                                             tr("Do nothing"));
    visualDeckStoragePromptForConversionSelector.setItemText(visualDeckStoragePromptForConversionIndexPrompt,
                                                             tr("Ask to convert to .cod"));
    visualDeckStoragePromptForConversionSelector.setItemText(visualDeckStoragePromptForConversionIndexAlways,
                                                             tr("Always convert to .cod"));
    defaultDeckEditorTypeLabel.setText(tr("Default deck editor type"));
    defaultDeckEditorTypeSelector.setItemText(TabSupervisor::ClassicDeckEditor, tr("Classic Deck Editor"));
    defaultDeckEditorTypeSelector.setItemText(TabSupervisor::VisualDeckEditor, tr("Visual Deck Editor"));
    vdeStartupTabLabel.setText(tr("Visual deck editor startup tab"));
    vdeStartupTabSelector.setItemText(VdeStartupTabContext, tr("Context"));
    vdeStartupTabSelector.setItemText(VdeStartupTabDeckDisplay, tr("Deck display"));
    vdeStartupTabSelector.setItemText(VdeStartupTabDatabaseDisplay, tr("Database display"));
    vdeStartupTabSelector.setToolTip(
        tr("New decks open on the database display; existing decks open on the deck view."));

    commanderSpellbookIntegrationEnabledLabel.setText(
        tr("CommanderSpellbook integration to estimate commander bracket"));
    commanderSpellbookIntegrationEnabledSelector.setItemText(commanderSpellbookIntegrationEnabledIndexDisabled,
                                                             tr("Disabled"));
    commanderSpellbookIntegrationEnabledSelector.setItemText(commanderSpellbookIntegrationEnabledIndexEnabled,
                                                             tr("Enabled"));
    commanderSpellbookIntegrationEnabledSelector.setItemText(commanderSpellbookIntegrationEnabledIndexAutomatic,
                                                             tr("Automatic"));
    commanderSpellbookIntegrationUseOfficialBracketNamesLabel.setText(tr("Bracket naming"));
    commanderSpellbookIntegrationBracketNamingSelector.setItemText(
        0, CommanderBracketNames::CommanderSpellbookBracketNames);
    commanderSpellbookIntegrationBracketNamingSelector.setItemText(
        1, CommanderBracketNames::OfficialCommanderBracketNames);

    commanderSpellbookIntegrationUseOfficialBracketNamesExplainer.setToolTip(CommanderBracketNames::Explainer);
    replayGroupBox->setTitle(tr("Replay settings"));
    rewindBufferingMsLabel.setText(tr("Buffer time for backwards skip via shortcut:"));
    rewindBufferingMsBox.setSuffix(" ms");
}
