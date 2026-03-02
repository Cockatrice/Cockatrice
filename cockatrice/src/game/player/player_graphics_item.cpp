#include "player_graphics_item.h"

#include "../../interface/widgets/tabs/tab_game.h"
#include "../board/abstract_card_item.h"
#include "../hand_counter.h"
#include "../zones/hand_zone.h"
#include "../zones/pile_zone.h"
#include "../zones/stack_zone.h"
#include "../zones/table_zone.h"

PlayerGraphicsItem::PlayerGraphicsItem(Player *_player) : player(_player)
{
    connect(&SettingsCache::instance(), &SettingsCache::horizontalHandChanged, this,
            &PlayerGraphicsItem::rearrangeZones);
    connect(&SettingsCache::instance(), &SettingsCache::handJustificationChanged, this,
            &PlayerGraphicsItem::rearrangeZones);
    connect(player, &Player::rearrangeCounters, this, &PlayerGraphicsItem::rearrangeCounters);

    playerArea = new PlayerArea(this);

    playerTarget = new PlayerTarget(player, playerArea);
    qreal avatarMargin = (counterAreaWidth + CARD_HEIGHT + 15 - playerTarget->boundingRect().width()) / 2.0;
    playerTarget->setPos(QPointF(avatarMargin, avatarMargin));

    initializeZones();

    connect(tableZoneGraphicsItem, &TableZone::sizeChanged, this, &PlayerGraphicsItem::updateBoundingRect);

    updateBoundingRect();

    rearrangeZones();
    retranslateUi();
}

void PlayerGraphicsItem::retranslateUi()
{
    player->getPlayerMenu()->retranslateUi();

    QMapIterator<QString, CardZoneLogic *> zoneIterator(player->getZones());
    while (zoneIterator.hasNext()) {
        emit zoneIterator.next().value()->retranslateUi();
    }

    QMapIterator<int, AbstractCounter *> counterIterator(player->getCounters());
    while (counterIterator.hasNext()) {
        counterIterator.next().value()->retranslateUi();
    }
}

void PlayerGraphicsItem::onPlayerActiveChanged(bool _active)
{
    tableZoneGraphicsItem->setActive(_active);
}

void PlayerGraphicsItem::initializeZones()
{
    deckZoneGraphicsItem = new PileZone(player->getDeckZone(), this);
    auto base = QPointF(counterAreaWidth + (CARD_HEIGHT - CARD_WIDTH + 15) / 2.0,
                        10 + playerTarget->boundingRect().height() + 5 - (CARD_HEIGHT - CARD_WIDTH) / 2.0);
    deckZoneGraphicsItem->setPos(base);

    qreal h = deckZoneGraphicsItem->boundingRect().width() + 5;

    sideboardGraphicsItem = new PileZone(player->getSideboardZone(), this);
    player->getSideboardZone()->setGraphicsVisibility(false);

    auto *handCounter = new HandCounter(playerArea);
    handCounter->setPos(base + QPointF(0, h + 10));
    qreal h2 = handCounter->boundingRect().height();

    graveyardZoneGraphicsItem = new PileZone(player->getGraveZone(), this);
    graveyardZoneGraphicsItem->setPos(base + QPointF(0, h + h2 + 10));

    rfgZoneGraphicsItem = new PileZone(player->getRfgZone(), this);
    rfgZoneGraphicsItem->setPos(base + QPointF(0, 2 * h + h2 + 10));

    tableZoneGraphicsItem = new TableZone(player->getTableZone(), this);
    connect(tableZoneGraphicsItem, &TableZone::sizeChanged, this, &PlayerGraphicsItem::updateBoundingRect);

    stackZoneGraphicsItem =
        new StackZone(player->getStackZone(), static_cast<int>(tableZoneGraphicsItem->boundingRect().height()), this);

    handZoneGraphicsItem =
        new HandZone(player->getHandZone(), static_cast<int>(tableZoneGraphicsItem->boundingRect().height()), this);

    connect(handZoneGraphicsItem->getLogic(), &HandZoneLogic::cardCountChanged, handCounter,
            &HandCounter::updateNumber);
    connect(handCounter, &HandCounter::showContextMenu, handZoneGraphicsItem, &HandZone::showContextMenu);
}

QRectF PlayerGraphicsItem::boundingRect() const
{
    return bRect;
}

qreal PlayerGraphicsItem::getMinimumWidth() const
{
    qreal result = tableZoneGraphicsItem->getMinimumWidth() + CARD_HEIGHT + 15 + counterAreaWidth +
                   stackZoneGraphicsItem->boundingRect().width();
    if (!SettingsCache::instance().getHorizontalHand()) {
        result += handZoneGraphicsItem->boundingRect().width();
    }
    return result;
}

