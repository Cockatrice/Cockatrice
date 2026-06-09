#include "player_logic.h"

#include "../../game_graphics/board/arrow_item.h"
#include "../../game_graphics/board/card_item.h"
#include "../../game_graphics/board/commander_tax_counter.h"
#include "../../game_graphics/board/counter_general.h"
#include "../../game_graphics/game_scene.h"
#include "../../game_graphics/player/player_target.h"
#include "../../game_graphics/zones/command_zone.h"
#include "../../game_graphics/zones/hand_zone.h"
#include "../../game_graphics/zones/pile_zone.h"
#include "../../game_graphics/zones/stack_zone.h"
#include "../../game_graphics/zones/table_zone.h"
#include "../../interface/theme_manager.h"
#include "../../interface/widgets/tabs/tab_game.h"
#include "../board/card_list.h"
#include "player_actions.h"

#include <QDebug>
#include <QMenu>
#include <QMetaType>
#include <QPainter>
#include <QtConcurrent>
#include <libcockatrice/protocol/pb/command_attach_card.pb.h>
#include <libcockatrice/protocol/pb/command_set_card_counter.pb.h>
#include <libcockatrice/protocol/pb/event_create_arrow.pb.h>
#include <libcockatrice/protocol/pb/event_create_counter.pb.h>
#include <libcockatrice/protocol/pb/event_draw_cards.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_player.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_zone.pb.h>
#include <libcockatrice/utility/color.h>
#include <libcockatrice/utility/counter_ids.h>

PlayerLogic::PlayerLogic(const ServerInfo_User &info, int _id, bool _local, bool _judge, AbstractGame *_parent)
    : QObject(_parent), game(_parent), playerInfo(new PlayerInfo(info, _id, _local, _judge)),
      playerEventHandler(new PlayerEventHandler(this)), playerActions(new PlayerActions(this)), active(false),
      conceded(false), zoneId(0), dialogSemaphore(false), serverHasCommandZone(false)
{
    initializeZones();
}

void PlayerLogic::initializeZones()
{
    addZone(new PileZoneLogic(this, ZoneNames::DECK, false, true, false, this));
    addZone(new PileZoneLogic(this, ZoneNames::GRAVE, false, false, true, this));
    addZone(new PileZoneLogic(this, ZoneNames::EXILE, false, false, true, this));
    addZone(new PileZoneLogic(this, ZoneNames::SIDEBOARD, false, false, false, this));
    addZone(new TableZoneLogic(this, ZoneNames::TABLE, true, false, true, this));
    addZone(new StackZoneLogic(this, ZoneNames::STACK, true, false, true, this));
    bool visibleHand = playerInfo->getLocalOrJudge() ||
                       (game->getPlayerManager()->isSpectator() && game->getGameMetaInfo()->spectatorsOmniscient());
    addZone(new HandZoneLogic(this, ZoneNames::HAND, false, false, visibleHand, this));
    addZone(new CommandZoneLogic(this, ZoneNames::COMMAND, true, false, true, this));
}

PlayerLogic::~PlayerLogic()
{
    qCInfo(PlayerLog) << "Player destructor:" << getPlayerInfo()->getName();

    QMapIterator<QString, CardZoneLogic *> i(zones);
    while (i.hasNext()) {
        delete i.next().value();
    }
    zones.clear();

    delete getPlayerInfo()->userInfo;
}

void PlayerLogic::clear()
{
    emit arrowsClearedLocally();

    QMapIterator<QString, CardZoneLogic *> i(zones);
    while (i.hasNext()) {
        i.next().value()->clearContents();
    }

    clearCounters();
}

void PlayerLogic::setConceded(bool _conceded)
{
    if (conceded != _conceded) {
        conceded = _conceded;

        if (conceded) {
            clear();
        }
        emit concededChanged(getPlayerInfo()->getId(), conceded);
    }
}

void PlayerLogic::setZoneId(int _zoneId)
{
    if (zoneId != _zoneId) {
        zoneId = _zoneId;
        emit zoneIdChanged(zoneId);
    }
}

