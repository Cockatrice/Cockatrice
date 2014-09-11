#include <QTextEdit>
#include <QDateTime>
#include <QScrollBar>
#include <QMouseEvent>
#include <QDesktopServices>
#include "chatview.h"
#include "user_level.h"
#include "user_context_menu.h"
#include "tab_supervisor.h"
#include "pixmapgenerator.h"

ChatView::ChatView(const TabSupervisor *_tabSupervisor, TabGame *_game, bool _showTimestamps, QWidget *parent)
    : QTextBrowser(parent), tabSupervisor(_tabSupervisor), game(_game), evenNumber(true), showTimestamps(_showTimestamps), hoveredItemType(HoveredNothing)
{
    document()->setDefaultStyleSheet("a { text-decoration: none; color: blue; }");
    userContextMenu = new UserContextMenu(tabSupervisor, this, game);
    connect(userContextMenu, SIGNAL(openMessageDialog(QString, bool)), this, SIGNAL(openMessageDialog(QString, bool)));
    
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
    if (!same) {
        QTextBlockFormat blockFormat;
        if ((evenNumber = !evenNumber))
            blockFormat.setBackground(palette().alternateBase());
        blockFormat.setBottomMargin(2);
        cursor.insertBlock(blockFormat);
    } else
        cursor.insertHtml("<br>");
    
    return cursor;
}

void ChatView::appendHtml(const QString &html)
{
    bool atBottom = verticalScrollBar()->value() >= verticalScrollBar()->maximum();
    prepareBlock().insertHtml(html);
    if (atBottom)
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void ChatView::appendCardTag(QTextCursor &cursor, const QString &cardName)
{
    QTextCharFormat oldFormat = cursor.charFormat();
    QTextCharFormat anchorFormat = oldFormat;
    anchorFormat.setForeground(Qt::blue);
    anchorFormat.setAnchor(true);
    anchorFormat.setAnchorHref("card://" + cardName);
    
    cursor.setCharFormat(anchorFormat);
    cursor.insertText(cardName);
    cursor.setCharFormat(oldFormat);
}

void ChatView::appendUrlTag(QTextCursor &cursor, QString url)
{
    if (!url.contains("://"))
        url.prepend("http://");
    
    QTextCharFormat oldFormat = cursor.charFormat();
    QTextCharFormat anchorFormat = oldFormat;
    anchorFormat.setForeground(Qt::blue);
    anchorFormat.setAnchor(true);
    anchorFormat.setAnchorHref(url);
    
    cursor.setCharFormat(anchorFormat);
    cursor.insertText(url);
    cursor.setCharFormat(oldFormat);
}

void ChatView::appendMessage(QString message, QString sender, UserLevelFlags userLevel, bool playerBold)
{
    bool atBottom = verticalScrollBar()->value() >= verticalScrollBar()->maximum();
    bool sameSender = (sender == lastSender) && !lastSender.isEmpty();
    QTextCursor cursor = prepareBlock(sameSender);
    lastSender = sender;
    
    if (showTimestamps && !sameSender) {
        QTextCharFormat timeFormat;
        timeFormat.setForeground(Qt::black);
        cursor.setCharFormat(timeFormat);
        cursor.insertText(QDateTime::currentDateTime().toString("[hh:mm] "));
    }
    
    QTextCharFormat senderFormat;
    if (tabSupervisor && tabSupervisor->getUserInfo() && (sender == QString::fromStdString(tabSupervisor->getUserInfo()->name()))) {
        senderFormat.setFontWeight(QFont::Bold);
        senderFormat.setForeground(Qt::red);
    } else {
        senderFormat.setForeground(Qt::blue);
        if (playerBold)
            senderFormat.setFontWeight(QFont::Bold);
    }
    senderFormat.setAnchor(true);
    senderFormat.setAnchorHref("user://" + QString::number(userLevel) + "_" + sender);
    if (!sameSender) {
        if (!sender.isEmpty()) {
            const int pixelSize = QFontInfo(cursor.charFormat().font()).pixelSize();
            cursor.insertImage(UserLevelPixmapGenerator::generatePixmap(pixelSize, userLevel).toImage(), QString::number(pixelSize) + "_" + QString::number((int) userLevel));
            cursor.insertText(" ");
        }
        cursor.setCharFormat(senderFormat);
        if (!sender.isEmpty())
            sender.append(": ");
        cursor.insertText(sender);
    } else
        cursor.insertText("    ");
    
    QTextCharFormat messageFormat;
    if (sender.isEmpty())
        messageFormat.setForeground(Qt::darkGreen);
    cursor.setCharFormat(messageFormat);
    
    int from = 0, index = 0;
    while ((index = message.indexOf('[', from)) != -1) {
        cursor.insertText(message.left(index));
        message = message.mid(index);
        if (message.isEmpty())
            break;

        if (message.startsWith("[card]")) {
            message = message.mid(6);
            int closeTagIndex = message.indexOf("[/card]");
            QString cardName = message.left(closeTagIndex);
            if (closeTagIndex == -1)
                message.clear();
            else
                message = message.mid(closeTagIndex + 7);
            
            appendCardTag(cursor, cardName);
        } else if (message.startsWith("[[")) {
            message = message.mid(2);
            int closeTagIndex = message.indexOf("]]");
            QString cardName = message.left(closeTagIndex);
            if (closeTagIndex == -1)
                message.clear();
            else
                message = message.mid(closeTagIndex + 2);
            
            appendCardTag(cursor, cardName);
        } else if (message.startsWith("[url]")) {
            message = message.mid(5);
            int closeTagIndex = message.indexOf("[/url]");
            QString url = message.left(closeTagIndex);
            if (closeTagIndex == -1)
                message.clear();
            else
                message = message.mid(closeTagIndex + 6);
            
            appendUrlTag(cursor, url);
        } else
            from = 1;
    }
    if (!message.isEmpty())
        cursor.insertText(message);
    
    if (atBottom)
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
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
            if (event->button() == Qt::RightButton) {
                const int delimiterIndex = hoveredContent.indexOf("_");
                UserLevelFlags userLevel(hoveredContent.left(delimiterIndex).toInt());
                const QString userName = hoveredContent.mid(delimiterIndex + 1);
                
                userContextMenu->showContextMenu(event->globalPos(), userName, userLevel);
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
