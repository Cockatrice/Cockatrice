#ifndef TAB_MESSAGE_H
#define TAB_MESSAGE_H

#include "tab.h"

class AbstractClient;
class QTextEdit;
class QLineEdit;
class Event_Message;

class TabMessage : public Tab {
	Q_OBJECT
private:
	AbstractClient *client;
	QString userName;
	
	QTextEdit *textEdit;
	QLineEdit *sayEdit;

	QAction *aLeave;
	QString sanitizeHtml(QString dirty) const;
signals:
	void talkClosing(TabMessage *tab);
private slots:
	void sendMessage();
	void actLeave();
public slots:	
	void processMessageEvent(Event_Message *event);
	void processUserLeft(const QString &userName);
public:
	TabMessage(AbstractClient *_client, const QString &_userName);
	~TabMessage();
	void retranslateUi();
	QString getUserName() const { return userName; }
	QString getTabText() const { return tr("Talking to %1").arg(userName); }
};

#endif
