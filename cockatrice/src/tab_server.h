#ifndef TAB_SERVER_H
#define TAB_SERVER_H

#include <QGroupBox>
#include "tab.h"
#include "protocol_datastructures.h"

class AbstractClient;
class QTreeView;
class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;
class QCheckBox;
class QTextEdit;

class GamesModel;
class GamesProxyModel;

class Event_ListGames;
class Event_ListChatChannels;
class Event_ServerMessage;
class Event_UserJoined;
class Event_UserLeft;
class ProtocolResponse;

class GameSelector : public QGroupBox {
	Q_OBJECT
public:
	GameSelector(AbstractClient *_client, QWidget *parent = 0);
	void retranslateUi();
private slots:
	void processListGamesEvent(Event_ListGames *event);
	void showFullGamesChanged(int state);
	void actCreate();
	void actJoin();
	void checkResponse(ResponseCode response);
signals:
	void gameJoined(int gameId);
private:
	AbstractClient *client;

	QTreeView *gameListView;
	GamesModel *gameListModel;
	GamesProxyModel *gameListProxyModel;
	QPushButton *createButton, *joinButton, *spectateButton;
	QCheckBox *showFullGamesCheckBox;
};

class ChatChannelSelector : public QGroupBox {
	Q_OBJECT
private:
	QTreeWidget *channelList;
	QPushButton *joinButton;
	AbstractClient *client;
	
	void joinChannel(const QString &channelName);
private slots:
	void processListChatChannelsEvent(Event_ListChatChannels *event);
	void joinClicked();
	void joinFinished(ResponseCode resp);
signals:
	void channelJoined(const QString &channelName);
public:
	ChatChannelSelector(AbstractClient *_client, QWidget *parent = 0);
	void retranslateUi();
};

class ServerMessageLog : public QGroupBox {
	Q_OBJECT
private:
	QTextEdit *textEdit;
private slots:
	void processServerMessageEvent(Event_ServerMessage *event);
public:
	ServerMessageLog(AbstractClient *_client, QWidget *parent = 0);
	void retranslateUi();
};

class UserList : public QGroupBox {
	Q_OBJECT
private:
	QTreeWidget *userTree;
	void processUserInfo(ServerInfo_User *user);
private slots:
	void processResponse(ProtocolResponse *response);
	void processUserJoinedEvent(Event_UserJoined *event);
	void processUserLeftEvent(Event_UserLeft *event);
	void userClicked(QTreeWidgetItem *item, int column);
signals:
	void openMessageDialog(const QString &userName, bool focus);
	void userLeft(const QString &userName);
public:
	UserList(AbstractClient *_client, QWidget *parent = 0);
	void retranslateUi();
};

class TabServer : public Tab {
	Q_OBJECT
signals:
	void chatChannelJoined(const QString &channelName);
	void gameJoined(int gameId);
	void openMessageDialog(const QString &userName, bool focus);
	void userLeft(const QString &userName);
private:
	AbstractClient *client;
	GameSelector *gameSelector;
	ChatChannelSelector *chatChannelSelector;
	ServerMessageLog *serverMessageLog;
	UserList *userList;
public:
	TabServer(AbstractClient *_client, QWidget *parent = 0);
	void retranslateUi();
	QString getTabText() const { return tr("Server"); }
};

#endif
