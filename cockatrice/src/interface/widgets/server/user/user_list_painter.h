#ifndef COCKATRICE_USER_LIST_PAINTER_H
#define COCKATRICE_USER_LIST_PAINTER_H

#include "user_level.h"

#include <QColor>
#include <QList>
#include <QMap>
#include <QPalette>
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
    QString cardProviderId = "";
    double marginPctL = 0.33;
    double marginPctR = 0.02;
    double verticalOffset = 0.35;
    double zoom = 1.0;
};

class UserListPainter
{
public:
    /**
     * Palette-derived surface colors for the current color scheme. Both the
     * light and the dark scheme read from the active QPalette so custom
     * palettes are respected. @c dark only tunes the blend strengths (and
     * whether the name text keeps its drop shadow).
     */
    struct Style
    {
        bool dark = true;
        QColor cardStart; ///< row fill, left edge (normal)
        QColor cardEnd;   ///< row fill, right edge (normal)
        QColor base;      ///< lightest surface, used for blending accent hues
        QColor textOnline;
        QColor textOffline;
        QColor ringOffline;
        bool dropShadow = false;
    };

    static Style resolveStyle(const QPalette &palette, bool dark);
    /// Linear interpolation: @p t = 0 returns @p a, @p t = 1 returns @p b.
    static QColor blend(const QColor &a, const QColor &b, qreal t);

    static void paint(QPainter *painter,
                      const QStyleOptionViewItem &option,
                      const QModelIndex &index,
                      const ServerInfo_User &userInfo,
                      const QMap<QString, QPixmap> *avatarCache,
                      const QMap<QString, QPixmap> *cardArtCache,
                      const QMap<QString, CardArtParams> *cardArtParamsMap,
                      bool dark);

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
    static void drawBackground(QPainter *painter,
                               const QRectF &cardRect,
                               const QColor &accentColor,
                               bool selected,
                               const Style &style,
                               bool hasRole);
    static QRect getAvatarRect(const QRect &rect);
    static void drawAvatar(QPainter *painter,
                           const QRect &avatarRect,
                           const QString &userName,
                           const QColor &accentColor,
                           const UserLevelFlags &userLevel,
                           const ServerInfo_User &userInfo,
                           const QString &privLevel,
                           const QMap<QString, QPixmap> *avatarCache,
                           const Style &style);
    static void drawStatusRing(QPainter *painter, const QRect &avatarRect, bool online, const Style &style);
    static void drawUserName(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QRect &rect,
                             int cardRight,
                             int textX,
                             const QString &userName,
                             bool online,
                             const Style &style);
    static void drawCountryFlag(QPainter *painter, const QRect &rect, int textX, const ServerInfo_User &userInfo);
    static QList<Badge> buildBadges(const UserLevelFlags &userLevel, const QString &privLevel);
    static void drawBadges(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QRect &rect,
                           int cardRight,
                           const QList<Badge> &badges,
                           bool online,
                           const Style &style);
};

#endif // COCKATRICE_USER_LIST_PAINTER_H
