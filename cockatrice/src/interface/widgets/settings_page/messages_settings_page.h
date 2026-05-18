#ifndef COCKATRICE_MESSAGES_SETTINGS_PAGE_H
#define COCKATRICE_MESSAGES_SETTINGS_PAGE_H

#include "abstract_settings_page.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <libcockatrice/utility/macros.h>

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
    QCheckBox messagePopups;
    QCheckBox mentionPopups;
    QCheckBox roomHistory;
    QGroupBox *chatGroupBox;
    QGroupBox *highlightGroupBox;
    QGroupBox *messageGroupBox;
    QLineEdit *mentionColor;
    QLineEdit *highlightColor;
    QLineEdit *customAlertString;
    QLabel hexLabel;
    QLabel hexHighlightLabel;
    QLabel customAlertStringLabel;
    QLabel explainMessagesLabel;

    void storeSettings();
    void updateMentionPreview();
    void updateHighlightPreview();
};

#endif // COCKATRICE_MESSAGES_SETTINGS_PAGE_H