void PlayerLogic::processPlayerInfo(const ServerInfo_Player &info)
{
    static QSet<QString> builtinZones{/* PileZones */
                                      ZoneNames::DECK, ZoneNames::GRAVE, ZoneNames::EXILE, ZoneNames::SIDEBOARD,
                                      /* TableZone */
                                      ZoneNames::TABLE,
                                      /* StackZone */
                                      ZoneNames::STACK,
                                      /* HandZone */
                                      ZoneNames::HAND,
                                      /* CommandZone */
                                      ZoneNames::COMMAND};
    clearCounters();
    emit arrowsClearedLocally();

    QMutableMapIterator<QString, CardZoneLogic *> zoneIt(zones);
    while (zoneIt.hasNext()) {
        zoneIt.next().value()->clearContents();

        if (!builtinZones.contains(zoneIt.key())) {
            zoneIt.remove();
        }
    }

    emit clearCustomZonesMenu();

    // Check if server has command zone by scanning the zone list
    const int zoneListSize = info.zone_list_size();
    bool foundCommandZone = false;
    for (int i = 0; i < zoneListSize; ++i) {
        if (QString::fromStdString(info.zone_list(i).name()) == ZoneNames::COMMAND) {
            foundCommandZone = true;
            break;
        }
    }
    if (serverHasCommandZone != foundCommandZone) {
        serverHasCommandZone = foundCommandZone;
        emit commandZoneSupportChanged(foundCommandZone);
    }
    for (int i = 0; i < zoneListSize; ++i) {
        const ServerInfo_Zone &zoneInfo = info.zone_list(i);

        QString zoneName = QString::fromStdString(zoneInfo.name());
        CardZoneLogic *zone = zones.value(zoneName, 0);
        if (!zone) {
            // Create a new CardZone if it doesn't exist

            if (zoneInfo.with_coords()) {
                // Visibility not currently supported for TableZone
                zone = addZone(new TableZoneLogic(this, zoneName, true, false, true, this));
            } else {
                // Zones without coordinats are always treated as non-shufflable
                // PileZones, although supporting alternate hand or stack zones
                // might make sense in some scenarios.
                bool contentsKnown;

                switch (zoneInfo.type()) {
                    case ServerInfo_Zone::PrivateZone:
                        contentsKnown =
                            playerInfo->getLocalOrJudge() || (game->getPlayerManager()->isSpectator() &&
                                                              game->getGameMetaInfo()->spectatorsOmniscient());
                        break;

                    case ServerInfo_Zone::PublicZone:
                        contentsKnown = true;
                        break;

                    case ServerInfo_Zone::HiddenZone:
                        contentsKnown = false;
                        break;
                }

                zone = addZone(new PileZoneLogic(this, zoneName, false, /* isShufflable */ false, contentsKnown, this));
            }

            // Non-builtin zones are hidden by default and can't be interacted
            // with, except through menus.
            emit zone->setGraphicsVisibility(false);

            emit addViewCustomZoneActionToCustomZoneMenu(zoneName);

            continue;
        }

        const int cardListSize = zoneInfo.card_list_size();
        if (!cardListSize) {
            for (int j = 0; j < zoneInfo.card_count(); ++j) {
                zone->addCard(new CardItem(this), false, -1);
            }
        } else {
            for (int j = 0; j < cardListSize; ++j) {
                const ServerInfo_Card &cardInfo = zoneInfo.card_list(j);
                auto *card = new CardItem(this);
                card->processCardInfo(cardInfo);
                zone->addCard(card, false, cardInfo.x(), cardInfo.y());
            }
        }
        if (zoneInfo.has_always_reveal_top_card()) {
            zone->setAlwaysRevealTopCard(zoneInfo.always_reveal_top_card());
        }

        zone->reorganizeCards();
    }

    const int counterListSize = info.counter_list_size();
    for (int i = 0; i < counterListSize; ++i) {
        addCounter(info.counter_list(i));
    }

    setConceded(info.properties().conceded());
}

