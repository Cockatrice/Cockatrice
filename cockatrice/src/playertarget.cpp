#include "playertarget.h"
#include "player.h"
#include "pixmapgenerator.h"
#include "pb/serverinfo_user.pb.h"
#include <QPainter>
#include <QPixmapCache>
#include <QDebug>
#include <cmath>
#ifdef _WIN32
#include "round.h"
#endif /* _WIN32 */

PlayerCounter::PlayerCounter(Player *_player, int _id, const QString &_name, int _value, QGraphicsItem *parent)
    : AbstractCounter(_player, _id, _name, false, _value, parent)
{
}

QRectF PlayerCounter::boundingRect() const
{
    return QRectF(0, 0, 50, 30);
}

void PlayerCounter::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    const int radius = 8;
    const qreal border = 1;
    QPainterPath path(QPointF(50 - border / 2, border / 2));
    path.lineTo(radius, border / 2);
    path.arcTo(border / 2, border / 2, 2 * radius, 2 * radius, 90, 90);
    path.lineTo(border / 2, 30 - border / 2);
    path.lineTo(50 - border / 2, 30 - border / 2);
    path.closeSubpath();
    
    QPen pen(QColor(100, 100, 100));
    pen.setWidth(border);
    painter->setPen(pen);
    painter->setBrush(hovered ? QColor(50, 50, 50, 160) : QColor(0, 0, 0, 160));
    
    painter->drawPath(path);

    QRectF translatedRect = path.controlPointRect();
    QSize translatedSize = translatedRect.size().toSize();
    QFont font("Serif");
    font.setWeight(QFont::Bold);
    font.setPixelSize(qMax((int) round(translatedSize.height() / 1.3), 9));
    painter->setFont(font);
    painter->setPen(Qt::white);
    painter->drawText(translatedRect, Qt::AlignCenter, QString::number(value));
}

PlayerTarget::PlayerTarget(Player *_owner, QGraphicsItem *parentItem)
    : ArrowTarget(_owner, parentItem), playerCounter(0)
{
    setCacheMode(DeviceCoordinateCache);
    
    const std::string &bmp = _owner->getUserInfo()->avatar_bmp();
    if (!fullPixmap.loadFromData((const uchar *) bmp.data(), bmp.size()))
        fullPixmap = QPixmap();
}

PlayerTarget::~PlayerTarget()
{
    // Explicit deletion is necessary in spite of parent/child relationship
    // as we need this object to be alive to receive the destroyed() signal.
    delete playerCounter;
}

QRectF PlayerTarget::boundingRect() const
{
    return QRectF(0, 0, 160, 64);
}

void PlayerTarget::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    const ServerInfo_User *const info = owner->getUserInfo();

    const qreal border = 2;

    QRectF avatarBoundingRect = boundingRect().adjusted(border, border, -border, -border);
    QRectF translatedRect = painter->combinedTransform().mapRect(avatarBoundingRect);
    QSize translatedSize = translatedRect.size().toSize();
    QPixmap cachedPixmap;
    const QString cacheKey = "avatar" + QString::number(translatedSize.width()) + "_" + QString::number(info->user_level()) + "_" + QString::number(fullPixmap.cacheKey());
    if (!QPixmapCache::find(cacheKey, &cachedPixmap)) {
        cachedPixmap = QPixmap(translatedSize.width(), translatedSize.height());
        
        QPainter tempPainter(&cachedPixmap);
	// pow(foo, 0.5) equals to sqrt(foo), but using sqrt(foo) in this context will produce a compile error with MSVC++
        QRadialGradient grad(translatedRect.center(), pow(translatedSize.width() * translatedSize.width() + translatedSize.height() * translatedSize.height(), 0.5) / 2);
        grad.setColorAt(1, Qt::black);
        grad.setColorAt(0, QColor(180, 180, 180));
        tempPainter.fillRect(QRectF(0, 0, translatedSize.width(), translatedSize.height()), grad);
        
        QPixmap tempPixmap;
        if (fullPixmap.isNull())
            tempPixmap = UserLevelPixmapGenerator::generatePixmap(translatedSize.height(), UserLevelFlags(info->user_level()), false);
        else
            tempPixmap = fullPixmap.scaled(translatedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        
        tempPainter.drawPixmap((translatedSize.width() - tempPixmap.width()) / 2, (translatedSize.height() - tempPixmap.height()) / 2, tempPixmap);
        QPixmapCache::insert(cacheKey, cachedPixmap);
    }
    
    painter->save();
    painter->resetTransform();
    painter->translate((translatedSize.width() - cachedPixmap.width()) / 2.0, 0);
    painter->drawPixmap(translatedRect, cachedPixmap, cachedPixmap.rect());
    painter->restore();

    QRectF nameRect = QRectF(0, boundingRect().height() - 20, 110, 20);
    painter->fillRect(nameRect, QColor(0, 0, 0, 160));
    QRectF translatedNameRect = painter->combinedTransform().mapRect(nameRect);
    
    painter->save();
    painter->resetTransform();
    
    QString name = QString::fromStdString(info->name());
    if (name.size() > 13)
        name = name.mid(0, 10) + "...";
    
    QFont font;
    font.setPixelSize(qMax((int) round(translatedNameRect.height() / 1.5), 9));
    painter->setFont(font);
    painter->setPen(Qt::white);
    painter->drawText(translatedNameRect, Qt::AlignVCenter | Qt::AlignLeft, "  " + name);
    painter->restore();

    QPen pen(QColor(100, 100, 100));
    pen.setWidth(border);
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);
    painter->drawRect(boundingRect().adjusted(border / 2, border / 2, -border / 2, -border / 2));
    
    if (getBeingPointedAt())
        painter->fillRect(boundingRect(), QBrush(QColor(255, 0, 0, 100)));
}

AbstractCounter *PlayerTarget::addCounter(int _counterId, const QString &_name, int _value)
{
    if (playerCounter) {
        disconnect(playerCounter, 0, this, 0);
        playerCounter->delCounter();
    }

    playerCounter = new PlayerCounter(owner, _counterId, _name, _value, this);
    playerCounter->setPos(boundingRect().width() - playerCounter->boundingRect().width(), boundingRect().height() - playerCounter->boundingRect().height());
    connect(playerCounter, SIGNAL(destroyed()), this, SLOT(counterDeleted()));
    
    return playerCounter;
}

void PlayerTarget::counterDeleted()
{
    playerCounter = 0;
}
