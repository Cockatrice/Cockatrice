#include <QTextEdit>
#include <QDateTime>
#include <QTextTable>
#include <QScrollBar>
#include "chatview.h"

ChatView::ChatView(const QString &_ownName, QWidget *parent)
	: QTextEdit(parent), ownName(_ownName)
{
	setTextInteractionFlags(Qt::TextSelectableByMouse);
	
	QTextTableFormat format;
	format.setBorderStyle(QTextFrameFormat::BorderStyle_None);
	table = textCursor().insertTable(1, 3, format);
}

void ChatView::appendMessage(const QString &sender, const QString &message)
{
	QTextCursor cellCursor = table->cellAt(table->rows() - 1, 0).lastCursorPosition();
	cellCursor.insertText(QDateTime::currentDateTime().toString("[hh:mm]"));
	QTextTableCell senderCell = table->cellAt(table->rows() - 1, 1);
	QTextCharFormat senderFormat;
	if (sender == ownName) {
		senderFormat.setFontWeight(QFont::Bold);
		senderFormat.setForeground(Qt::red);
	} else
		senderFormat.setForeground(Qt::blue);
	senderCell.setFormat(senderFormat);
	cellCursor = senderCell.lastCursorPosition();
	cellCursor.insertText(sender);
	QTextTableCell messageCell = table->cellAt(table->rows() - 1, 2);
	QTextCharFormat messageFormat;
	if (sender.isEmpty())
		messageFormat.setForeground(Qt::darkGreen);
	messageCell.setFormat(messageFormat);
	cellCursor = messageCell.lastCursorPosition();
	cellCursor.insertText(message);
	
	table->appendRows(1);
	
	verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}