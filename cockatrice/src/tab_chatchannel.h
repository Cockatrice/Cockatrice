#ifndef TAB_CHATCHANNEL_H
#define TAB_CHATCHANNEL_H

#include "tab.h"

class AbstractClient;
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
	AbstractClient *client;
	QString channelName;
	
	QListWidget *playerList;
	QTextEdit *textEdit;
	QLineEdit *sayEdit;

	QAction *aLeaveChannel;
	QString sanitizeHtml(QString dirty) const;
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
	TabChatChannel(AbstractClient *_client, const QString &_channelName);
	~TabChatChannel();
	void retranslateUi();
	void processChatEvent(ChatEvent *event);
	QString getChannelName() const { return channelName; }
	QString getTabText() const { return channelName; }
};

#endif
