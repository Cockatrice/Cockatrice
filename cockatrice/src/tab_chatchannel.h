#ifndef TAB_CHATCHANNEL_H
#define TAB_CHATCHANNEL_H

#include "tab.h"

class Client;
class QListWidget;
class QTextEdit;
class QLineEdit;
class ChatEvent;
class Event_ChatListPlayers;
class Event_ChatJoinChannel;
class Event_ChatLeaveChannel;
class Event_ChatSay;

class TabChatChannel : public Tab {
	Q_OBJECT
private:
	Client *client;
	QString channelName;
	
	QListWidget *playerList;
	QTextEdit *textEdit;
	QLineEdit *sayEdit;

	QAction *aLeaveChannel;
signals:
	void channelClosing(TabChatChannel *tab);
private slots:
	void sendMessage();
	void actLeaveChannel();
	
	void processListPlayersEvent(Event_ChatListPlayers *event);
	void processJoinChannelEvent(Event_ChatJoinChannel *event);
	void processLeaveChannelEvent(Event_ChatLeaveChannel *event);
	void processSayEvent(Event_ChatSay *event);
public:
	TabChatChannel(Client *_client, const QString &_channelName);
	~TabChatChannel();
	void retranslateUi();
	void processChatEvent(ChatEvent *event);
	QString getChannelName() const { return channelName; }
};

#endif