void PlayerLogic::processCardAttachment(const ServerInfo_Player &info)
{
    const int zoneListSize = info.zone_list_size();
    for (int i = 0; i < zoneListSize; ++i) {
        const ServerInfo_Zone &zoneInfo = info.zone_list(i);
        CardZoneLogic *zone = zones.value(QString::fromStdString(zoneInfo.name()), 0);
        if (!zone) {
            continue;
        }

        const int cardListSize = zoneInfo.card_list_size();
        for (int j = 0; j < cardListSize; ++j) {
            const ServerInfo_Card &cardInfo = zoneInfo.card_list(j);
            if (cardInfo.has_attach_player_id()) {
                CardItem *startCard = zone->getCard(cardInfo.id());
                CardItem *targetCard =
                    game->getCard(cardInfo.attach_player_id(), QString::fromStdString(cardInfo.attach_zone()),
                                  cardInfo.attach_card_id());
                if (!targetCard) {
                    continue;
                }

                startCard->setAttachedTo(targetCard);
            }
        }
    }

    const int arrowListSize = info.arrow_list_size();
    for (int i = 0; i < arrowListSize; ++i) {
        emit arrowCreateRequested(QSharedPointer<ArrowData>::create(
            ArrowData::fromProto(info.arrow_list(i), getPlayerInfo()->getId(), getPlayerInfo()->getLocal())));
    }
}

void PlayerLogic::addCard(CardItem *card)
{
    emit newCardAdded(card);
}

void PlayerLogic::deleteCard(CardItem *card)
{
    if (card == nullptr) {
        return;
    } else if (dialogSemaphore) {
        cardsToDelete.append(card);
    } else {
        card->deleteLater();
    }
}

void PlayerLogic::setDeck(const DeckList &_deck)
{
    deck = _deck;

    emit deckChanged();
}

CounterState *PlayerLogic::addCounter(const ServerInfo_Counter &counter)
{
    return addCounter(counter.id(), QString::fromStdString(counter.name()),
                      convertColorToQColor(counter.counter_color()), counter.radius(), counter.count(),
                      counter.active());
}

CounterState *
PlayerLogic::addCounter(int id, const QString &name, const QColor &color, int radius, int value, bool active)
{
    if (counters.contains(id)) {
        return nullptr;
    }
    auto *state = new CounterState(id, name, color, radius, value, active, this);
    counters.insert(id, state);
    emit counterAdded(state);
    return state;
}

void PlayerLogic::delCounter(int id)
{
    auto *state = counters.take(id);
    if (!state) {
        return;
    }
    emit counterRemoved(id);
    state->deleteLater();
}

void PlayerLogic::clearCounters()
{
    for (int id : counters.keys()) {
        emit counterRemoved(id);
    }
    qDeleteAll(counters);
    counters.clear();
}

CounterState *PlayerLogic::getLifeCounter() const
{
    for (auto *s : counters.values()) {
        if (s->getName() == "life") {
            return s;
        }
    }
    return nullptr;
}

AbstractCounter *PlayerLogic::getCounterWidget(int counterId) const
{
    Q_UNUSED(counterId);
    return nullptr;
    // TODO: Do not reach into graphics like this return graphicsItem->getCounterWidget(counterId);
}

bool PlayerLogic::clearCardsToDelete()
{
    if (cardsToDelete.isEmpty()) {
        return false;
    }

    for (auto &i : cardsToDelete) {
        if (i != nullptr) {
            i->deleteLater();
        }
    }
    cardsToDelete.clear();

    return true;
}

void PlayerLogic::setActive(bool _active)
{
    active = _active;
    emit activeChanged(active);
}
void PlayerLogic::onRequestZoneViewToggle(const QString &zoneName, int numberCards, bool isReversed)
{
    emit requestZoneViewToggle(this, zoneName, numberCards, isReversed);
}

void PlayerLogic::updateZones()
{
    getTableZone()->reorganizeCards();
}

void PlayerLogic::setGameStarted()
{
    if (playerInfo->local) {
        emit resetTopCardMenuActions();
    }
    setConceded(false);
}
