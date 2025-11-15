/**
 * @file chat_view.h
 * @ingroup NetworkingWidgets
 * @ingroup Lobby
 * @brief TODO: Document this.
 */

#ifndef CHATVIEW_H
#define CHATVIEW_H

#include "../../interface/widgets/tabs/tab_supervisor.h"
#include "../user/user_list_widget.h"

#include <QAction>
#include <QColor>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextFragment>
#include <libcockatrice/network/server/remote/room_message_type.h>
#include <libcockatrice/network/server/remote/user_level.h>

class AbstractGame;
class QTextTable;
class QMouseEvent;
class UserContextMenu;
class UserListProxy;

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
    AbstractGame *const game;

private:
    enum HoveredItemType
    {
        HoveredNothing,
        HoveredUrl,
        HoveredCard,
        HoveredUser
    };
    const UserListProxy *const userListProxy;
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

    [[nodiscard]] QTextFragment getFragmentUnderMouse(const QPoint &pos) const;
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
    ChatView(TabSupervisor *_tabSupervisor, AbstractGame *_game, bool _showTimestamps, QWidget *parent = nullptr);
    void retranslateUi();
    void appendHtml(const QString &html);
    void virtual appendHtmlServerMessage(const QString &html,
                                         bool optionalIsBold = false,
                                         QString optionalFontColor = QString());
    void appendMessage(QString message,
                       RoomMessageTypeFlags messageType = {},
                       const ServerInfo_User &userInfo = {},
                       bool playerBold = false);
    void clearChat();
    void redactMessages(const QString &userName, int amount);

protected:
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
signals:
    void openMessageDialog(const QString &userName, bool focus);
    void cardNameHovered(QString cardName);
    void showCardInfoPopup(const QPoint &pos, const CardRef &cardRef);
    void deleteCardInfoPopup(QString cardName);
    void addMentionTag(QString mentionTag);
    void messageClickedSignal();
    void showMentionPopup(const QString &userName);
};

#endif
