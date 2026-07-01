#ifndef COCKATRICE_USER_INTERFACE_SETTINGS_PAGE_H
#define COCKATRICE_USER_INTERFACE_SETTINGS_PAGE_H

#include "abstract_settings_page.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QToolButton>
#include <libcockatrice/utility/macros.h>

enum deckEditorCommanderSpellbookIntegrationEnabledIndex
{
    deckEditorCommanderSpellbookIntegrationEnabledIndexDisabled,
    deckEditorCommanderSpellbookIntegrationEnabledIndexEnabled,
    deckEditorCommanderSpellbookIntegrationEnabledIndexAutomatic,
    deckEditorCommanderSpellbookIntegrationEnabledIndexUnprompted,
};

namespace CommanderBracketNames
{
inline const char *CommanderSpellbookBracketNames = QT_TR_NOOP("CommanderSpellbook");
inline const char *OfficialCommanderBracketNames = QT_TR_NOOP("Official (approximate)");
inline const char *Explainer = QT_TR_NOOP(
    "The bracket system combines both objective data, as well as subjective play experience to estimate a "
    "bracket for a deck.\nCommanderSpellbook's estimation is algorithmical, which means that it can only operate "
    "on the objective data, not the subjective intent. \nThey have chosen to represent this by defining their "
    "own bracket system which matches their algorithm.\n"
    "This custom bracket system maps loosely to the standard system. \nYou may choose to use these mapped "
    "standardized names if these are more familiar to you, however, you should keep in mind that these are just "
    "rough estimations.\n\nAlways consider the subjective factors of the bracket system when determing a deck's "
    "final bracket!");
} // namespace CommanderBracketNames

class UserInterfaceSettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
private slots:
    void setNotificationEnabled(QT_STATE_CHANGED_T);
    void updateCommanderSpellbookUiState();

private:
    QCheckBox notificationsEnabledCheckBox;
    QCheckBox specNotificationsEnabledCheckBox;
    QCheckBox buddyConnectNotificationsEnabledCheckBox;
    QCheckBox doubleClickToPlayCheckBox;
    QCheckBox clickPlaysAllSelectedCheckBox;
    QCheckBox playToStackCheckBox;
    QCheckBox doNotDeleteArrowsInSubPhasesCheckBox;
    QCheckBox closeEmptyCardViewCheckBox;
    QCheckBox focusCardViewSearchBarCheckBox;
    QCheckBox annotateTokensCheckBox;
    QCheckBox showDragSelectionCountCheckBox;
    QCheckBox showTotalSelectionCountCheckBox;
    QCheckBox showSubtypeSelectionTallyCheckBox;
    QCheckBox useTearOffMenusCheckBox;
    QCheckBox keepGameChatFocusCheckBox;
    QCheckBox tapAnimationCheckBox;
    QCheckBox openDeckInNewTabCheckBox;
    QLabel visualDeckStoragePromptForConversionLabel;
    QComboBox visualDeckStoragePromptForConversionSelector;
    QCheckBox visualDeckStorageInGameCheckBox;
    QCheckBox visualDeckStorageSelectionAnimationCheckBox;
    QLabel defaultDeckEditorTypeLabel;
    QComboBox defaultDeckEditorTypeSelector;
    QLabel deckEditorCommanderSpellbookIntegrationEnabledLabel;
    QComboBox deckEditorCommanderSpellbookIntegrationEnabledSelector;
    QLabel deckEditorCommanderSpellbookIntegrationUseOfficialBracketNamesLabel;
    QToolButton deckEditorCommanderSpellBookIntegrationUseOfficialBracketNamesExplainer;
    QComboBox deckEditorCommanderSpellbookIntegrationBracketNamingSelector;
    QLabel rewindBufferingMsLabel;
    QSpinBox rewindBufferingMsBox;
    QGroupBox *generalGroupBox;
    QGroupBox *notificationsGroupBox;
    QGroupBox *animationGroupBox;
    QGroupBox *deckEditorGroupBox;
    QGroupBox *replayGroupBox;

public:
    UserInterfaceSettingsPage();
    void retranslateUi() override;
};

#endif // COCKATRICE_USER_INTERFACE_SETTINGS_PAGE_H
