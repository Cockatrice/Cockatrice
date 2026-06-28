#include "user_list_painter.h"

#include "../../interface/pixel_map_generator.h"

#include <QAbstractScrollArea>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QStyle>
#include <QStyleOptionViewItem>

static constexpr int RowHeight = 72;
static constexpr int AvatarSize = 36;
static constexpr int LeftPadding = 14;
static constexpr int TextSpacing = 10;

QSize UserListPainter::sizeHint()
{
    return QSize(0, RowHeight);
}

QColor UserListPainter::getAccentColor(const UserLevelFlags &userLevel, bool online)
{
    QColor accentColor;

    if (userLevel.testFlag(ServerInfo_User::IsAdmin)) {
        accentColor = QColor(245, 158, 11);
    } else if (userLevel.testFlag(ServerInfo_User::IsModerator)) {
        accentColor = QColor(59, 130, 246);
    } else if (userLevel.testFlag(ServerInfo_User::IsJudge)) {
        accentColor = QColor(168, 85, 247);
    } else {
        accentColor = QColor(100, 116, 139);
    }

    if (!online) {
        accentColor = accentColor.darker(160);
    }

    return accentColor;
}

int UserListPainter::getCardRight(const QStyleOptionViewItem &option, const QRect &rect)
{
    int scrollBarWidth = 0;

    if (const auto *scrollArea = qobject_cast<const QAbstractScrollArea *>(option.widget)) {
        const QScrollBar *sb = scrollArea->verticalScrollBar();
        if (sb && sb->isVisible()) {
            scrollBarWidth = sb->width();
        }
    }

    const int viewportRight = option.widget ? option.widget->width() - scrollBarWidth : rect.right();

    return qMin(rect.right(), viewportRight - 4);
}

void UserListPainter::drawBackground(QPainter *painter,
                                     const QRectF &cardRect,
                                     const QColor &accentColor,
                                     bool selected)
{
    QLinearGradient bg(cardRect.topLeft(), cardRect.topRight());
    bg.setColorAt(0, selected ? accentColor.darker(130) : accentColor.darker(320));
    bg.setColorAt(1, selected ? QColor(40, 48, 60) : QColor(18, 22, 30));

    painter->setPen(Qt::NoPen);
    painter->setBrush(bg);
    painter->drawRoundedRect(cardRect, 6, 6);

    painter->setBrush(accentColor);
    painter->drawRoundedRect(QRectF(cardRect.left(), cardRect.top(), 3, cardRect.height()), 2, 2);
}

static QString makeKey(const QString &user, const QString &card, const QString &providerId)
{
    return user + u'|' + card + u'|' + providerId;
}

void UserListPainter::drawCardArt(QPainter *painter,
                                  const QRect &rect,
                                  int cardRight,
                                  const QString &userName,
                                  const QMap<QString, QPixmap> *cardArtCache,
                                  const CardArtParams &params,
                                  const QPixmap *overridePixmap = nullptr)
{
    QPixmap art;

    if (overridePixmap && !overridePixmap->isNull()) {
        art = *overridePixmap;
    } else {
        if (!cardArtCache) {
            return;
        }

        const QString key = makeKey(userName, params.cardName, params.cardProviderId);

        if (!cardArtCache->contains(key)) {
            return;
        }

        art = cardArtCache->value(key);
    }

    if (art.isNull()) {
        return;
    }

    const int cardH = rect.height() - 4;
    const int totalW = cardRight - rect.left();
    const int marginL = qRound(totalW * params.marginPctL);
    const int marginR = qRound(totalW * params.marginPctR);
    const int drawW = totalW - marginL - marginR;

    const double basescale = qMax(double(drawW) / art.width(), double(cardH) / art.height());
    const double scale = basescale * params.zoom;

    const int scaledW = qRound(art.width() * scale);
    const int scaledH = qRound(art.height() * scale);

    const QPixmap scaled = art.scaled(scaledW, scaledH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    const int srcX = (scaledW - drawW) / 2;
    const int srcY = qRound((scaledH - cardH) * params.verticalOffset);

    // Clamp srcY so we never copy outside the pixmap bounds
    const int safeSrcY = qBound(0, srcY, qMax(0, scaledH - cardH));

    QImage img =
        scaled.copy(srcX, safeSrcY, drawW, cardH).toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);

    {
        QPainter mask(&img);
        mask.setCompositionMode(QPainter::CompositionMode_DestinationIn);

        QLinearGradient grad(0, 0, img.width(), 0);
        grad.setColorAt(0.00, Qt::transparent);
        grad.setColorAt(0.22, Qt::white);
        grad.setColorAt(0.78, Qt::white);
        grad.setColorAt(1.00, Qt::transparent);

        mask.fillRect(img.rect(), grad);
    }

    painter->setOpacity(0.55);
    painter->drawImage(rect.left() + marginL, rect.top() + 2, img);
    painter->setOpacity(1.0);
}

