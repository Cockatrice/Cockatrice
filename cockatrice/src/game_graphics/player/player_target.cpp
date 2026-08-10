#include "player_target.h"

#include "../../client/settings/cache_settings.h"
#include "../../game/player/player_logic.h"
#include "../../interface/pixel_map_generator.h"
#include "../../interface/theme_manager.h"

#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QPixmapCache>
#include <QTimer>
#include <QtMath>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>

PlayerCounter::PlayerCounter(CounterState *state, PlayerLogic *player, QGraphicsItem *parent)
    : AbstractCounter(state, player, false, false, parent)
{
}

QRectF PlayerCounter::boundingRect() const
{
    return {0, 0, 50, 30};
}

void PlayerCounter::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    // design-game: life totals live in a fully-rounded translucent HUD pill
    // with an accent (Mana Green) border and mono numerals.
    const int radius = 15;
    const qreal border = 1.5;
    QPainterPath path(QPointF(50 - border / 2, border / 2));
    path.lineTo(radius, border / 2);
    path.arcTo(border / 2, border / 2, 2 * radius, 2 * radius, 90, 90);
    path.lineTo(border / 2, 30 - border / 2);
    path.lineTo(50 - border / 2, 30 - border / 2);
    path.closeSubpath();

    QPen pen(qApp->palette().highlight().color());
    pen.setWidthF(border);
    painter->setPen(pen);
    painter->setBrush(hovered ? QColor(18, 24, 38, 200) : QColor(11, 14, 20, 184));

    painter->drawPath(path);

    QRectF translatedRect = path.controlPointRect();
    QSize translatedSize = translatedRect.size().toSize();
    QFont font = themeManager->monoFont();
    font.setWeight(QFont::Bold);
    font.setPixelSize(qMax(qRound(translatedSize.height() / 1.3), 9));
    painter->setFont(font);
    painter->setPen(Qt::white);
    painter->drawText(translatedRect, Qt::AlignCenter, QString::number(value));

    // Life-change flash: emerald on gain, red on loss, decaying over a few ticks.
    if (flashAlpha > 0) {
        painter->save();
        QColor flashColor = flashDelta > 0 ? QColor(52, 224, 122) : QColor(239, 68, 68);
        flashColor.setAlphaF(0.45 * flashAlpha);
        painter->setPen(Qt::NoPen);
        painter->setBrush(flashColor);
        painter->setOpacity(0.85);
        painter->drawPath(path);
        painter->restore();
    }
}

void PlayerCounter::onValueChanged(int oldValue, int newValue)
{
    flashDelta = newValue - oldValue;
    if (flashDelta == 0) {
        return;
    }

    if (!SettingsCache::instance().userInterface().getAnimationsEnabled() ||
        !SettingsCache::instance().userInterface().getLifeCounterAnimationsEnabled()) {
        flashAlpha = 0.0;
        return;
    }

    flashAlpha = 1.0;
    if (flashTimer == nullptr) {
        flashTimer = new QTimer(this);
        connect(flashTimer, &QTimer::timeout, this, &PlayerCounter::flashTick);
    }
    flashTimer->start(50);
}

void PlayerCounter::flashTick()
{
    flashAlpha -= 0.12;
    if (flashAlpha <= 0.0) {
        flashAlpha = 0.0;
        flashTimer->stop();
    }
    update();
}

PlayerTarget::PlayerTarget(PlayerLogic *_owner, QGraphicsItem *parentItem)
    : ArrowTarget(_owner, parentItem), playerCounter(nullptr)
{
    setCacheMode(DeviceCoordinateCache);

    const std::string &bmp = _owner->getPlayerInfo()->getUserInfo()->avatar_bmp();
    if (!fullPixmap.loadFromData((const uchar *)bmp.data(), static_cast<uint>(bmp.size()))) {
        fullPixmap = QPixmap();
    }
}

PlayerTarget::~PlayerTarget()
{
    // Explicit deletion is necessary in spite of parent/child relationship
    // as we need this object to be alive to receive the destroyed() signal.
    delete playerCounter;
}

QRectF PlayerTarget::boundingRect() const
{
    return {0, 0, 160, 64};
}

