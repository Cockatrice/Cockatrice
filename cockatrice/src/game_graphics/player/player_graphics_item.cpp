#include "player_graphics_item.h"

#include "../../game/player/player_actions.h"
#include "../../interface/card_picture_loader/card_picture_loader.h"
#include "../../interface/widgets/cards/art_crop_attribution.h"
#include "../../interface/widgets/playmat/playmat_utils.h"
#include "../../interface/widgets/tabs/tab_game.h"
#include "../board/abstract_card_item.h"
#include "../board/counter_general.h"
#include "../hand_counter.h"
#include "../zones/hand_zone.h"
#include "../zones/pile_zone.h"
#include "../zones/stack_zone.h"
#include "../zones/table_zone.h"
#include "menu/player_menu.h"
#include "player_dialogs.h"

#include <QGraphicsView>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/playmat_resolver.h>
#include <libcockatrice/settings/interface_settings.h>

PlayerGraphicsItem::PlayerGraphicsItem(PlayerLogic *_player) : player(_player)
{
    connect(&SettingsCache::instance().userInterface(), &InterfaceSettings::horizontalHandChanged, this,
            &PlayerGraphicsItem::rearrangeZones);
    connect(&SettingsCache::instance().userInterface(), &InterfaceSettings::handJustificationChanged, this,
            &PlayerGraphicsItem::rearrangeZones);
    connect(player, &PlayerLogic::rearrangeCounters, this, &PlayerGraphicsItem::rearrangeCounters);
    connect(player, &PlayerLogic::activeChanged, this, &PlayerGraphicsItem::onPlayerActiveChanged);
    connect(player, &PlayerLogic::concededChanged, this, [this](int, bool c) { setVisible(!c); });
    connect(player, &PlayerLogic::zoneIdChanged, this, [this](int id) { playerArea->setPlayerZoneId(id); });

    connect(player, &PlayerLogic::counterAdded, this, &PlayerGraphicsItem::onCounterAdded);
    connect(player, &PlayerLogic::counterRemoved, this, &PlayerGraphicsItem::onCounterRemoved);
    connect(player, &PlayerLogic::deckChanged, this, &PlayerGraphicsItem::updatePlaymat);
    connect(player, &PlayerLogic::playmatChanged, this, &PlayerGraphicsItem::updatePlaymat);
    connect(&SettingsCache::instance().userInterface(), &InterfaceSettings::playmatVisibilityChanged, this,
            [this](int) { updatePlaymat(); });

    playerMenu = new PlayerMenu(this);

    connect(playerMenu, &PlayerMenu::shortcutsActivated, this, [this]() {
        for (auto *ctr : counterWidgets) {
            ctr->setShortcutsActive();
        }
    });
    connect(playerMenu, &PlayerMenu::shortcutsDeactivated, this, [this]() {
        for (auto *ctr : counterWidgets) {
            ctr->setShortcutsInactive();
        }
    });
    connect(playerMenu, &PlayerMenu::retranslateRequested, this, [this]() {
        for (auto *ctr : counterWidgets) {
            ctr->retranslateUi();
        }
    });

    playerDialogs = new PlayerDialogs(this, player->getPlayerActions());

    connect(playerDialogs, &PlayerDialogs::requestDialogSemaphore, player, &PlayerLogic::setDialogSemaphore);

    playerArea = new PlayerArea(this);

    playerTarget = new PlayerTarget(player, playerArea);
    qreal avatarMargin =
        (counterAreaWidth + CardDimensions::HEIGHT_F + 15 - playerTarget->boundingRect().width()) / 2.0;
    playerTarget->setPos(QPointF(avatarMargin, avatarMargin));

    initializeZones();

    connect(player, &PlayerLogic::addViewCustomZoneActionToCustomZoneMenu, this,
            &PlayerGraphicsItem::onCustomZoneAdded);

    playerMenu->setMenusForGraphicItems();

    connect(tableZoneGraphicsItem, &TableZone::sizeChanged, this, &PlayerGraphicsItem::updateBoundingRect);

    connect(this, &PlayerGraphicsItem::playmatChanged, tableZoneGraphicsItem, &TableZone::onPlaymatChanged);
    connect(this, &PlayerGraphicsItem::playmatChanged, stackZoneGraphicsItem, &StackZone::onPlaymatChanged);

    updateBoundingRect();

    rearrangeZones();
    retranslateUi();
}

