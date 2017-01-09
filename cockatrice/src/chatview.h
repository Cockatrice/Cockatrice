#ifndef CHATVIEW_H
#define CHATVIEW_H

#include <QTextBrowser>
#include <QTextFragment>
#include <QTextCursor>
#include <QColor>
#include <QAction>
#include "userlist.h"
#include "user_level.h"
#include "room_message_type.h"
#include "tab_supervisor.h"

class QTextTable;
class QMouseEvent;
class UserContextMenu;
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
    QString userName;
    QString mention;
    QTextCharFormat mentionFormat;
    QTextCharFormat highlightFormat;
    QTextCharFormat mentionFormatOtherUser;
    QTextCharFormat defaultFormat;
    QStringList highlightedWords;
    bool evenNumber;
    bool showTimestamps;
    HoveredItemType hoveredItemType;
    QString hoveredContent;
    QAction *messageClicked;
    QTextFragment getFragmentUnderMouse(const QPoint &pos) const;
    QTextCursor prepareBlock(bool same = false);
    void appendCardTag(QTextCursor &cursor, const QString &cardName);
    void appendUrlTag(QTextCursor &cursor, QString url);
    QString getNameFromUserList(QMap<QString, UserListTWI *> &userList, QString &userName);
    bool isFullMentionAValidUser(QMap<QString, UserListTWI *> &userList, QString userNameToMatch);
    QColor getCustomMentionColor();
    QColor getCustomHighlightColor();
    bool shouldShowSystemPopup();
    void showSystemPopup(QString &sender);
    bool isModeratorSendingGlobal(QFlags<ServerInfo_User::UserLevelFlag> userLevelFlag, QString message);
    void checkTag(QTextCursor &cursor, QString &message);
    void checkMention(QTextCursor &cursor, QString &message, QString &sender, UserLevelFlags userLevel);
    void checkWord(QTextCursor &cursor, QString &message);
    QString extractNextWord(QString &message, QString &rest);
private slots:
    void openLink(const QUrl &link);
    void actMessageClicked();
public:
    ChatView(const TabSupervisor *_tabSupervisor, TabGame *_game, bool _showTimestamps, QWidget *parent = 0);
    void retranslateUi();
    void appendHtml(const QString &html);
    void appendHtmlServerMessage(const QString &html, bool optionalIsBold = false, QString optionalFontColor = QString());
    void appendMessage(QString message, RoomMessageTypeFlags messageType = 0, QString sender = QString(), UserLevelFlags userLevel = UserLevelFlags(), QString UserPrivLevel = "NONE", bool playerBold = false);
    void clearChat();
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
    void addMentionTag(QString mentionTag);
    void messageClickedSignal();
    void showMentionPopup(QString &sender);
};

#endif
