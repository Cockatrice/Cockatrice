#ifndef COCKATRICE_USER_INTERFACE_SETTINGS_PAGE_H
#define COCKATRICE_USER_INTERFACE_SETTINGS_PAGE_H

#include "abstract_settings_page.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <libcockatrice/utility/macros.h>

class UserInterfaceSettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
private slots:
    void setNotificationEnabled(QT_STATE_CHANGED_T);

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
