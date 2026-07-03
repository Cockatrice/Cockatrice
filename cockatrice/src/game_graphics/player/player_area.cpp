#include "player_area.h"

#include "../../interface/theme_manager.h"

#include <QPainter>

PlayerArea::PlayerArea(QGraphicsItem *parentItem) : QObject(), QGraphicsItem(parentItem)
{
    setCacheMode(DeviceCoordinateCache);
    connect(themeManager, &ThemeManager::themeChanged, this, &PlayerArea::updateBg);
    updateBg();
}

void PlayerArea::updateBg()
{
    update();
}

void PlayerArea::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    QBrush brush = themeManager->getExtraBgBrush(ThemeManager::Player, playerZoneId);
    painter->fillRect(boundingRect(), brush);
}

void PlayerArea::setSize(qreal width, qreal height)
{
    prepareGeometryChange();
    bRect = QRectF(0, 0, width, height);
}

void PlayerArea::setPlayerZoneId(int _playerZoneId)
{
    playerZoneId = _playerZoneId;
}