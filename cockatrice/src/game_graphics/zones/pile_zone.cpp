#include "pile_zone.h"

#include "../../client/settings/cache_settings.h"
#include "../../interface/theme_manager.h"
#include "../board/card_drag_item.h"
#include "../board/card_item.h"
#include "../card_dimensions.h"

#include <QApplication>
#include <QColor>
#include <QFont>
#include <QPixmap>
#include <QString>
#include <QCursor>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QList>
#include <QPainter>
#include <QPainterPath>
#include <QPoint>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QTransform>
#include <QtGlobal>
#include <libcockatrice/game/board/card_list.h>
#include <libcockatrice/game/player/player_actions.h>
#include <libcockatrice/game/player/player_info.h>
#include <libcockatrice/game/player/player_logic.h>
#include <libcockatrice/game/zones/card_zone_logic.h>
#include <libcockatrice/game/zones/pile_zone_logic.h>
#include <libcockatrice/settings/cards_display_settings.h>
#include <qnamespace.h>
#include <string>

PileZone::PileZone(PileZoneLogic *_logic, QGraphicsItem *parent,
                   const QString &displayName, const QString &iconAsset)
    : CardZone(_logic, parent), customDisplayName(displayName), customIconAsset(iconAsset)
{
    setCacheMode(DeviceCoordinateCache); // Do not move this line to the parent constructor!
    setAcceptHoverEvents(true);
    setCursor(Qt::OpenHandCursor);

    if (!customIconAsset.isEmpty()) {
        connect(themeManager, &ThemeManager::themeChanged, this, &PileZone::reloadCustomIcon);
        reloadCustomIcon();
    }

    setTransform(QTransform()
                     .translate(CardDimensions::WIDTH_HALF_F, CardDimensions::HEIGHT_HALF_F)
                     .rotate(90)
                     .translate(-CardDimensions::WIDTH_HALF_F, -CardDimensions::HEIGHT_HALF_F));

    connect(&SettingsCache::instance().cardsDisplay(), &CardsDisplaySettings::roundCardCornersChanged, this,
            [this](bool _roundCardCorners) {
                Q_UNUSED(_roundCardCorners);

                prepareGeometryChange();
                update();
            });
}

void PileZone::reloadCustomIcon()
{
    // Try the active theme's piles/<name> asset first, then the bundled SVG.
    const QString path = QStringLiteral("piles/") + customIconAsset;
    customIconPixmap = QPixmap(QStringLiteral("theme:") + themeManager->assetPath(path));
    if (customIconPixmap.isNull()) {
        customIconPixmap =
            QPixmap(QStringLiteral(":/resources/piles/") + customIconAsset + QStringLiteral(".svg"));
    }
    update();
}

QRectF PileZone::boundingRect() const
{
    return QRectF(0, 0, CardDimensions::WIDTH_F, CardDimensions::HEIGHT_F);
}

QPainterPath PileZone::shape() const
{
    QPainterPath shape;
    qreal cardCornerRadius =
        SettingsCache::instance().cardsDisplay().getRoundCardCorners() ? 0.05 * CardDimensions::WIDTH_F : 0.0;
    shape.addRoundedRect(boundingRect(), cardCornerRadius, cardCornerRadius);
    return shape;
}

void PileZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    painter->drawPath(shape());

    if (!cardItems().isEmpty()) {
        cardItems().at(0)->paintPicture(painter, cardItems().at(0)->getTranslatedSize(painter), 90);
    }

    painter->translate(CardDimensions::WIDTH_HALF_F, CardDimensions::HEIGHT_HALF_F);
    painter->rotate(-90);
    painter->translate(-CardDimensions::WIDTH_HALF_F, -CardDimensions::HEIGHT_HALF_F);
    paintNumberEllipse(cardItems().size(), 28, Qt::white, -1, -1, painter);

    if (!customDisplayName.isEmpty()) {
        painter->save();
        // Undo the pile's 90-degree scene rotation to make the badge readable.
        painter->translate(CardDimensions::WIDTH_HALF_F, CardDimensions::HEIGHT_HALF_F);
        painter->rotate(-90);
        painter->translate(-CardDimensions::HEIGHT_HALF_F, -CardDimensions::WIDTH_HALF_F);

        const qreal labelWidth = CardDimensions::HEIGHT_F;
        painter->fillRect(QRectF(2, 2, labelWidth - 4, 29), QColor(16, 18, 28, 220));
        if (!customIconPixmap.isNull()) {
            painter->drawPixmap(QRect(7, 6, 21, 21), customIconPixmap);
        }
        QFont font = painter->font();
        font.setPixelSize(12);
        font.setBold(true);
        painter->setFont(font);
        painter->setPen(Qt::white);
        painter->drawText(QRectF(31, 3, labelWidth - 34, 26), Qt::AlignLeft | Qt::AlignVCenter,
                          customDisplayName.toUpper());
        painter->restore();
    }
}

void PileZone::handleDropEvent(const QList<CardDragItem *> &dragItems, CardZoneLogic *startZone, const QPoint &)
{
    QList<CardMoveRequest> cards;
    cards.reserve(dragItems.size());
    for (const CardDragItem *item : dragItems) {
        cards.append(CardMoveRequest{item->getId(), item->isForceFaceDown()});
    }

    getLogic()->getPlayer()->getPlayerActions()->moveCards(startZone, getLogic(), 0, 0, cards);
}

void PileZone::onCardAdded(CardState *addedCard)
{
    CardZone::onCardAdded(addedCard);
    auto *addedItem = addedCard == nullptr ? nullptr : qobject_cast<CardItem *>(addedCard->parent());
    if (addedItem) {
        addedItem->setPos(0, 0);
        addedItem->setVisible(false);
    }
}

void PileZone::reorganizeCards()
{
    update();
}

void PileZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    CardZone::mousePressEvent(event);
    if (event->isAccepted()) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else {
        event->ignore();
    }
}

void PileZone::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() <
        QApplication::startDragDistance()) {
        return;
    }

    if (cardItems().isEmpty()) {
        return;
    }

    bool forceFaceDown = event->modifiers().testFlag(Qt::ShiftModifier);
    bool bottomCard = event->modifiers().testFlag(Qt::ControlModifier);
    CardItem *card = bottomCard ? cardItems().last() : cardItems().first();
    const int cardid = getLogic()->contentsKnown() ? card->getId() : (bottomCard ? cardItems().size() - 1 : 0);
    CardDragItem *drag = card->createDragItem(cardid, event->pos(), event->scenePos(), forceFaceDown);
    drag->grabMouse();
    setCursor(Qt::OpenHandCursor);
}

void PileZone::mouseReleaseEvent(QGraphicsSceneMouseEvent * /*event*/)
{
    setCursor(Qt::OpenHandCursor);
}

void PileZone::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (!cardItems().isEmpty()) {
        cardItems()[0]->processHoverEvent();
    }
    QGraphicsItem::hoverEnterEvent(event);
}
