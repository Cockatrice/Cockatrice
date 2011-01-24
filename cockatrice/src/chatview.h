#ifndef CHATVIEW_H
#define CHATVIEW_H

#include <QTextEdit>

class QTextTable;

class ChatView : public QTextEdit {
	Q_OBJECT;
private:
	QTextTable *table;
	QString ownName;
public:
	ChatView(const QString &_ownName, QWidget *parent = 0);
	void appendMessage(const QString &sender, const QString &message);
};

#endif