#include "chatview.h"

#include "../pixmapgenerator.h"
#include "../settingscache.h"
#include "../soundengine.h"
#include "../tab_account.h"
#include "../user_context_menu.h"
#include "user_level.h"

#include <QApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTextEdit>

const QColor DEFAULT_MENTION_COLOR = QColor(194, 31, 47);

ChatView::ChatView(const TabSupervisor *_tabSupervisor,
                   const UserlistProxy *_userlistProxy,
                   TabGame *_game,
                   bool _showTimestamps,
                   QWidget *parent)
    : QTextBrowser(parent), tabSupervisor(_tabSupervisor), game(_game), userlistProxy(_userlistProxy), evenNumber(true),
      showTimestamps(_showTimestamps), hoveredItemType(HoveredNothing)
{
    if (palette().windowText().color().lightness() > 200) {
        document()->setDefaultStyleSheet(R"(
           a { text-decoration: none; color: rgb(71,158,252); }
           .blue { color: rgb(71,158,252); }
        )");
        serverMessageColor = QColor(0xFF, 0x73, 0x83);
        otherUserColor = otherUserColor.lighter(150);
        linkColor = QColor(71, 158, 252);
    } else {
        document()->setDefaultStyleSheet(R"(
            a { text-decoration: none; color: blue; }
            .blue { color: blue }
        )");
        linkColor = palette().link().color();
    }

    userContextMenu = new UserContextMenu(tabSupervisor, this, game);
    connect(userContextMenu, SIGNAL(openMessageDialog(QString, bool)), this, SIGNAL(openMessageDialog(QString, bool)));

    userName = userlistProxy->getOwnUsername();
    mention = "@" + userName;

    mentionFormat.setFontWeight(QFont::Bold);

    mentionFormatOtherUser.setFontWeight(QFont::Bold);
    mentionFormatOtherUser.setForeground(linkColor);
    mentionFormatOtherUser.setAnchor(true);

    viewport()->setCursor(Qt::IBeamCursor);
    setReadOnly(true);
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    setOpenLinks(false);
    connect(this, SIGNAL(anchorClicked(const QUrl &)), this, SLOT(openLink(const QUrl &)));
}

void ChatView::retranslateUi()
{
    userContextMenu->retranslateUi();
}

QTextCursor ChatView::prepareBlock(bool same)
{
    lastSender.clear();

    QTextCursor cursor(document()->lastBlock());
    cursor.movePosition(QTextCursor::End);
    if (same) {
        cursor.insertHtml("<br>");
    } else {
        QTextBlockFormat blockFormat;
        if ((evenNumber = !evenNumber))
            blockFormat.setBackground(palette().window());
        else
            blockFormat.setBackground(palette().base());

        blockFormat.setForeground(palette().text());
        blockFormat.setBottomMargin(4);
        cursor.insertBlock(blockFormat);
    }

    return cursor;
}