void PlayerTarget::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    const ServerInfo_User *const info = owner->getPlayerInfo()->getUserInfo();

    const qreal border = 2;

    QRectF avatarBoundingRect = boundingRect().adjusted(border, border, -border, -border);
    QRectF translatedRect = painter->combinedTransform().mapRect(avatarBoundingRect);
    QSize translatedSize = translatedRect.size().toSize();
    QPixmap cachedPixmap;
    const QString cacheKey = "avatar" + QString::number(translatedSize.width()) + "_" +
                             QString::number(info->user_level()) + "_" + QString::number(fullPixmap.cacheKey());
    if (!QPixmapCache::find(cacheKey, &cachedPixmap)) {
        cachedPixmap = QPixmap(translatedSize.width(), translatedSize.height());

        QPainter tempPainter(&cachedPixmap);
        // pow(foo, 0.5) equals to sqrt(foo), but using sqrt(foo) in this context will produce a compile error with
        // MSVC++
        QRadialGradient grad(translatedRect.center(), qPow(translatedSize.width() * translatedSize.width() +
                                                               translatedSize.height() * translatedSize.height(),
                                                           0.5) /
                                                          2);
        grad.setColorAt(1, Qt::black);
        grad.setColorAt(0, QColor(180, 180, 180));
        tempPainter.fillRect(QRectF(0, 0, translatedSize.width(), translatedSize.height()), grad);

        if (fullPixmap.isNull()) {
            int sideLength = translatedSize.height();
            QPixmap tempPixmap = UserLevelPixmapGenerator::generatePixmap(
                sideLength, UserLevelFlags(info->user_level()), info->pawn_colors(), false,
                QString::fromStdString(info->privlevel()));
            int x = (translatedSize.width() - sideLength) / 2;
            int y = 0;
            tempPainter.drawPixmap(x, y, tempPixmap);
        } else {
            QPixmap tempPixmap = fullPixmap.scaled(translatedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            int x = (translatedSize.width() - tempPixmap.width()) / 2;
            int y = (translatedSize.height() - tempPixmap.height()) / 2;
            tempPainter.drawPixmap(x, y, tempPixmap);
        }

        QPixmapCache::insert(cacheKey, cachedPixmap);
    }

    painter->save();
    resetPainterTransform(painter);
    painter->translate((translatedSize.width() - cachedPixmap.width()) / 2.0, 0);
    painter->drawPixmap(translatedRect, cachedPixmap, cachedPixmap.rect());
    painter->restore();

    QRectF nameRect = QRectF(0, boundingRect().height() - 20, 110, 20);
    painter->fillRect(nameRect, QColor(0, 0, 0, 160));
    QRectF translatedNameRect = painter->combinedTransform().mapRect(nameRect);

    painter->save();
    resetPainterTransform(painter);

    QString name = QString::fromStdString(info->name());
    if (name.size() > 13) {
        name = name.mid(0, 10) + "...";
    }

    QFont font;
    font.setPixelSize(qMax(qRound(translatedNameRect.height() / 1.5), 9));
    painter->setFont(font);
    painter->setPen(Qt::white);
    painter->drawText(translatedNameRect, Qt::AlignVCenter | Qt::AlignLeft, "  " + name);
    painter->restore();

    QPen pen(QColor(100, 100, 100));
    pen.setWidth(border);
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);
    painter->drawRect(boundingRect().adjusted(border / 2, border / 2, -border / 2, -border / 2));

    if (getBeingPointedAt()) {
        painter->fillRect(boundingRect(), QBrush(QColor(255, 0, 0, 100)));
    }
}

AbstractCounter *PlayerTarget::addCounter(CounterState *state)
{
    if (playerCounter) {
        disconnect(playerCounter, nullptr, this, nullptr);
        playerCounter->delCounter();
    }
    playerCounter = new PlayerCounter(state, owner, this);
    playerCounter->setPos(boundingRect().width() - playerCounter->boundingRect().width(),
                          boundingRect().height() - playerCounter->boundingRect().height());
    connect(playerCounter, &PlayerCounter::destroyed, this, &PlayerTarget::counterDeleted);
    return playerCounter;
}

void PlayerTarget::counterDeleted()
{
    playerCounter = nullptr;
}