void PlayerGraphicsItem::paint(QPainter * /*painter*/,
                               const QStyleOptionGraphicsItem * /*option*/,
                               QWidget * /*widget*/)
{
}

void PlayerGraphicsItem::processSceneSizeChange(int newPlayerWidth)
{
    // Extend table (and hand, if horizontal) to accommodate the new player width.
    qreal tableWidth =
        newPlayerWidth - CARD_HEIGHT - 15 - counterAreaWidth - stackZoneGraphicsItem->boundingRect().width();
    if (!SettingsCache::instance().getHorizontalHand()) {
        tableWidth -= handZoneGraphicsItem->boundingRect().width();
    }

    tableZoneGraphicsItem->setWidth(tableWidth);
    handZoneGraphicsItem->setWidth(tableWidth + stackZoneGraphicsItem->boundingRect().width());
}

void PlayerGraphicsItem::setMirrored(bool _mirrored)
{
    if (mirrored != _mirrored) {
        mirrored = _mirrored;
        rearrangeZones();
    }
}

void PlayerGraphicsItem::rearrangeCounters()
{
    qreal marginTop = 80;
    const qreal padding = 5;
    qreal ySize = boundingRect().y() + marginTop;

    // Place objects
    for (const auto &counter : player->getCounters()) {
        AbstractCounter *ctr = counter;

        if (!ctr->getShownInCounterArea()) {
            continue;
        }

        QRectF br = ctr->boundingRect();
        ctr->setPos((counterAreaWidth - br.width()) / 2, ySize);
        ySize += br.height() + padding;
    }
}

void PlayerGraphicsItem::rearrangeZones()
{
    auto base = QPointF(CARD_HEIGHT + counterAreaWidth + 15, 0);
    if (SettingsCache::instance().getHorizontalHand()) {
        if (mirrored) {
            if (player->getHandZone()->contentsKnown()) {
                player->getPlayerInfo()->setHandVisible(true);
                handZoneGraphicsItem->setPos(base);
                base += QPointF(0, handZoneGraphicsItem->boundingRect().height());
            } else {
                player->getPlayerInfo()->setHandVisible(false);
            }

            stackZoneGraphicsItem->setPos(base);
            base += QPointF(stackZoneGraphicsItem->boundingRect().width(), 0);

            tableZoneGraphicsItem->setPos(base);
        } else {
            stackZoneGraphicsItem->setPos(base);

            tableZoneGraphicsItem->setPos(base.x() + stackZoneGraphicsItem->boundingRect().width(), 0);
            base += QPointF(0, tableZoneGraphicsItem->boundingRect().height());

            if (player->getHandZone()->contentsKnown()) {
                player->getPlayerInfo()->setHandVisible(true);
                handZoneGraphicsItem->setPos(base);
            } else {
                player->getPlayerInfo()->setHandVisible(false);
            }
        }
        handZoneGraphicsItem->setWidth(tableZoneGraphicsItem->getWidth() +
                                       stackZoneGraphicsItem->boundingRect().width());
    } else {
        player->getPlayerInfo()->setHandVisible(true);

        handZoneGraphicsItem->setPos(base);
        base += QPointF(handZoneGraphicsItem->boundingRect().width(), 0);

        stackZoneGraphicsItem->setPos(base);
        base += QPointF(stackZoneGraphicsItem->boundingRect().width(), 0);

        tableZoneGraphicsItem->setPos(base);
    }
    handZoneGraphicsItem->setVisible(player->getPlayerInfo()->getHandVisible());
    handZoneGraphicsItem->updateOrientation();
    tableZoneGraphicsItem->reorganizeCards();
    updateBoundingRect();
    rearrangeCounters();
}

void PlayerGraphicsItem::updateBoundingRect()
{
    prepareGeometryChange();
    qreal width = CARD_HEIGHT + 15 + counterAreaWidth + stackZoneGraphicsItem->boundingRect().width();
    if (SettingsCache::instance().getHorizontalHand()) {
        qreal handHeight =
            player->getPlayerInfo()->getHandVisible() ? handZoneGraphicsItem->boundingRect().height() : 0;
        bRect = QRectF(0, 0, width + tableZoneGraphicsItem->boundingRect().width(),
                       tableZoneGraphicsItem->boundingRect().height() + handHeight);
    } else {
        bRect = QRectF(
            0, 0, width + handZoneGraphicsItem->boundingRect().width() + tableZoneGraphicsItem->boundingRect().width(),
            tableZoneGraphicsItem->boundingRect().height());
    }
    playerArea->setSize(CARD_HEIGHT + counterAreaWidth + 15, bRect.height());

    emit sizeChanged();
}