void PlayerGraphicsItem::retranslateUi()
{
    playerMenu->retranslateUi();

    QMapIterator<QString, CardZoneLogic *> zoneIterator(player->getZones());
    while (zoneIterator.hasNext()) {
        emit zoneIterator.next().value()->retranslateUi();
    }
}

void PlayerGraphicsItem::onPlayerActiveChanged(bool _active)
{
    tableZoneGraphicsItem->setActive(_active);
}

void PlayerGraphicsItem::initializeZones()
{
    deckZoneGraphicsItem = new PileZone(player->getDeckZone(), this);
    auto base = QPointF(counterAreaWidth + (CardDimensions::HEIGHT_F - CardDimensions::WIDTH_F + 15) / 2.0,
                        10 + playerTarget->boundingRect().height() + 5 -
                            (CardDimensions::HEIGHT_F - CardDimensions::WIDTH_F) / 2.0);
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

    tableZoneGraphicsItem = new TableZone(player->getTableZone(), mirrored, this);
    connect(this, &PlayerGraphicsItem::mirroredChanged, tableZoneGraphicsItem, &TableZone::setMirrored);

    stackZoneGraphicsItem =
        new StackZone(player->getStackZone(), static_cast<int>(tableZoneGraphicsItem->boundingRect().height()), this);

    handZoneGraphicsItem =
        new HandZone(player->getHandZone(), static_cast<int>(tableZoneGraphicsItem->boundingRect().height()), this);
    connect(player->getPlayerActions(), &PlayerActions::requestSortHand, handZoneGraphicsItem, &HandZone::sortHand);

    connect(handZoneGraphicsItem->getLogic(), &HandZoneLogic::cardCountChanged, handCounter,
            &HandCounter::updateNumber);
    connect(handCounter, &HandCounter::showContextMenu, handZoneGraphicsItem, &HandZone::showContextMenu);

    zoneGraphicsItems.insert(player->getDeckZone()->getName(), deckZoneGraphicsItem);
    zoneGraphicsItems.insert(player->getGraveZone()->getName(), graveyardZoneGraphicsItem);
    zoneGraphicsItems.insert(player->getRfgZone()->getName(), rfgZoneGraphicsItem);
    zoneGraphicsItems.insert(player->getSideboardZone()->getName(), sideboardGraphicsItem);
    zoneGraphicsItems.insert(player->getTableZone()->getName(), tableZoneGraphicsItem);
    zoneGraphicsItems.insert(player->getStackZone()->getName(), stackZoneGraphicsItem);
    zoneGraphicsItems.insert(player->getHandZone()->getName(), handZoneGraphicsItem);
}

void PlayerGraphicsItem::onCustomZoneAdded(QString customZoneName)
{
    zoneGraphicsItems.insert(customZoneName, nullptr); // Custom zone view goes here, if we ever implement it.
}

QRectF PlayerGraphicsItem::boundingRect() const
{
    return bRect;
}

qreal PlayerGraphicsItem::getMinimumWidth() const
{
    qreal result = tableZoneGraphicsItem->getMinimumWidth() + CardDimensions::HEIGHT_F + 15 + counterAreaWidth +
                   stackZoneGraphicsItem->boundingRect().width();
    if (!SettingsCache::instance().userInterface().getHorizontalHand()) {
        result += handZoneGraphicsItem->boundingRect().width();
    }
    return result;
}

void PlayerGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!hasPlaymat || playmatPixmap.isNull()) {
        return;
    }

    // Calculate the combined bounding rect of stack + table zones
    QPointF stackPos = stackZoneGraphicsItem->pos();
    QPointF tablePos = tableZoneGraphicsItem->pos();
    QSizeF stackSize = stackZoneGraphicsItem->boundingRect().size();
    QSizeF tableSize = tableZoneGraphicsItem->boundingRect().size();

    // Combined area: from stack left edge to table right edge
    double combinedLeft = qMin(stackPos.x(), tablePos.x());
    double combinedTop = qMin(stackPos.y(), tablePos.y());
    double combinedRight = qMax(stackPos.x() + stackSize.width(), tablePos.x() + tableSize.width());
    double combinedBottom = qMax(stackPos.y() + stackSize.height(), tablePos.y() + tableSize.height());

    QRectF combinedArea(combinedLeft, combinedTop, combinedRight - combinedLeft, combinedBottom - combinedTop);

    const QRectF srcRect = computeArtSourceRect(playmatPixmap.size(), playmatParams);
    const QRectF dstRect = coverFitRect(combinedArea, srcRect.size());

    painter->save();
    painter->setClipRect(combinedArea);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    // Render from a down-scaled copy of the art so the full-resolution source
    // pixmap is never re-sampled at a tiny device size (also much cheaper than
    // scaling it on every frame).
    const QPixmap scaledPixmap = scaledPlaymatFor(srcRect, painter->worldTransform().mapRect(dstRect).size());
    painter->drawPixmap(dstRect, scaledPixmap, QRectF(scaledPixmap.rect()));

    painter->restore();

    if (!playmatAttribution.isEmpty()) {
        paintArtAttribution(*painter, combinedArea, playmatAttribution, Qt::AlignRight | Qt::AlignBottom, 0.8);
    }
}

QPixmap PlayerGraphicsItem::scaledPlaymatFor(const QRectF &srcRect, const QSizeF &deviceDstSize)
{
    // Bucket the render size so the source pixmap is re-scaled at most once per
    // zoom step instead of once per frame.
    constexpr int bucketSize = 32;
    const QSize target = QSize(qMax(1, qRound(deviceDstSize.width() / bucketSize) * bucketSize),
                               qMax(1, qRound(deviceDstSize.height() / bucketSize) * bucketSize))
                             .boundedTo(srcRect.toAlignedRect().size());

    if (scaledPlaymatKey != target) {
        const QPixmap crop = playmatPixmap.copy(srcRect.toAlignedRect());
        scaledPlaymatPixmap = crop.scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        scaledPlaymatKey = target;
    }
    return scaledPlaymatPixmap;
}

void PlayerGraphicsItem::processSceneSizeChange(int newPlayerWidth)
{
    // Extend table (and hand, if horizontal) to accommodate the new player width.
    qreal tableWidth = newPlayerWidth - CardDimensions::HEIGHT_F - 15 - counterAreaWidth -
                       stackZoneGraphicsItem->boundingRect().width();
    if (!SettingsCache::instance().userInterface().getHorizontalHand()) {
        tableWidth -= handZoneGraphicsItem->boundingRect().width();
    }

    tableZoneGraphicsItem->setWidth(tableWidth);
    handZoneGraphicsItem->setWidth(tableWidth + stackZoneGraphicsItem->boundingRect().width());
}

void PlayerGraphicsItem::setMirrored(bool _mirrored)
{
    if (mirrored != _mirrored) {
        mirrored = _mirrored;
        emit mirroredChanged(mirrored);
        rearrangeZones();
    }
}

