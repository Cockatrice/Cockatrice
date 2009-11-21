#ifndef TAB_SERVER_H
#define TAB_SERVER_H

#include <QGroupBox>
#include "protocol_datastructures.h"

class Client;
class QTreeView;
class QTreeWidget;
class QPushButton;
class QCheckBox;
class QTextEdit;

class GamesModel;
class GamesProxyModel;

class Event_ListGames;
class Event_ListChatChannels;
class Event_ServerMessage;

class GameSelector : public QGroupBox {
	Q_OBJECT
public:
	GameSelector(Client *_client, QWidget *parent = 0);
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
	Client *client;

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
	Client *client;
	
	void joinChannel(const QString &channelName);
private slots:
	void processListChatChannelsEvent(Event_ListChatChannels *event);
	void joinClicked();
	void joinFinished(ResponseCode resp);
signals:
	void channelJoined(const QString &channelName);
public:
	ChatChannelSelector(Client *_client, QWidget *parent = 0);
	void retranslateUi();
};

class ServerMessageLog : public QGroupBox {
	Q_OBJECT
private:
	QTextEdit *textEdit;
private slots:
	void processServerMessageEvent(Event_ServerMessage *event);
public:
	ServerMessageLog(Client *_client, QWidget *parent = 0);
	void retranslateUi();
};

class TabServer : public QWidget {
	Q_OBJECT
signals:
	void chatChannelJoined(const QString &channelName);
	void gameJoined(int gameId);
private:
	Client *client;
	GameSelector *gameSelector;
	ChatChannelSelector *chatChannelSelector;
	ServerMessageLog *serverMessageLog;
public:
	TabServer(Client *_client, QWidget *parent = 0);
	void retranslateUi();
};

#endif