void ChatView::appendHtml(const QString &html)
{
    bool atBottom = verticalScrollBar()->value() >= verticalScrollBar()->maximum();
    prepareBlock().insertHtml(html);
    if (atBottom)
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void ChatView::appendHtmlServerMessage(const QString &html, bool optionalIsBold, QString optionalFontColor)
{
    bool atBottom = verticalScrollBar()->value() >= verticalScrollBar()->maximum();

    QString htmlText =
        "<font color=" + ((optionalFontColor.size() > 0) ? optionalFontColor : serverMessageColor.name()) + ">" +
        QDateTime::currentDateTime().toString("[hh:mm:ss] ") + html + "</font>";

    if (optionalIsBold)
        htmlText = "<b>" + htmlText + "</b>";

    prepareBlock().insertHtml(htmlText);
    if (atBottom)
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void ChatView::appendCardTag(QTextCursor &cursor, const QString &cardName)
{
    QTextCharFormat oldFormat = cursor.charFormat();
    QTextCharFormat anchorFormat = oldFormat;
    anchorFormat.setForeground(linkColor);
    anchorFormat.setAnchor(true);
    anchorFormat.setAnchorHref("card://" + cardName);
    anchorFormat.setFontItalic(true);

    cursor.setCharFormat(anchorFormat);
    cursor.insertText(cardName);
    cursor.setCharFormat(oldFormat);
}

void ChatView::appendUrlTag(QTextCursor &cursor, QString url)
{
    if (!url.contains("://"))
        url.prepend("https://");

    QTextCharFormat oldFormat = cursor.charFormat();
    QTextCharFormat anchorFormat = oldFormat;
    anchorFormat.setForeground(linkColor);
    anchorFormat.setAnchor(true);
    anchorFormat.setAnchorHref(url);
    anchorFormat.setUnderlineColor(linkColor);
    anchorFormat.setFontUnderline(true);

    cursor.setCharFormat(anchorFormat);
    cursor.insertText(url);
    cursor.setCharFormat(oldFormat);
}

void ChatView::appendMessage(QString message,
                             RoomMessageTypeFlags messageType,
                             QString sender,
                             UserLevelFlags userLevel,
                             QString UserPrivLevel,
                             bool playerBold)
{
    bool atBottom = verticalScrollBar()->value() >= verticalScrollBar()->maximum();
    bool sameSender = (sender == lastSender) && !lastSender.isEmpty();
    QTextCursor cursor = prepareBlock(sameSender);
    lastSender = sender;

    // timestamp
    if (showTimestamps && (!sameSender || sender.toLower() == "servatrice") && !sender.isEmpty()) {
        QTextCharFormat timeFormat;
        timeFormat.setForeground(serverMessageColor);
        if (sender.isEmpty())
            timeFormat.setFontWeight(QFont::Bold);
        cursor.setCharFormat(timeFormat);
        cursor.insertText(QDateTime::currentDateTime().toString("[hh:mm:ss] "));
    }

    // nickname
    if (sender.toLower() != "servatrice") {
        QTextCharFormat senderFormat;
        if (sender == userName) {
            senderFormat.setForeground(QBrush(getCustomMentionColor()));
            senderFormat.setFontWeight(QFont::Bold);
        } else {
            senderFormat.setForeground(QBrush(otherUserColor));
            if (playerBold)
                senderFormat.setFontWeight(QFont::Bold);
        }
        senderFormat.setAnchor(true);
        senderFormat.setAnchorHref("user://" + QString::number(userLevel) + "_" + sender);
        if (sameSender) {
            cursor.insertText("    ");
        } else {
            if (!sender.isEmpty()) {
                const int pixelSize = QFontInfo(cursor.charFormat().font()).pixelSize();
                bool isBuddy = userlistProxy->isUserBuddy(sender);
                cursor.insertImage(
                    UserLevelPixmapGenerator::generatePixmap(pixelSize, userLevel, isBuddy, UserPrivLevel).toImage());
                cursor.insertText(" ");
            }
            cursor.setCharFormat(senderFormat);
            if (!sender.isEmpty())
                sender.append(": ");
            cursor.insertText(sender);
        }
    }

    // use different color for server messages
    defaultFormat = QTextCharFormat();
    if (sender.isEmpty()) {
        switch (messageType) {
            case Event_RoomSay::Welcome:
                defaultFormat.setForeground(Qt::darkGreen);
                defaultFormat.setFontWeight(QFont::Bold);
                break;
            case Event_RoomSay::ChatHistory:
                defaultFormat.setForeground(Qt::gray);
                defaultFormat.setFontWeight(QFont::Light);
                defaultFormat.setFontItalic(true);
                break;
            default:
                defaultFormat.setForeground(Qt::darkGreen);
                defaultFormat.setFontWeight(QFont::Bold);
        }
    } else if (sender.toLower() == "servatrice") {
        defaultFormat.setForeground(Qt::darkGreen);
        defaultFormat.setFontWeight(QFont::Bold);
    }
    cursor.setCharFormat(defaultFormat);

    bool mentionEnabled = SettingsCache::instance().getChatMention();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    highlightedWords = SettingsCache::instance().getHighlightWords().split(' ', Qt::SkipEmptyParts);
#else
    highlightedWords = SettingsCache::instance().getHighlightWords().split(' ', QString::SkipEmptyParts);
#endif

    // parse the message
    while (message.size()) {
        QChar c = message.at(0);
        switch (c.toLatin1()) {
            case '[':
                checkTag(cursor, message);
                break;
            case '@':
                if (mentionEnabled) {
                    checkMention(cursor, message, sender, userLevel);
                } else {
                    cursor.insertText(c, defaultFormat);
                    message = message.mid(1);
                }
                break;
            case ' ':
                cursor.insertText(c, defaultFormat);
                message = message.mid(1);
                break;
            default:
                if (c.isLetterOrNumber()) {
                    checkWord(cursor, message);
                } else {
                    cursor.insertText(c, defaultFormat);
                    message = message.mid(1);
                }
                break;
        }
    }

    if (atBottom)
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void ChatView::checkTag(QTextCursor &cursor, QString &message)
{
    if (message.startsWith("[card]")) {
        message = message.mid(6);
        int closeTagIndex = message.indexOf("[/card]");
        QString cardName = message.left(closeTagIndex);
        if (closeTagIndex == -1)
            message.clear();
        else
            message = message.mid(closeTagIndex + 7);

        appendCardTag(cursor, cardName);
        return;
    }

    if (message.startsWith("[[")) {
        message = message.mid(2);
        int closeTagIndex = message.indexOf("]]");
        QString cardName = message.left(closeTagIndex);
        if (closeTagIndex == -1)
            message.clear();
        else
            message = message.mid(closeTagIndex + 2);

        appendCardTag(cursor, cardName);
        return;
    }

    if (message.startsWith("[url]")) {
        message = message.mid(5);
        int closeTagIndex = message.indexOf("[/url]");
        QString url = message.left(closeTagIndex);
        if (closeTagIndex == -1)
            message.clear();
        else
            message = message.mid(closeTagIndex + 6);

        appendUrlTag(cursor, url);
        return;
    }

    // no valid tag found
    checkWord(cursor, message);
}

void ChatView::checkMention(QTextCursor &cursor, QString &message, QString &sender, UserLevelFlags userLevel)
{
    const QRegExp notALetterOrNumber = QRegExp("[^a-zA-Z0-9]");

    int firstSpace = message.indexOf(' ');
    QString fullMentionUpToSpaceOrEnd = (firstSpace == -1) ? message.mid(1) : message.mid(1, firstSpace - 1);
    QString mentionIntact = fullMentionUpToSpaceOrEnd;

    while (fullMentionUpToSpaceOrEnd.size()) {
        const ServerInfo_User *onlineUser = userlistProxy->getOnlineUser(fullMentionUpToSpaceOrEnd);
        if (onlineUser) // Is there a user online named this?
        {
            if (userName.toLower() == fullMentionUpToSpaceOrEnd.toLower()) // Is this user you?
            {
                // You have received a valid mention!!
                soundEngine->playSound("chat_mention");
                mentionFormat.setBackground(QBrush(getCustomMentionColor()));
                mentionFormat.setForeground(SettingsCache::instance().getChatMentionForeground() ? QBrush(Qt::white)
                                                                                                 : QBrush(Qt::black));
                cursor.insertText(mention, mentionFormat);
                message = message.mid(mention.size());
                showSystemPopup(sender);
            } else {
                QString correctUserName = QString::fromStdString(onlineUser->name());
                mentionFormatOtherUser.setAnchorHref("user://" + QString::number(onlineUser->user_level()) + "_" +
                                                     correctUserName);
                cursor.insertText("@" + correctUserName, mentionFormatOtherUser);

                message = message.mid(correctUserName.size() + 1);
            }

            cursor.setCharFormat(defaultFormat);
            return;
        }

        if (isModeratorSendingGlobal(userLevel, fullMentionUpToSpaceOrEnd)) {
            // Moderator Sending Global Message
            soundEngine->playSound("all_mention");
            mentionFormat.setBackground(QBrush(getCustomMentionColor()));
            mentionFormat.setForeground(SettingsCache::instance().getChatMentionForeground() ? QBrush(Qt::white)
                                                                                             : QBrush(Qt::black));
            cursor.insertText("@" + fullMentionUpToSpaceOrEnd, mentionFormat);
            message = message.mid(fullMentionUpToSpaceOrEnd.size() + 1);
            showSystemPopup(sender);

            cursor.setCharFormat(defaultFormat);
            return;
        }

        if (fullMentionUpToSpaceOrEnd.right(1).indexOf(notALetterOrNumber) == -1 ||
            fullMentionUpToSpaceOrEnd.size() < 2) {
            cursor.insertText("@" + mentionIntact, defaultFormat);
            message = message.mid(mentionIntact.size() + 1);
            cursor.setCharFormat(defaultFormat);
            return;
        }

        fullMentionUpToSpaceOrEnd.chop(1);
    }

    // no valid mention found
    checkWord(cursor, message);
}

void ChatView::checkWord(QTextCursor &cursor, QString &message)
{
    // extract the first word
    QString rest;
    QString fullWordUpToSpaceOrEnd = extractNextWord(message, rest);

    // check urls
    if (fullWordUpToSpaceOrEnd.startsWith("http://", Qt::CaseInsensitive) ||
        fullWordUpToSpaceOrEnd.startsWith("https://", Qt::CaseInsensitive) ||
        fullWordUpToSpaceOrEnd.startsWith("www.", Qt::CaseInsensitive)) {
        QUrl qUrl(fullWordUpToSpaceOrEnd);
        if (qUrl.isValid()) {
            appendUrlTag(cursor, fullWordUpToSpaceOrEnd);
            cursor.insertText(rest, defaultFormat);
            return;
        }
    }

    // check word mentions
    foreach (QString word, highlightedWords) {
        if (fullWordUpToSpaceOrEnd.compare(word, Qt::CaseInsensitive) == 0) {
            // You have received a valid mention of custom word!!
            highlightFormat.setBackground(QBrush(getCustomHighlightColor()));
            highlightFormat.setForeground(SettingsCache::instance().getChatHighlightForeground() ? QBrush(Qt::white)
                                                                                                 : QBrush(Qt::black));
            cursor.insertText(fullWordUpToSpaceOrEnd, highlightFormat);
            cursor.insertText(rest, defaultFormat);
            QApplication::alert(this);
            return;
        }
    }

    // not a special word; just print it
    cursor.insertText(fullWordUpToSpaceOrEnd + rest, defaultFormat);
}

QString ChatView::extractNextWord(QString &message, QString &rest)
{
    // get the first next space and extract the word
    QString word;
    int firstSpace = message.indexOf(' ');
    if (firstSpace == -1) {
        word = message;
        message.clear();
    } else {
        word = message.mid(0, firstSpace);
        message = message.mid(firstSpace);
    }

    // remove any punctuation from the end and pass it separately
    for (int len = word.size() - 1; len >= 0; --len) {
        if (word.at(len).isLetterOrNumber()) {
            rest = word.mid(len + 1);
            return word.mid(0, len + 1);
        }
    }

    rest = word;
    return QString();
}

bool ChatView::isModeratorSendingGlobal(QFlags<ServerInfo_User::UserLevelFlag> userLevelFlag, QString message)
{
    int userLevel = QString::number(userLevelFlag).toInt();

    QStringList getAttentionList;
    getAttentionList << "/all"; // Send a message to all users

    return (getAttentionList.contains(message) &&
            (userLevel & ServerInfo_User::IsModerator || userLevel & ServerInfo_User::IsAdmin));
}

void ChatView::actMessageClicked()
{
    emit messageClickedSignal();
}

void ChatView::showSystemPopup(QString &sender)
{
    QApplication::alert(this);
    if (SettingsCache::instance().getShowMentionPopup()) {
        QString ref = sender.left(sender.length() - 2);
        emit showMentionPopup(ref);
    }
}

QColor ChatView::getCustomMentionColor()
{
    QColor customColor;
    customColor.setNamedColor("#" + SettingsCache::instance().getChatMentionColor());
    return customColor.isValid() ? customColor : DEFAULT_MENTION_COLOR;
}

QColor ChatView::getCustomHighlightColor()
{
    QColor customColor;
    customColor.setNamedColor("#" + SettingsCache::instance().getChatHighlightColor());
    return customColor.isValid() ? customColor : DEFAULT_MENTION_COLOR;
}

void ChatView::clearChat()
{
    document()->clear();
    lastSender = "";
}

void ChatView::enterEvent(QEvent * /*event*/)
{
    setMouseTracking(true);
}

void ChatView::leaveEvent(QEvent * /*event*/)
{
    setMouseTracking(false);
}

QTextFragment ChatView::getFragmentUnderMouse(const QPoint &pos) const
{
    QTextCursor cursor(cursorForPosition(pos));
    QTextBlock block(cursor.block());
    QTextBlock::iterator it;
    for (it = block.begin(); !(it.atEnd()); ++it) {
        QTextFragment frag = it.fragment();
        if (frag.contains(cursor.position()))
            return frag;
    }
    return QTextFragment();
}

void ChatView::mouseMoveEvent(QMouseEvent *event)
{
    QString anchorHref = getFragmentUnderMouse(event->pos()).charFormat().anchorHref();
    if (!anchorHref.isEmpty()) {
        const int delimiterIndex = anchorHref.indexOf("://");
        if (delimiterIndex != -1) {
            const QString scheme = anchorHref.left(delimiterIndex);
            hoveredContent = anchorHref.mid(delimiterIndex + 3);
            if (scheme == "card") {
                hoveredItemType = HoveredCard;
                emit cardNameHovered(hoveredContent);
            } else if (scheme == "user")
                hoveredItemType = HoveredUser;
            else
                hoveredItemType = HoveredUrl;
            viewport()->setCursor(Qt::PointingHandCursor);
        } else {
            hoveredItemType = HoveredNothing;
            viewport()->setCursor(Qt::IBeamCursor);
        }
    } else {
        hoveredItemType = HoveredNothing;
        viewport()->setCursor(Qt::IBeamCursor);
    }

    QTextBrowser::mouseMoveEvent(event);
}

void ChatView::mousePressEvent(QMouseEvent *event)
{
    switch (hoveredItemType) {
        case HoveredCard: {
            if ((event->button() == Qt::MidButton) || (event->button() == Qt::LeftButton))
                emit showCardInfoPopup(event->globalPos(), hoveredContent);
            break;
        }
        case HoveredUser: {
            if (event->button() != Qt::MidButton) {
                const int delimiterIndex = hoveredContent.indexOf("_");
                const QString userName = hoveredContent.mid(delimiterIndex + 1);
                switch (event->button()) {
                    case Qt::RightButton: {
                        UserLevelFlags userLevel(hoveredContent.left(delimiterIndex).toInt());
                        userContextMenu->showContextMenu(event->globalPos(), userName, userLevel);
                        break;
                    }
                    case Qt::LeftButton: {
                        if (event->modifiers() == Qt::ControlModifier) {
                            emit openMessageDialog(userName, true);
                        } else
                            emit addMentionTag("@" + userName);
                        break;
                    }
                    default:
                        break;
                }
            }
            break;
        }
        default: {
            QTextBrowser::mousePressEvent(event);
        }
    }
}

void ChatView::mouseReleaseEvent(QMouseEvent *event)
{
    if ((event->button() == Qt::MidButton) || (event->button() == Qt::LeftButton))
        emit deleteCardInfoPopup(QString("_"));

    QTextBrowser::mouseReleaseEvent(event);
}

void ChatView::openLink(const QUrl &link)
{
    if ((link.scheme() == "card") || (link.scheme() == "user"))
        return;

    QDesktopServices::openUrl(link);
}