void PlayerGraphicsItem::onCounterAdded(CounterState *state)
{
    AbstractCounter *widget;
    if (state->getName() == "life") {
        widget = playerTarget->addCounter(state);
        connect(state, &CounterState::valueChanged, this, [this](int oldValue, int newValue) {
            if (newValue < oldValue) {
                tableZoneGraphicsItem->triggerDamageShimmer();
            }
        });
    } else {
        widget = new GeneralCounter(state, player, true, this);
    }
    counterWidgets.insert(state->getId(), widget);

    if (playerMenu->getCountersMenu() && widget->getMenu()) {
        playerMenu->getCountersMenu()->addMenu(widget->getMenu());
    }

    if (playerMenu->getShortcutsActive()) {
        widget->setShortcutsActive();
    }

    rearrangeCounters();
}

void PlayerGraphicsItem::onCounterRemoved(int counterId)
{
    auto *widget = counterWidgets.take(counterId);
    if (!widget) {
        return;
    }
    if (playerMenu->getCountersMenu() && widget->getMenu()) {
        playerMenu->getCountersMenu()->removeAction(widget->getMenu()->menuAction());
    }
    widget->delCounter();
    rearrangeCounters();
}

void PlayerGraphicsItem::rearrangeCounters()
{
    qreal ySize = boundingRect().y() + 80;
    constexpr qreal padding = 5;
    for (auto *ctr : counterWidgets.values()) {
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
    auto base = QPointF(CardDimensions::HEIGHT_F + counterAreaWidth + 15, 0);
    if (SettingsCache::instance().userInterface().getHorizontalHand()) {
        if (mirrored) {
            if (player->getHandZone()->contentsKnown()) {
                handVisible = true;
                handZoneGraphicsItem->setPos(base);
                base += QPointF(0, handZoneGraphicsItem->boundingRect().height());
            } else {
                handVisible = false;
            }

            stackZoneGraphicsItem->setPos(base);
            base += QPointF(stackZoneGraphicsItem->boundingRect().width(), 0);

            tableZoneGraphicsItem->setPos(base);
        } else {
            stackZoneGraphicsItem->setPos(base);

            tableZoneGraphicsItem->setPos(base.x() + stackZoneGraphicsItem->boundingRect().width(), 0);
            base += QPointF(0, tableZoneGraphicsItem->boundingRect().height());

            if (player->getHandZone()->contentsKnown()) {
                handVisible = true;
                handZoneGraphicsItem->setPos(base);
            } else {
                handVisible = false;
            }
        }
        handZoneGraphicsItem->setWidth(tableZoneGraphicsItem->getWidth() +
                                       stackZoneGraphicsItem->boundingRect().width());
    } else {
        handVisible = true;

        handZoneGraphicsItem->setPos(base);
        base += QPointF(handZoneGraphicsItem->boundingRect().width(), 0);

        stackZoneGraphicsItem->setPos(base);
        base += QPointF(stackZoneGraphicsItem->boundingRect().width(), 0);

        tableZoneGraphicsItem->setPos(base);
    }
    handZoneGraphicsItem->setVisible(handVisible);
    handZoneGraphicsItem->updateOrientation();
    tableZoneGraphicsItem->reorganizeCards();
    updateBoundingRect();
    rearrangeCounters();
}

void PlayerGraphicsItem::updateBoundingRect()
{
    prepareGeometryChange();
    qreal width = CardDimensions::HEIGHT_F + 15 + counterAreaWidth + stackZoneGraphicsItem->boundingRect().width();
    if (SettingsCache::instance().userInterface().getHorizontalHand()) {
        qreal handHeight = handVisible ? handZoneGraphicsItem->boundingRect().height() : 0;
        bRect = QRectF(0, 0, width + tableZoneGraphicsItem->boundingRect().width(),
                       tableZoneGraphicsItem->boundingRect().height() + handHeight);
    } else {
        bRect = QRectF(
            0, 0, width + handZoneGraphicsItem->boundingRect().width() + tableZoneGraphicsItem->boundingRect().width(),
            tableZoneGraphicsItem->boundingRect().height());
    }
    playerArea->setSize(CardDimensions::HEIGHT_F + counterAreaWidth + 15, bRect.height());

    emit sizeChanged();
}

void PlayerGraphicsItem::updatePlaymat()
{
    int visibility = SettingsCache::instance().userInterface().getPlaymatVisibility();

    // "Don't use playmats" — never show
    if (visibility == 0) {
        clearPlaymat();
        return;
    }

    // "Show own playmat only" — hide playmats for remote players
    if (visibility == 1 && !player->getPlayerInfo()->getLocal()) {
        clearPlaymat();
        return;
    }

    CardRef playmatCard;
    PlaymatParams params;

    if (player->getHasRemotePlaymat()) {
        // Prefer the server-confirmed playmat (updated by Command_SetPlaymat).
        playmatCard = player->getRemotePlaymatCard();
        params = player->getRemotePlaymatParams();
    } else if (player->getPlayerInfo()->getLocal()) {
        // Local player without a server broadcast yet: apply the full
        // settings-based resolution chain (mode, fallback list, behavior).
        const auto &settings = SettingsCache::instance().userInterface();
        const DeckList &deck = player->getDeck();
        const auto fallbackBehavior = static_cast<PlaymatFallbackMode>(settings.getPlaymatFallbackBehavior());

        switch (settings.getPlaymatMode()) {
            case 0: { // Override deck playmat — always use collection
                DeckList emptyDeck;
                const PlaymatResolution resolved =
                    resolveEffectivePlaymat(emptyDeck, {}, settings.getPlaymatFallbackList(), fallbackBehavior, 0);
                playmatCard = resolved.card;
                params = resolved.params;
                break;
            }
            case 1: { // Fallback if deck has none — deck > collection > none
                const PlaymatResolution resolved =
                    resolveEffectivePlaymat(deck, {}, settings.getPlaymatFallbackList(), fallbackBehavior, 0);
                playmatCard = resolved.card;
                params = resolved.params;
                break;
            }
            case 2: { // Deck only, ignore collection
                const PlaymatResolution &deckPlaymat = deck.getPlaymat();
                if (!deckPlaymat.card.isEmpty()) {
                    playmatCard = deckPlaymat.card;
                    params = deckPlaymat.params;
                }
                break;
            }
        }
    } else {
        // Opponent without a server broadcast: use the deck-embedded playmat.
        const DeckList &deck = player->getDeck();
        const PlaymatResolution &deckPlaymat = deck.getPlaymat();
        if (!deckPlaymat.card.isEmpty()) {
            playmatCard = deckPlaymat.card;
            params = deckPlaymat.params;
        }
    }

    if (playmatCard.isEmpty()) {
        clearPlaymat();
        return;
    }

    playmatParams = params;
    scaledPlaymatKey = QSize(); // the art crop depends on the params, drop any cached scale

    ExactCard card = CardDatabaseManager::query()->getCard(playmatCard);
    if (!card) {
        clearPlaymat();
        return;
    }

    playmatAttribution = buildArtAttribution(card);

    QPixmap fullRes;
    CardPictureLoader::getPixmap(fullRes, card, QSize(745, 1040));

    if (fullRes.isNull()) {
        disconnect(playmatPixmapConnection);
        CardInfo *cardInfo = card.getCardPtr().data();
        if (cardInfo) {
            playmatPixmapConnection =
                connect(cardInfo, &CardInfo::pixmapUpdated, this, &PlayerGraphicsItem::onPlaymatPixmapReady);
        }
        return;
    }

    if (!hasPlaymat) {
        hasPlaymat = true;
        emit playmatChanged(true);
    }
    playmatPixmap = fullRes;
    update();
}

void PlayerGraphicsItem::clearPlaymat()
{
    disconnect(playmatPixmapConnection);
    playmatAttribution.clear();
    if (hasPlaymat) {
        hasPlaymat = false;
        playmatPixmap = QPixmap();
        scaledPlaymatKey = QSize();
        emit playmatChanged(false);
        update();
    }
}

void PlayerGraphicsItem::onPlaymatPixmapReady()
{
    updatePlaymat();
}
