#ifndef TAB_MESSAGE_H
#define TAB_MESSAGE_H

#include "tab.h"

class AbstractClient;
class ChatView;
class QLineEdit;
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
    QLineEdit *sayEdit;

    QAction *aLeave;
signals:
    void talkClosing(TabMessage *tab);
    void maximizeClient();
private slots:
    void sendMessage();
    void actLeave();
    void messageSent(const Response &response);
    void addMentionTag(QString mentionTag);
    void messageClicked();

public:
    TabMessage(TabSupervisor *_tabSupervisor,
               AbstractClient *_client,
               const ServerInfo_User &_ownUserInfo,
               const ServerInfo_User &_otherUserInfo);
    ~TabMessage();
    void retranslateUi();
    void closeRequest();
    void tabActivated();
    QString getUserName() const;
    QString getTabText() const;

    void processUserMessageEvent(const Event_UserMessage &event);

    void processUserLeft();
    void processUserJoined(const ServerInfo_User &_userInfo);

private:
    bool shouldShowSystemPopup(const Event_UserMessage &event);
    void showSystemPopup(const Event_UserMessage &event);
};

#endif
