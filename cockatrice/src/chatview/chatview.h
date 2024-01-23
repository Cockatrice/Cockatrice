#ifndef CHATVIEW_H
#define CHATVIEW_H

#include "../tab_supervisor.h"
#include "../userlist.h"
#include "room_message_type.h"
#include "user_level.h"
#include "userlistProxy.h"

#include <QAction>
#include <QColor>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextFragment>

class QTextTable;
class QMouseEvent;
class UserContextMenu;
class TabGame;

class UserMessagePosition
{
public:
#if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
    UserMessagePosition() = default; // older qt versions require a default constructor to use in containers
#endif
    UserMessagePosition(QTextCursor &cursor);
    int relativePosition;
    QTextBlock block;
};

class ChatView : public QTextBrowser
{
    Q_OBJECT
protected:
    TabSupervisor *const tabSupervisor;
    TabGame *const game;

private:
    enum HoveredItemType
    {
        HoveredNothing,
        HoveredUrl,
        HoveredCard,
        HoveredUser
    };
    const UserlistProxy *const userlistProxy;
    UserContextMenu *userContextMenu;
    QString lastSender;
    QString ownUserName;
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
    QMap<QString, QVector<UserMessagePosition>> userMessagePositions;

    QTextFragment getFragmentUnderMouse(const QPoint &pos) const;
    QTextCursor prepareBlock(bool same = false);
    void appendCardTag(QTextCursor &cursor, const QString &cardName);
    void appendUrlTag(QTextCursor &cursor, QString url);
    static QColor getCustomMentionColor();
    static QColor getCustomHighlightColor();
    void showSystemPopup(const QString &userName);
    bool isModeratorSendingGlobal(QFlags<ServerInfo_User::UserLevelFlag> userLevelFlag, QString message);
    void checkTag(QTextCursor &cursor, QString &message);
    void checkMention(QTextCursor &cursor, QString &message, const QString &userName, UserLevelFlags userLevel);
    void checkWord(QTextCursor &cursor, QString &message);
    QString extractNextWord(QString &message, QString &rest);

    QColor otherUserColor = QColor(0, 65, 255); // dark blue
    QColor serverMessageColor = QColor(0x85, 0x15, 0x15);
    QColor linkColor;

private slots:
    void openLink(const QUrl &link);
    void actMessageClicked();

public:
    ChatView(TabSupervisor *_tabSupervisor,
             const UserlistProxy *_userlistProxy,
             TabGame *_game,
             bool _showTimestamps,
             QWidget *parent = nullptr);
    void retranslateUi();
    void appendHtml(const QString &html);
    void virtual appendHtmlServerMessage(const QString &html,
                                         bool optionalIsBold = false,
                                         QString optionalFontColor = QString());
    void appendMessage(QString message,
                       RoomMessageTypeFlags messageType = {},
                       const QString &userName = QString(),
                       UserLevelFlags userLevel = UserLevelFlags(),
                       QString UserPrivLevel = "NONE",
                       bool playerBold = false);
    void clearChat();
    void redactMessages(const QString &userName, int amount);

protected:
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    void enterEvent(QEnterEvent *event);
#else
    void enterEvent(QEvent *event);
#endif
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
    void showMentionPopup(const QString &userName);
};

#endif
