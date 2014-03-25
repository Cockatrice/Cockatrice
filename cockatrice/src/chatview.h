#ifndef CHATVIEW_H
#define CHATVIEW_H

#include <QTextBrowser>
#include <QTextFragment>
#include <QTextCursor>
#include <QColor>
#include "user_level.h"

class QTextTable;
class QMouseEvent;
class UserContextMenu;
class TabSupervisor;
class TabGame;

class ChatView : public QTextBrowser {
    Q_OBJECT
protected:
    const TabSupervisor * const tabSupervisor;
    TabGame * const game;
private:
    enum HoveredItemType { HoveredNothing, HoveredUrl, HoveredCard, HoveredUser };
    UserContextMenu *userContextMenu;
    QString lastSender;
    bool evenNumber;
    bool showTimestamps;
    HoveredItemType hoveredItemType;
    QString hoveredContent;
    QTextFragment getFragmentUnderMouse(const QPoint &pos) const;
    QTextCursor prepareBlock(bool same = false);
    void appendCardTag(QTextCursor &cursor, const QString &cardName);
    void appendUrlTag(QTextCursor &cursor, QString url);
private slots:
    void openLink(const QUrl &link);
public:
    ChatView(const TabSupervisor *_tabSupervisor, TabGame *_game, bool _showTimestamps, QWidget *parent = 0);
    void retranslateUi();
    void appendHtml(const QString &html);
    void appendMessage(QString message, QString sender = QString(), UserLevelFlags userLevel = UserLevelFlags(), bool playerBold = false);
protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
signals:
    void openMessageDialog(const QString &userName, bool focus);
    void cardNameHovered(QString cardName);
    void showCardInfoPopup(QPoint pos, QString cardName);
    void deleteCardInfoPopup(QString cardName);
};

#endif
