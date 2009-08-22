#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include "servereventdata.h"

class QListWidget;
class QTextEdit;
class QLineEdit;
class QTreeWidget;
class QTabWidget;
class Client;

class ChannelWidget : public QWidget {
private:
	QListWidget *playerList;
	QTextEdit *textEdit;
	QLineEdit *sayEdit;
public:
	ChannelWidget(QWidget *parent = 0);
};

class ChatWidget : public QWidget {
	Q_OBJECT
private:
	QTreeWidget *channelList;
	QTabWidget *tab;
	Client *client;
private slots:
	void chatEvent(const ChatEventData &data);
public:
	ChatWidget(Client *_client, QWidget *parent = 0);
	void retranslateUi();
	void enableChat();
	void disableChat();
};

#endif
