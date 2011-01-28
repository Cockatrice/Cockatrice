#include <QTextEdit>
#include <QDateTime>
#include <QTextTable>
#include <QScrollBar>
#include "chatview.h"

ChatView::ChatView(const QString &_ownName, QWidget *parent)
	: QTextEdit(parent), ownName(_ownName)
{
	setTextInteractionFlags(Qt::TextSelectableByMouse);
}

void ChatView::appendMessage(const QString &sender, const QString &message)
{
	QTextCursor cursor(document()->lastBlock());
	cursor.movePosition(QTextCursor::End);
	
	QTextBlockFormat blockFormat;
	blockFormat.setBottomMargin(3);
	cursor.insertBlock(blockFormat);
	
	QTextCharFormat timeFormat;
	timeFormat.setForeground(Qt::black);
	cursor.setCharFormat(timeFormat);
	cursor.insertText(QDateTime::currentDateTime().toString("[hh:mm] "));
	
	QTextCharFormat senderFormat;
	if (sender == ownName) {
		senderFormat.setFontWeight(QFont::Bold);
		senderFormat.setForeground(Qt::red);
	} else
		senderFormat.setForeground(Qt::blue);
	cursor.setCharFormat(senderFormat);
	cursor.insertText(sender + " ");
	
	QTextCharFormat messageFormat;
	if (sender.isEmpty())
		messageFormat.setForeground(Qt::darkGreen);
	cursor.setCharFormat(messageFormat);
	cursor.insertText(message);
	
	
	verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}
