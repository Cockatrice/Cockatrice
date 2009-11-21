#ifndef TAB_CHATCHANNEL_H
#define TAB_CHATCHANNEL_H

#include <QWidget>

class Client;
class QListWidget;
class QTextEdit;
class QLineEdit;
class ChatEvent;
class Event_ChatListPlayers;
class Event_ChatJoinChannel;
class Event_ChatLeaveChannel;
class Event_ChatSay;

class TabChatChannel : public QWidget {
	Q_OBJECT
private:
	Client *client;
	QString channelName;
	
	QListWidget *playerList;
	QTextEdit *textEdit;
	QLineEdit *sayEdit;
private slots:
	void sendMessage();
	
	void processListPlayersEvent(Event_ChatListPlayers *event);
	void processJoinChannelEvent(Event_ChatJoinChannel *event);
	void processLeaveChannelEvent(Event_ChatLeaveChannel *event);
	void processSayEvent(Event_ChatSay *event);
public:
	TabChatChannel(Client *_client, const QString &_channelName);
	void retranslateUi();
	void processChatEvent(ChatEvent *event);
};

#endif