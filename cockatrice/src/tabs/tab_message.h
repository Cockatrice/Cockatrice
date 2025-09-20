#ifndef TAB_MESSAGE_H
#define TAB_MESSAGE_H

#include "tab.h"

#include <QLoggingCategory>

inline Q_LOGGING_CATEGORY(TabMessageLog, "tab_message");

class AbstractClient;
class ChatView;
class LineEditUnfocusable;
class Event_UserMessage;
class Response;
class ServerInfo_User;

class TabMessage : public Tab
{
    Q_OBJECT
private:
    AbstractClient *client;
    QMenu *messageMenu;
    ServerInfo_User *ownUserInfo;
    ServerInfo_User *otherUserInfo;
    bool userOnline;

    ChatView *chatView;
    LineEditUnfocusable *sayEdit;

    QAction *aLeave;
signals:
    void talkClosing(TabMessage *tab);
    void maximizeClient();
private slots:
    void sendMessage();
    void messageSent(const Response &response);
    void addMentionTag(QString mentionTag);
    void messageClicked();

protected slots:
    void closeEvent(QCloseEvent *event) override;

public:
    TabMessage(TabSupervisor *_tabSupervisor,
               AbstractClient *_client,
               const ServerInfo_User &_ownUserInfo,
               const ServerInfo_User &_otherUserInfo);
    ~TabMessage() override;
    void retranslateUi() override;
    void tabActivated() override;
    QString getUserName() const;
    QString getTabText() const override;

    void processUserMessageEvent(const Event_UserMessage &event);

    void processUserLeft();
    void processUserJoined(const ServerInfo_User &_userInfo);

private:
    bool shouldShowSystemPopup(const Event_UserMessage &event);
    void showSystemPopup(const Event_UserMessage &event);
};

#endif
