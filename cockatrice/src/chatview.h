#ifndef CHATVIEW_H
#define CHATVIEW_H

#include <QTextBrowser>
#include <QTextFragment>
#include <QTextCursor>
#include <QColor>

class QTextTable;
class QMouseEvent;

class ChatView : public QTextBrowser {
	Q_OBJECT;
private:
	bool evenNumber;
	QString ownName;
	bool showTimestamps;
	QTextFragment getFragmentUnderMouse(const QPoint &pos) const;
	QString getCardNameUnderMouse(QTextFragment frag) const;
	QString getCardNameUnderMouse(const QPoint &pos) const;
	QTextCursor prepareBlock();
private slots:
	void openLink(const QUrl &link);
public:
	ChatView(const QString &_ownName, bool _showTimestamps, QWidget *parent = 0);
	void appendHtml(const QString &html);
	void appendMessage(QString sender, QString message, QColor playerColor = QColor(), bool playerBold = false);
protected:
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
signals:
	void cardNameHovered(QString cardName);
	void showCardInfoPopup(QPoint pos, QString cardName);
	void deleteCardInfoPopup(QString cardName);
};

#endif