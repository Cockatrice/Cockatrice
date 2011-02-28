#ifndef TAB_MESSAGE_H
#define TAB_MESSAGE_H

#include "tab.h"

class AbstractClient;
class ChatView;
class QLineEdit;
class Event_Message;

class TabMessage : public Tab {
	Q_OBJECT
private:
	AbstractClient *client;
	QString userName;
	bool userOnline;
	
	ChatView *chatView;
	QLineEdit *sayEdit;

	QAction *aLeave;
signals:
	void talkClosing(TabMessage *tab);
private slots:
	void sendMessage();
	void actLeave();
public:
	TabMessage(TabSupervisor *_tabSupervisor, AbstractClient *_client, const QString &_ownName, const QString &_userName);
	~TabMessage();
	void retranslateUi();
	QString getUserName() const { return userName; }
	QString getTabText() const { return tr("Talking to %1").arg(userName); }

	void processMessageEvent(Event_Message *event);
	void processUserLeft();
	void processUserJoined();
};

#endif
