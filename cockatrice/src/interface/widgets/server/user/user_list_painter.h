#ifndef COCKATRICE_USER_LIST_PAINTER_H
#define COCKATRICE_USER_LIST_PAINTER_H

#include "user_level.h"

#include <QColor>
#include <QList>
#include <QMap>
#include <QPixmap>
#include <QRect>
#include <QSize>

class QPainter;
class QModelIndex;
class QStyleOptionViewItem;
class ServerInfo_User;

struct CardArtParams
{
    QString cardName = "";
    double marginPctL = 0.33;
    double marginPctR = 0.02;
    double verticalOffset = 0.35;
    double zoom = 1.0;
};

class UserListPainter
{
public:
    static void paint(QPainter *painter,
                      const QStyleOptionViewItem &option,
                      const QModelIndex &index,
                      const ServerInfo_User &userInfo,
                      const QMap<QString, QPixmap> *avatarCache,
                      const QMap<QString, QPixmap> *cardArtCache,
                      const QMap<QString, CardArtParams> *cardArtParamsMap);

    static QSize sizeHint();

    static void drawCardArt(QPainter *painter,
                            const QRect &rect,
                            int cardRight,
                            const QString &userName,
                            const QMap<QString, QPixmap> *cardArtCache,
                            const CardArtParams &params,
                            const QPixmap *overridePixmap);

private:
    struct Badge
    {
        QString text;
        QColor color;
    };

    static QColor getAccentColor(const UserLevelFlags &userLevel, bool online);
    static int getCardRight(const QStyleOptionViewItem &option, const QRect &rect);
    static void drawBackground(QPainter *painter, const QRectF &cardRect, const QColor &accentColor, bool selected);
    static QRect getAvatarRect(const QRect &rect);
    static void drawAvatar(QPainter *painter,
                           const QRect &avatarRect,
                           const QString &userName,
                           const QColor &accentColor,
                           const UserLevelFlags &userLevel,
                           const ServerInfo_User &userInfo,
                           const QString &privLevel,
                           const QMap<QString, QPixmap> *avatarCache);
    static void drawStatusRing(QPainter *painter, const QRect &avatarRect, bool online);
    static void drawUserName(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QRect &rect,
                             int cardRight,
                             int textX,
                             const QString &userName,
                             bool online,
                             bool selected);
    static void drawCountryFlag(QPainter *painter, const QRect &rect, int textX, const ServerInfo_User &userInfo);
    static QList<Badge> buildBadges(const UserLevelFlags &userLevel, const QString &privLevel);
    static void drawBadges(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QRect &rect,
                           int cardRight,
                           const QList<Badge> &badges,
                           bool online);
};

#endif // COCKATRICE_USER_LIST_PAINTER_H