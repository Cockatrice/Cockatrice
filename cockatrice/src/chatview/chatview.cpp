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

const QColor DEFAULT_MENTION_COLOR = QColor(194, 31, 47);

UserMessagePosition::UserMessagePosition(QTextCursor &cursor)
{
    block = cursor.block();
    relativePosition = cursor.position() - block.position();
}

ChatView::ChatView(TabSupervisor *_tabSupervisor,
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

    ownUserName = userlistProxy->getOwnUsername();
    mention = "@" + ownUserName;

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

    QTextCursor cursor(document());
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
                             const QString &userName,
                             UserLevelFlags userLevel,
                             QString UserPrivLevel,
                             bool playerBold)
{
    bool atBottom = verticalScrollBar()->value() >= verticalScrollBar()->maximum();
    // messageType should be Event_RoomSay::UserMessage though we don't actually check
    bool isUserMessage = !(userName.toLower() == "servatrice" || userName.isEmpty());
    bool sameSender = isUserMessage && userName == lastSender;
    QTextCursor cursor = prepareBlock(sameSender);
    lastSender = userName;

    // timestamp
    if (showTimestamps && ((!sameSender && isUserMessage) || userName.toLower() == "servatrice")) {
        QTextCharFormat timeFormat;
        timeFormat.setForeground(serverMessageColor);
        timeFormat.setFontWeight(QFont::Bold);
        cursor.setCharFormat(timeFormat);
        cursor.insertText(QDateTime::currentDateTime().toString("[hh:mm:ss] "));
    }

    // nickname
    if (isUserMessage) {
        QTextCharFormat senderFormat;
        if (userName == ownUserName) {
            senderFormat.setForeground(QBrush(getCustomMentionColor()));
            senderFormat.setFontWeight(QFont::Bold);
        } else {
            senderFormat.setForeground(QBrush(otherUserColor));
            if (playerBold) {
                senderFormat.setFontWeight(QFont::Bold);
            }
        }
        senderFormat.setAnchor(true);
        senderFormat.setAnchorHref("user://" + QString::number(userLevel) + "_" + userName);
        if (sameSender) {
            cursor.insertText("    ");
        } else {
            const int pixelSize = QFontInfo(cursor.charFormat().font()).pixelSize();
            bool isBuddy = userlistProxy->isUserBuddy(userName);
            cursor.insertImage(
                UserLevelPixmapGenerator::generatePixmap(pixelSize, userLevel, isBuddy, UserPrivLevel).toImage());
            cursor.insertText(" ");
            cursor.setCharFormat(senderFormat);
            cursor.insertText(userName);
            cursor.insertText(": ");
            userMessagePositions[userName].append(cursor);
        }
    }

    // use different color for server messages
    defaultFormat = QTextCharFormat();
    if (!isUserMessage) {
        if (messageType == Event_RoomSay::ChatHistory) {
            defaultFormat.setForeground(Qt::gray); // FIXME : hardcoded color
            defaultFormat.setFontWeight(QFont::Light);
            defaultFormat.setFontItalic(true);
            static const QRegularExpression userNameRegex("^(\\[[^\\]]*\\]\\s)(\\S+):\\s");
            auto match = userNameRegex.match(message);
            if (match.hasMatch()) {
                cursor.setCharFormat(defaultFormat);
                UserMessagePosition pos(cursor);
                pos.relativePosition = match.captured(0).length(); // set message start
                auto before = match.captured(1);
                auto sentBy = match.captured(2);
                cursor.insertText(before); // add message timestamp
                QTextCharFormat senderFormat(defaultFormat);
                senderFormat.setAnchor(true);
                // this underscore is important, it is used to add the user level, but in this case the level is
                // unknown, if the name contains an underscore it would split up the name
                senderFormat.setAnchorHref("user://_" + sentBy);
                cursor.setCharFormat(senderFormat);
                cursor.insertText(sentBy);                   // add username with href so it shows the menu
                userMessagePositions[sentBy].append(pos);    // save message position
                message.remove(0, pos.relativePosition - 2); // do not remove semicolon
            }
        } else {
            defaultFormat.setForeground(Qt::darkGreen); // FIXME : hardcoded color
            defaultFormat.setFontWeight(QFont::Bold);
        }
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
                    checkMention(cursor, message, userName, userLevel);
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

void ChatView::checkMention(QTextCursor &cursor, QString &message, const QString &userName, UserLevelFlags userLevel)
{
    const static auto notALetterOrNumber = QRegularExpression("[^a-zA-Z0-9]");

    int firstSpace = message.indexOf(' ');
    QString fullMentionUpToSpaceOrEnd = (firstSpace == -1) ? message.mid(1) : message.mid(1, firstSpace - 1);
    QString mentionIntact = fullMentionUpToSpaceOrEnd;

    while (fullMentionUpToSpaceOrEnd.size()) {
        const ServerInfo_User *onlineUser = userlistProxy->getOnlineUser(fullMentionUpToSpaceOrEnd);
        if (onlineUser) // Is there a user online named this?
        {
            if (ownUserName.toLower() == fullMentionUpToSpaceOrEnd.toLower()) // Is this user you?
            {
                // You have received a valid mention!!
                soundEngine->playSound("chat_mention");
                mentionFormat.setBackground(QBrush(getCustomMentionColor()));
                mentionFormat.setForeground(SettingsCache::instance().getChatMentionForeground() ? QBrush(Qt::white)
                                                                                                 : QBrush(Qt::black));
                cursor.insertText(mention, mentionFormat);
                message = message.mid(mention.size());
                showSystemPopup(userName);
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
            showSystemPopup(userName);

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

void ChatView::showSystemPopup(const QString &userName)
{
    QApplication::alert(this);
    if (SettingsCache::instance().getShowMentionPopup()) {
        emit showMentionPopup(userName);
    }
}

QColor ChatView::getCustomMentionColor()
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
    QColor customColor = QColor::fromString("#" + SettingsCache::instance().getChatMentionColor());
#else
    QColor customColor;
    customColor.setNamedColor("#" + SettingsCache::instance().getChatMentionColor());
#endif
    return customColor.isValid() ? customColor : DEFAULT_MENTION_COLOR;
}

QColor ChatView::getCustomHighlightColor()
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
    QColor customColor = QColor::fromString("#" + SettingsCache::instance().getChatMentionColor());
#else
    QColor customColor;
    customColor.setNamedColor("#" + SettingsCache::instance().getChatMentionColor());
#endif
    return customColor.isValid() ? customColor : DEFAULT_MENTION_COLOR;
}

void ChatView::clearChat()
{
    document()->clear();
    lastSender = "";
}

void ChatView::redactMessages(const QString &userName, int amount)
{
    auto &messagePositions = userMessagePositions[userName];
    bool removedLastMessage = false;
    QTextCursor cursor(document());
    for (; !messagePositions.isEmpty() && amount != 0; --amount) {
        auto position = messagePositions.takeLast();   // go backwards from last message
        cursor.setPosition(position.block.position()); // move to start of block, then continue to start of message
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, position.relativePosition);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor); // select until end of block
        cursor.removeSelectedText();
        // if the cursor is at the end of the text it is possible to add text to this block still
        removedLastMessage |= cursor.atEnd();
        // we will readd this position later
    }
    if (removedLastMessage) {
        cursor.movePosition(QTextCursor::End);
        messagePositions.append(cursor);
        // note that this message might stay empty, this is not harmful as it will simply remove nothing the next time
    }
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
void ChatView::enterEvent(QEnterEvent * /*event*/)
#else
void ChatView::enterEvent(QEvent * /*event*/)
#endif
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
            if ((event->button() == Qt::MiddleButton) || (event->button() == Qt::LeftButton))
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                emit showCardInfoPopup(event->globalPosition().toPoint(), hoveredContent);
#else
                emit showCardInfoPopup(event->globalPos(), hoveredContent);
#endif
            break;
        }
        case HoveredUser: {
            if (event->button() != Qt::MiddleButton) {
                const int delimiterIndex = hoveredContent.indexOf("_");
                const QString userName = hoveredContent.mid(delimiterIndex + 1);
                switch (event->button()) {
                    case Qt::RightButton: {
                        UserLevelFlags userLevel(hoveredContent.left(delimiterIndex).toInt());
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                        userContextMenu->showContextMenu(event->globalPosition().toPoint(), userName, userLevel, this);
#else
                        userContextMenu->showContextMenu(event->globalPos(), userName, userLevel, this);
#endif
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
    if ((event->button() == Qt::MiddleButton) || (event->button() == Qt::LeftButton))
        emit deleteCardInfoPopup(QString("_"));

    QTextBrowser::mouseReleaseEvent(event);
}

void ChatView::openLink(const QUrl &link)
{
    if ((link.scheme() == "card") || (link.scheme() == "user"))
        return;

    QDesktopServices::openUrl(link);
}
