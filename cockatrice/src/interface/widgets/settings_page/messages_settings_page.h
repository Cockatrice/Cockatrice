#ifndef COCKATRICE_MESSAGES_SETTINGS_PAGE_H
#define COCKATRICE_MESSAGES_SETTINGS_PAGE_H

#include "abstract_settings_page.h"

#include <QCheckBox>
#include <QLabel>
#include <QString>
#include <libcockatrice/utility/macros.h>
#include <qtmetamacros.h>

class QAction;
class QGroupBox;
class QLineEdit;
class QListWidget;

class MessagesSettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
public:
    MessagesSettingsPage();
    void retranslateUi() override;

private slots:
    void actAdd();
    void actEdit();
    void actRemove();
    void updateColor(const QString &value);
    void updateHighlightColor(const QString &value);
    void updateChannelUnreadColor(const QString &value);
    void updateChannelMentionColor(const QString &value);
    void updateTextColor(QT_STATE_CHANGED_T value);
    void updateTextHighlightColor(QT_STATE_CHANGED_T value);

private:
    QListWidget *messageList;
    QAction *aAdd;
    QAction *aEdit;
    QAction *aRemove;
    QCheckBox chatMentionCheckBox;
    QCheckBox chatMentionCompleterCheckbox;
    QCheckBox invertMentionForeground;
    QCheckBox invertHighlightForeground;
    QCheckBox ignoreUnregUsersMainChat;
    QCheckBox ignoreUnregUserMessages;
    QCheckBox ignoreNonBuddyUserMessages;
    QCheckBox messagePopups;
    QCheckBox mentionPopups;
    QCheckBox roomHistory;
    QCheckBox ignoreAllPrivateMessagesCheckBox;
    QCheckBox useGameTimeCheckBox;
    QGroupBox *chatGroupBox;
    QGroupBox *highlightGroupBox;
    QGroupBox *channelGroupBox;
    QGroupBox *messageGroupBox;
    QLineEdit *mentionColor;
    QLineEdit *highlightColor;
    QLineEdit *channelUnreadColor;
    QLineEdit *channelMentionColor;
    QLineEdit *customAlertString;
    QLabel hexLabel;
    QLabel hexHighlightLabel;
    QLabel channelUnreadColorLabel;
    QLabel channelMentionColorLabel;
    QLabel hexChannelUnreadLabel;
    QLabel hexChannelMentionLabel;
    QLabel customAlertStringLabel;
    QLabel explainMessagesLabel;

    void storeSettings();
    void updateMentionPreview();
    void updateHighlightPreview();
    void updateChannelUnreadPreview();
    void updateChannelMentionPreview();
};

#endif // COCKATRICE_MESSAGES_SETTINGS_PAGE_H