QRect UserListPainter::getAvatarRect(const QRect &rect)
{
    const int avatarX = rect.left() + LeftPadding;
    const int avatarY = rect.top() + (rect.height() - AvatarSize) / 2;
    return QRect(avatarX, avatarY, AvatarSize, AvatarSize);
}

void UserListPainter::drawAvatar(QPainter *painter,
                                 const QRect &avatarRect,
                                 const QString &userName,
                                 const QColor &accentColor,
                                 const UserLevelFlags &userLevel,
                                 const ServerInfo_User &userInfo,
                                 const QString &privLevel,
                                 const QMap<QString, QPixmap> *avatarCache)
{
    QPainterPath clipPath;
    clipPath.addEllipse(avatarRect);

    painter->save();
    painter->setClipPath(clipPath);

    bool drewAvatar = false;

    if (avatarCache && avatarCache->contains(userName)) {
        const QPixmap &avatar = avatarCache->value(userName);
        if (!avatar.isNull()) {
            painter->drawPixmap(
                avatarRect, avatar.scaled(avatarRect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            drewAvatar = true;
        }
    }

    if (!drewAvatar) {
        painter->setBrush(accentColor.darker(200));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(avatarRect);

        const QPixmap pawn =
            UserLevelPixmapGenerator::generatePixmap(24, userLevel, userInfo.pawn_colors(), false, privLevel);

        painter->drawPixmap(avatarRect.center().x() - 12, avatarRect.center().y() - 12, pawn);
    }

    painter->restore();
}

void UserListPainter::drawStatusRing(QPainter *painter, const QRect &avatarRect, bool online)
{
    const QColor statusColor = online ? QColor(34, 197, 94) : QColor(70, 80, 95);

    painter->setPen(QPen(statusColor, 2));
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(avatarRect.adjusted(-1, -1, 1, 1));
}

void UserListPainter::drawUserName(QPainter *painter,
                                   const QStyleOptionViewItem &option,
                                   const QRect &rect,
                                   int cardRight,
                                   int textX,
                                   const QString &userName,
                                   bool online,
                                   bool selected)
{
    QFont nameFont = option.font;
    nameFont.setBold(true);
    painter->setFont(nameFont);

    const QRect nameRect(textX, rect.top() + 8, cardRight - textX - 10, 20);
    const QString elidedName = QFontMetrics(nameFont).elidedText(userName, Qt::ElideRight, cardRight - textX - 10);

    painter->setPen(QColor(0, 0, 0, 200));
    painter->drawText(nameRect.translated(1, 1), Qt::AlignVCenter | Qt::AlignLeft, elidedName);

    painter->setPen(online ? (selected ? Qt::white : QColor(226, 232, 240)) : QColor(90, 100, 115));
    painter->drawText(nameRect, Qt::AlignVCenter | Qt::AlignLeft, elidedName);
}

void UserListPainter::drawCountryFlag(QPainter *painter, const QRect &rect, int textX, const ServerInfo_User &userInfo)
{
    const QPixmap flag = CountryPixmapGenerator::generatePixmap(13, QString::fromStdString(userInfo.country()));
    if (!flag.isNull()) {
        painter->drawPixmap(textX, rect.top() + 46, flag);
    }
}

QList<UserListPainter::Badge> UserListPainter::buildBadges(const UserLevelFlags &userLevel, const QString &privLevel)
{
    QList<Badge> badges;

    if (userLevel.testFlag(ServerInfo_User::IsAdmin)) {
        badges << Badge{"ADMIN", QColor(245, 158, 11)};
    } else if (userLevel.testFlag(ServerInfo_User::IsModerator)) {
        badges << Badge{"MOD", QColor(59, 130, 246)};
    } else if (userLevel.testFlag(ServerInfo_User::IsJudge)) {
        badges << Badge{"JUDGE", QColor(168, 85, 247)};
    }

    if (privLevel == "VIP") {
        badges << Badge{"VIP", QColor(20, 184, 166)};
    } else if (privLevel == "DONATOR") {
        badges << Badge{"DONATOR", QColor(249, 115, 22)};
    }

    return badges;
}

void UserListPainter::drawBadges(QPainter *painter,
                                 const QStyleOptionViewItem &option,
                                 const QRect &rect,
                                 int cardRight,
                                 const QList<Badge> &badges,
                                 bool online)
{
    if (badges.isEmpty()) {
        return;
    }

    QFont badgeFont = option.font;
    badgeFont.setPointSizeF(badgeFont.pointSizeF() * 0.68);
    badgeFont.setBold(true);
    painter->setFont(badgeFont);

    QFontMetrics fm(badgeFont);

    int totalBadgeW = 0;
    for (const Badge &b : badges) {
        totalBadgeW += fm.horizontalAdvance(b.text) + 8 + 4;
    }
    totalBadgeW -= 4;

    int bx = cardRight - 6 - totalBadgeW;

    for (const Badge &b : badges) {
        const QColor col = online ? b.color : b.color.darker(180);
        const int bw = fm.horizontalAdvance(b.text) + 8;
        const QRect br(bx, rect.top() + 44, bw, 13);

        painter->setPen(Qt::NoPen);
        painter->setBrush(col.darker(online ? 160 : 220));
        painter->drawRoundedRect(br, 3, 3);

        painter->setPen(col.lighter(online ? 160 : 100));
        painter->drawText(br, Qt::AlignCenter, b.text);

        bx += bw + 4;
    }
}

void UserListPainter::paint(QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index,
                            const ServerInfo_User &userInfo,
                            const QMap<QString, QPixmap> *avatarCache,
                            const QMap<QString, QPixmap> *cardArtCache,
                            const QMap<QString, CardArtParams> *cardArtParamsMap)
{
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

    const QRect rect = option.rect;
    const bool online = index.data(Qt::UserRole + 1).toBool();
    const bool selected = option.state & QStyle::State_Selected;
    const UserLevelFlags userLevel(userInfo.user_level());
    const QString userName = QString::fromStdString(userInfo.name());
    const QString privLevel = QString::fromStdString(userInfo.privlevel());
    const QColor accentColor = getAccentColor(userLevel, online);
    const QRectF cardRect = QRectF(rect).adjusted(3, 2, -3, -2);
    const int cardRight = getCardRight(option, rect);

    const CardArtParams params = (cardArtParamsMap && cardArtParamsMap->contains(userName))
                                     ? cardArtParamsMap->value(userName)
                                     : CardArtParams{};

    drawBackground(painter, cardRect, accentColor, selected);
    drawCardArt(painter, rect, cardRight, userName, cardArtCache, params);

    const QRect avatarRect = getAvatarRect(rect);
    drawAvatar(painter, avatarRect, userName, accentColor, userLevel, userInfo, privLevel, avatarCache);
    drawStatusRing(painter, avatarRect, online);

    const int textX = avatarRect.right() + TextSpacing;
    drawUserName(painter, option, rect, cardRight, textX, userName, online, selected);
    drawCountryFlag(painter, rect, textX, userInfo);

    const QList<Badge> badges = buildBadges(userLevel, privLevel);
    drawBadges(painter, option, rect, cardRight, badges, online);

    painter->restore();
}