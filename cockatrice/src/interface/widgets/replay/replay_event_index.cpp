#include "replay_event_index.h"

#include "../../../game/phase.h"
#include "libcockatrice/protocol/get_pb_extension.h"
#include "libcockatrice/protocol/pb/card_attributes.pb.h"
#include "libcockatrice/protocol/pb/context_concede.pb.h"
#include "libcockatrice/protocol/pb/context_connection_state_changed.pb.h"
#include "libcockatrice/protocol/pb/context_deck_select.pb.h"
#include "libcockatrice/protocol/pb/context_mulligan.pb.h"
#include "libcockatrice/protocol/pb/context_ready_start.pb.h"
#include "libcockatrice/protocol/pb/context_set_sideboard_lock.pb.h"
#include "libcockatrice/protocol/pb/context_undo_draw.pb.h"
#include "libcockatrice/protocol/pb/event_attach_card.pb.h"
#include "libcockatrice/protocol/pb/event_change_zone_properties.pb.h"
#include "libcockatrice/protocol/pb/event_create_counter.pb.h"
#include "libcockatrice/protocol/pb/event_create_token.pb.h"
#include "libcockatrice/protocol/pb/event_del_counter.pb.h"
#include "libcockatrice/protocol/pb/event_destroy_card.pb.h"
#include "libcockatrice/protocol/pb/event_draw_cards.pb.h"
#include "libcockatrice/protocol/pb/event_dump_zone.pb.h"
#include "libcockatrice/protocol/pb/event_flip_card.pb.h"
#include "libcockatrice/protocol/pb/event_game_closed.pb.h"
#include "libcockatrice/protocol/pb/event_game_host_changed.pb.h"
#include "libcockatrice/protocol/pb/event_game_log_notice.pb.h"
#include "libcockatrice/protocol/pb/event_game_say.pb.h"
#include "libcockatrice/protocol/pb/event_game_state_changed.pb.h"
#include "libcockatrice/protocol/pb/event_join.pb.h"
#include "libcockatrice/protocol/pb/event_kicked.pb.h"
#include "libcockatrice/protocol/pb/event_leave.pb.h"
#include "libcockatrice/protocol/pb/event_move_card.pb.h"
#include "libcockatrice/protocol/pb/event_player_properties_changed.pb.h"
#include "libcockatrice/protocol/pb/event_reveal_cards.pb.h"
#include "libcockatrice/protocol/pb/event_reverse_turn.pb.h"
#include "libcockatrice/protocol/pb/event_roll_die.pb.h"
#include "libcockatrice/protocol/pb/event_set_active_phase.pb.h"
#include "libcockatrice/protocol/pb/event_set_active_player.pb.h"
#include "libcockatrice/protocol/pb/event_set_card_attr.pb.h"
#include "libcockatrice/protocol/pb/event_set_card_counter.pb.h"
#include "libcockatrice/protocol/pb/event_set_counter.pb.h"
#include "libcockatrice/protocol/pb/event_shuffle.pb.h"
#include "libcockatrice/protocol/pb/game_event.pb.h"
#include "libcockatrice/protocol/pb/game_event_container.pb.h"
#include "libcockatrice/protocol/pb/game_event_context.pb.h"
#include "libcockatrice/protocol/pb/game_replay.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_card.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_counter.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_player.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_playerproperties.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_zone.pb.h"
#include "libcockatrice/utility/zone_names.h"

#include <QCoreApplication>
#include <QHash>
#include <QStringList>
#include <QtGlobal>
#include <algorithm>

namespace
{
struct PendingDetail
{
    QString text;
    QString badge;
    ReplayEventKind kind = ReplayEventKind::Action;
    QString actor;
};

QString formatTime(int timeMs)
{
    const int totalSeconds = qMax(0, timeMs) / 1000;
    const int hours = totalSeconds / 3600;
    const int minutes = (totalSeconds / 60) % 60;
    const int seconds = totalSeconds % 60;

    if (hours > 0) {
        return QStringLiteral("%1:%2:%3")
            .arg(hours)
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'));
    }

    return QStringLiteral("%1:%2").arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0'));
}

class ReplayEventIndexBuilder
{
    Q_DECLARE_TR_FUNCTIONS(ReplayEventIndex)

public:
    ReplayEventIndexBuilder(const GameReplay *replay, const QList<int> &timeline) : replay(replay), timeline(timeline)
    {
    }

    QList<ReplayEventEntry> build()
    {
        QList<ReplayEventEntry> entries;

        for (int i = 0; i < replay->event_list_size(); ++i) {
            const GameEventContainer &container = replay->event_list(i);
            const QList<PendingDetail> details = describeContainer(container);
            if (details.isEmpty()) {
                continue;
            }

            ReplayEventEntry entry;
            entry.eventIndex = i;
            entry.timeMs = timeline.value(i, static_cast<int>(container.seconds_elapsed()) * 1000);
            entry.timeLabel = formatTime(entry.timeMs);
            entry.kind = details.first().kind;
            entry.badge = details.first().badge;
            if (details.size() > 1) {
                entry.badge += QStringLiteral(" +%1").arg(details.size() - 1);
            }

            const QString sharedActor = details.first().actor;
            const bool mixedActors = std::any_of(details.cbegin(), details.cend(), [&sharedActor](const auto &detail) {
                return detail.actor != sharedActor;
            });

            QStringList detailTexts;
            for (const PendingDetail &detail : details) {
                if (mixedActors && !detail.actor.isEmpty()) {
                    detailTexts.append(tr("%1: %2").arg(detail.actor, detail.text));
                } else {
                    detailTexts.append(detail.text);
                }
            }
            entry.actor = mixedActors ? QString() : sharedActor;
            entry.detail = detailTexts.join(QStringLiteral("  •  "));

            entries.append(entry);
        }

        return entries;
    }

private:
    const GameReplay *replay;
    const QList<int> &timeline;

    QHash<int, QString> playerNames;
    QHash<int, bool> spectators;
    QHash<QString, QString> cardNames;
    QHash<QString, QString> counterNames;

    int activePlayerId = -1;
    bool gameStarted = false;
    bool sawGameState = false;

    static QString cardKey(int playerId, const QString &zone, int cardId)
    {
        return QStringLiteral("%1/%2/%3").arg(playerId).arg(zone).arg(cardId);
    }

    static QString counterKey(int playerId, int counterId)
    {
        return QStringLiteral("%1/%2").arg(playerId).arg(counterId);
    }

    QString playerName(int playerId) const
    {
        if (playerId < 0) {
            return tr("Game");
        }
        return playerNames.value(playerId, tr("Player %1").arg(playerId));
    }

    QString actorName(int playerId) const
    {
        if (playerId < 0) {
            return QString();
        }
        return playerNames.value(playerId, tr("Player %1").arg(playerId));
    }

    bool isSpectator(int playerId) const
    {
        return spectators.value(playerId, false);
    }

    void rememberPlayer(const ServerInfo_PlayerProperties &properties)
    {
        const int playerId = properties.player_id();
        if (playerId < 0) {
            return;
        }

        const QString name = QString::fromStdString(properties.user_info().name());
        if (!name.isEmpty()) {
            playerNames.insert(playerId, name);
        }
        spectators.insert(playerId, properties.spectator());
    }

    void rememberCard(int playerId, const QString &zone, int cardId, const QString &name)
    {
        if (cardId >= 0 && !name.isEmpty()) {
            cardNames.insert(cardKey(playerId, zone, cardId), name);
        }
    }

    void forgetCard(int playerId, const QString &zone, int cardId)
    {
        if (cardId >= 0) {
            cardNames.remove(cardKey(playerId, zone, cardId));
        }
    }

    QString cardName(int playerId, const QString &zone, int cardId) const
    {
        if (cardId < 0) {
            return QString();
        }
        return cardNames.value(cardKey(playerId, zone, cardId));
    }

    void rememberCounter(int playerId, const ServerInfo_Counter &counter)
    {
        const QString name = QString::fromStdString(counter.name());
        if (!name.isEmpty()) {
            counterNames.insert(counterKey(playerId, counter.id()), name);
        }
    }

    QString counterName(int playerId, int counterId) const
    {
        return counterNames.value(counterKey(playerId, counterId));
    }

    QString zoneName(const QString &zone) const
    {
        if (zone == ZoneNames::TABLE) {
            return tr("battlefield");
        }
        if (zone == ZoneNames::GRAVE) {
            return tr("graveyard");
        }
        if (zone == ZoneNames::EXILE) {
            return tr("exile");
        }
        if (zone == ZoneNames::HAND) {
            return tr("hand");
        }
        if (zone == ZoneNames::DECK) {
            return tr("library");
        }
        if (zone == ZoneNames::SIDEBOARD) {
            return tr("sideboard");
        }
        if (zone == ZoneNames::STACK) {
            return tr("stack");
        }
        return zone;
    }

    QString zonePhrase(int playerId, const QString &zone, int actorId) const
    {
        if (zone == ZoneNames::TABLE) {
            return tr("the battlefield");
        }
        if (zone == ZoneNames::EXILE) {
            return tr("exile");
        }
        if (zone == ZoneNames::STACK) {
            return tr("the stack");
        }

        const QString name = zoneName(zone);
        if (playerId < 0 || playerId == actorId) {
            return tr("their %1").arg(name);
        }
        return tr("%1's %2").arg(playerName(playerId), name);
    }

    QString formatCardList(const QStringList &names, int total) const
    {
        // Keep repeats: each entry is a drawn/revealed card, so the list length must agree with the count.
        QStringList listedNames;
        for (const QString &name : names) {
            if (!name.isEmpty()) {
                listedNames.append(name);
            }
        }

        if (listedNames.isEmpty()) {
            if (total <= 0) {
                return tr("no cards");
            }
            return total == 1 ? tr("1 card") : tr("%1 cards").arg(total);
        }

        if (total < listedNames.size()) {
            total = listedNames.size();
        }

        if (listedNames.size() > 3) {
            return tr("%1 and %2 more").arg(listedNames.mid(0, 3).join(QStringLiteral(", "))).arg(total - 3);
        }
        if (total > listedNames.size()) {
            return tr("%1 and %2 more").arg(listedNames.join(QStringLiteral(", "))).arg(total - listedNames.size());
        }

        return listedNames.join(QStringLiteral(", "));
    }

    void updateGameState(const Event_GameStateChanged &event)
    {
        for (int i = 0; i < event.player_list_size(); ++i) {
            const ServerInfo_Player &player = event.player_list(i);
            const int playerId = player.properties().player_id();
            rememberPlayer(player.properties());

            for (int j = 0; j < player.zone_list_size(); ++j) {
                const ServerInfo_Zone &zone = player.zone_list(j);
                for (int k = 0; k < zone.card_list_size(); ++k) {
                    const ServerInfo_Card &card = zone.card_list(k);
                    rememberCard(playerId, QString::fromStdString(zone.name()), card.id(),
                                 QString::fromStdString(card.name()));
                }
            }

            for (int j = 0; j < player.counter_list_size(); ++j) {
                rememberCounter(playerId, player.counter_list(j));
            }
        }

        if (event.has_active_player_id()) {
            activePlayerId = event.active_player_id();
        }
        if (event.has_game_started()) {
            gameStarted = event.game_started();
        }
    }

    QString describeMoveCard(const Event_MoveCard &event, int actorId, const GameEventContext &context)
    {
        const int startPlayerId = event.start_player_id() >= 0 ? event.start_player_id() : actorId;
        const int targetPlayerId = event.target_player_id() >= 0 ? event.target_player_id() : startPlayerId;
        const QString startZone = QString::fromStdString(event.start_zone());
        const QString targetZone = event.has_target_zone() ? QString::fromStdString(event.target_zone()) : startZone;

        QString name;
        if (event.has_card_name()) {
            name = QString::fromStdString(event.card_name());
        }
        if (name.isEmpty()) {
            name = cardName(startPlayerId, startZone, event.card_id());
        }

        const bool knownName = !name.isEmpty();
        if (name.isEmpty()) {
            name = event.face_down() ? tr("a face-down card") : tr("a card");
        }

        const int newCardId = event.has_new_card_id() ? event.new_card_id() : event.card_id();
        forgetCard(startPlayerId, startZone, event.card_id());
        if (knownName) {
            rememberCard(targetPlayerId, targetZone, newCardId, name);
        }

        if (startZone == targetZone) {
            return tr("moved %1 within %2.").arg(name, zonePhrase(targetPlayerId, targetZone, actorId));
        }
        if (context.HasExtension(Context_UndoDraw::ext)) {
            return tr("returned %1 to %2.").arg(name, zonePhrase(targetPlayerId, targetZone, actorId));
        }

        return tr("moved %1 from %2 to %3.")
            .arg(name, zonePhrase(startPlayerId, startZone, actorId), zonePhrase(targetPlayerId, targetZone, actorId));
    }

    QString describeSetCardAttribute(const Event_SetCardAttr &event, int actorId)
    {
        const QString zone = QString::fromStdString(event.zone_name());
        QString name;
        if (event.has_card_id()) {
            name = cardName(actorId, zone, event.card_id());
        }
        if (name.isEmpty()) {
            name = event.has_card_id() ? tr("a card") : tr("all cards");
        }

        const QString value = QString::fromStdString(event.attr_value());
        switch (event.attribute()) {
            case AttrTapped:
                return value == QLatin1String("1") ? tr("tapped %1.").arg(name) : tr("untapped %1.").arg(name);
            case AttrAttacking:
                return value == QLatin1String("1") ? tr("%1 is now attacking.").arg(name)
                                                   : tr("%1 is no longer attacking.").arg(name);
            case AttrFaceDown:
                return value == QLatin1String("1") ? tr("turned %1 face down.").arg(name)
                                                   : tr("turned %1 face up.").arg(name);
            case AttrColor:
                return tr("changed %1's color to %2.").arg(name, value);
            case AttrPT:
                return tr("set %1's power/toughness to %2.").arg(name, value);
            case AttrAnnotation:
                return value.isEmpty() ? tr("cleared %1's annotation.").arg(name)
                                       : tr("annotated %1: %2").arg(name, value);
            case AttrDoesntUntap:
                return value == QLatin1String("1") ? tr("%1 will no longer untap.").arg(name)
                                                   : tr("%1 will untap normally again.").arg(name);
        }

        return tr("changed %1.").arg(name);
    }

    QString badgeForCardAttribute(CardAttribute attribute) const
    {
        switch (attribute) {
            case AttrTapped:
                return tr("TAP");
            case AttrAttacking:
                return tr("ATTACK");
            case AttrFaceDown:
                return tr("FLIP");
            case AttrColor:
                return tr("COLOR");
            case AttrPT:
                return tr("PT");
            case AttrAnnotation:
                return tr("NOTE");
            case AttrDoesntUntap:
                return tr("TAP");
        }
        return tr("CARD");
    }

    QString describeLeaveReason(Event_Leave::LeaveReason reason) const
    {
        switch (reason) {
            case Event_Leave::USER_KICKED:
                return tr("kicked by game host or moderator");
            case Event_Leave::USER_LEFT:
                return tr("left the game");
            case Event_Leave::USER_DISCONNECTED:
                return tr("disconnected");
            case Event_Leave::OTHER:
            default:
                return tr("reason unknown");
        }
    }

    void addDetail(QList<PendingDetail> &details,
                   const QString &text,
                   const QString &badge,
                   ReplayEventKind kind,
                   int actorId,
                   bool forcedByJudge) const
    {
        if (text.isEmpty()) {
            return;
        }

        const QString finalText = forcedByJudge ? tr("%1 (judge)").arg(text) : text;
        details.append(PendingDetail{.text = finalText, .badge = badge, .kind = kind, .actor = actorName(actorId)});
    }

    QList<PendingDetail> describeContainer(const GameEventContainer &container)
    {
        QList<PendingDetail> details;
        const GameEventContext &context = container.context();
        const bool forcedByJudge = container.has_forced_by_judge();
        const auto contextType = static_cast<GameEventContext::ContextType>(getPbExtension(context));

        for (int i = 0; i < container.event_list_size(); ++i) {
            const GameEvent &event = container.event_list(i);
            const int eventPlayerId = event.player_id();
            const auto eventType = static_cast<GameEvent::GameEventType>(getPbExtension(event));

            switch (eventType) {
                case GameEvent::GAME_STATE_CHANGED: {
                    const Event_GameStateChanged &state = event.GetExtension(Event_GameStateChanged::ext);
                    const bool wasStarted = gameStarted;
                    updateGameState(state);
                    if (state.game_started() && !wasStarted) {
                        addDetail(details, tr("Game started."), tr("GAME"), ReplayEventKind::System, -1, forcedByJudge);
                    } else if (!sawGameState) {
                        addDetail(details, tr("Game state loaded."), tr("GAME"), ReplayEventKind::System, -1,
                                  forcedByJudge);
                    }
                    sawGameState = true;
                    break;
                }
                case GameEvent::PLAYER_PROPERTIES_CHANGED: {
                    const Event_PlayerPropertiesChanged &propertiesEvent =
                        event.GetExtension(Event_PlayerPropertiesChanged::ext);
                    const bool hasProperties = propertiesEvent.has_player_properties();
                    const ServerInfo_PlayerProperties &properties = propertiesEvent.player_properties();
                    // The server only fills in the field that changed, so player_id is absent from these events
                    // unless they are about a specific player; fall back to the event's own player id.
                    const bool describesPlayer = hasProperties && properties.has_player_id();
                    const int playerId = describesPlayer ? properties.player_id() : eventPlayerId;
                    if (describesPlayer) {
                        rememberPlayer(properties);
                    }

                    switch (contextType) {
                        case GameEventContext::READY_START:
                            addDetail(details,
                                      properties.ready_start() ? tr("is ready to start.") : tr("is no longer ready."),
                                      tr("READY"), ReplayEventKind::System, playerId, forcedByJudge);
                            break;
                        case GameEventContext::CONCEDE:
                            addDetail(details, tr("conceded the game."), tr("CONCEDE"), ReplayEventKind::System,
                                      playerId, forcedByJudge);
                            break;
                        case GameEventContext::UNCONCEDE:
                            addDetail(details, tr("unconceded the game."), tr("CONCEDE"), ReplayEventKind::System,
                                      playerId, forcedByJudge);
                            break;
                        case GameEventContext::DECK_SELECT: {
                            const Context_DeckSelect &deckSelect = context.GetExtension(Context_DeckSelect::ext);
                            const int sideboardSize = deckSelect.sideboard_size();
                            addDetail(details,
                                      sideboardSize >= 0
                                          ? tr("selected a deck with %1 sideboard cards.").arg(sideboardSize)
                                          : tr("selected a deck."),
                                      tr("DECK"), ReplayEventKind::System, playerId, forcedByJudge);
                            break;
                        }
                        case GameEventContext::MULLIGAN: {
                            const Context_Mulligan &mulligan = context.GetExtension(Context_Mulligan::ext);
                            addDetail(details,
                                      mulligan.number() > 0 ? tr("took a mulligan (%1).").arg(mulligan.number())
                                                            : tr("took a mulligan."),
                                      tr("MULLIGAN"), ReplayEventKind::System, playerId, forcedByJudge);
                            break;
                        }
                        case GameEventContext::SET_SIDEBOARD_LOCK:
                            addDetail(details,
                                      properties.sideboard_locked() ? tr("locked their sideboard.")
                                                                    : tr("unlocked their sideboard."),
                                      tr("SIDEBOARD"), ReplayEventKind::System, playerId, forcedByJudge);
                            break;
                        case GameEventContext::CONNECTION_STATE_CHANGED:
                            addDetail(details,
                                      properties.ping_seconds() != -1 ? tr("reconnected to the game.")
                                                                      : tr("lost connection to the game."),
                                      tr("CONNECTION"), ReplayEventKind::System, playerId, forcedByJudge);
                            break;
                        default:
                            break;
                    }
                    break;
                }
                case GameEvent::JOIN: {
                    const Event_Join &join = event.GetExtension(Event_Join::ext);
                    if (join.has_player_properties()) {
                        rememberPlayer(join.player_properties());
                    }
                    const int playerId = join.player_properties().player_id() >= 0
                                             ? join.player_properties().player_id()
                                             : eventPlayerId;
                    addDetail(details, isSpectator(playerId) ? tr("joined as a spectator.") : tr("joined the game."),
                              tr("JOIN"), ReplayEventKind::System, playerId, forcedByJudge);
                    break;
                }
                case GameEvent::LEAVE: {
                    const Event_Leave &leave = event.GetExtension(Event_Leave::ext);
                    const QString reason = describeLeaveReason(leave.reason());
                    if (eventPlayerId < 0) {
                        addDetail(details, tr("A player left the game (%1).").arg(reason), tr("LEAVE"),
                                  ReplayEventKind::System, -1, forcedByJudge);
                    } else {
                        addDetail(details,
                                  isSpectator(eventPlayerId) ? tr("stopped spectating (%1).").arg(reason)
                                                             : tr("left the game (%1).").arg(reason),
                                  tr("LEAVE"), ReplayEventKind::System, eventPlayerId, forcedByJudge);
                    }
                    break;
                }
                case GameEvent::KICKED:
                    addDetail(details,
                              eventPlayerId >= 0 ? tr("was kicked from the game.")
                                                 : tr("A player was kicked from the game."),
                              tr("KICK"), ReplayEventKind::System, eventPlayerId, forcedByJudge);
                    break;
                case GameEvent::GAME_CLOSED:
                    addDetail(details, tr("Game closed."), tr("GAME"), ReplayEventKind::System, -1, forcedByJudge);
                    break;
                case GameEvent::GAME_HOST_CHANGED:
                    addDetail(details, eventPlayerId >= 0 ? tr("is now the game host.") : tr("The game host changed."),
                              tr("HOST"), ReplayEventKind::System, eventPlayerId, forcedByJudge);
                    break;
                case GameEvent::GAME_SAY: {
                    const Event_GameSay &say = event.GetExtension(Event_GameSay::ext);
                    addDetail(details, QString::fromStdString(say.message()), tr("CHAT"), ReplayEventKind::Chat,
                              eventPlayerId, forcedByJudge);
                    break;
                }
                case GameEvent::SET_ACTIVE_PLAYER: {
                    const Event_SetActivePlayer &setActivePlayer = event.GetExtension(Event_SetActivePlayer::ext);
                    activePlayerId = setActivePlayer.active_player_id();
                    if (activePlayerId >= 0) {
                        addDetail(details, tr("begins their turn."), tr("TURN"), ReplayEventKind::Turn, activePlayerId,
                                  forcedByJudge);
                    }
                    break;
                }
                case GameEvent::SET_ACTIVE_PHASE: {
                    const Event_SetActivePhase &setActivePhase = event.GetExtension(Event_SetActivePhase::ext);
                    if (setActivePhase.phase() >= 0) {
                        const QString phaseName = Phases::getPhase(setActivePhase.phase()).getName();
                        addDetail(details, tr("entered the %1 phase.").arg(phaseName), tr("PHASE"),
                                  ReplayEventKind::Phase, activePlayerId >= 0 ? activePlayerId : eventPlayerId,
                                  forcedByJudge);
                    }
                    break;
                }
                case GameEvent::REVERSE_TURN: {
                    const Event_ReverseTurn &reverseTurn = event.GetExtension(Event_ReverseTurn::ext);
                    addDetail(details,
                              reverseTurn.reversed() ? tr("reversed the turn order.") : tr("restored the turn order."),
                              tr("TURN"), ReplayEventKind::Turn, eventPlayerId, forcedByJudge);
                    break;
                }
                case GameEvent::MOVE_CARD: {
                    const Event_MoveCard &moveCard = event.GetExtension(Event_MoveCard::ext);
                    const QString text = describeMoveCard(moveCard, eventPlayerId, context);
                    const QString startZone = QString::fromStdString(moveCard.start_zone());
                    const QString targetZone = QString::fromStdString(moveCard.target_zone());
                    QString badge = tr("MOVE");
                    if (context.HasExtension(Context_UndoDraw::ext)) {
                        badge = tr("UNDO");
                    } else if (startZone == ZoneNames::STACK) {
                        badge = tr("RESOLVE");
                    } else if (startZone == ZoneNames::HAND && targetZone == ZoneNames::TABLE) {
                        badge = tr("PLAY");
                    }
                    addDetail(details, text, badge, ReplayEventKind::Action, eventPlayerId, forcedByJudge);
                    break;
                }
                case GameEvent::DRAW_CARDS: {
                    const Event_DrawCards &drawCards = event.GetExtension(Event_DrawCards::ext);
                    QStringList names;
                    for (int j = 0; j < drawCards.cards_size(); ++j) {
                        const ServerInfo_Card &card = drawCards.cards(j);
                        const QString name = QString::fromStdString(card.name());
                        names.append(name);
                        rememberCard(eventPlayerId, ZoneNames::HAND, card.id(), name);
                    }
                    if (drawCards.number() > 0) {
                        addDetail(details, tr("drew %1.").arg(formatCardList(names, drawCards.number())), tr("DRAW"),
                                  ReplayEventKind::Action, eventPlayerId, forcedByJudge);
                    }
                    break;
                }
                case GameEvent::REVEAL_CARDS: {
                    const Event_RevealCards &revealCards = event.GetExtension(Event_RevealCards::ext);
                    const QString zone = QString::fromStdString(revealCards.zone_name());
                    QStringList names;
                    for (int j = 0; j < revealCards.cards_size(); ++j) {
                        const ServerInfo_Card &card = revealCards.cards(j);
                        const QString name = QString::fromStdString(card.name());
                        names.append(name);
                        rememberCard(eventPlayerId, zone, card.id(), name);
                    }
                    const int total =
                        revealCards.number_of_cards() > 0 ? revealCards.number_of_cards() : revealCards.cards_size();
                    QString text =
                        tr("revealed %1 from %2.")
                            .arg(formatCardList(names, total), zonePhrase(eventPlayerId, zone, eventPlayerId));
                    if (revealCards.other_player_id() >= 0 && revealCards.other_player_id() != eventPlayerId) {
                        text = tr("revealed %1 from %2 to %3.")
                                   .arg(formatCardList(names, total), zonePhrase(eventPlayerId, zone, eventPlayerId),
                                        playerName(revealCards.other_player_id()));
                    }
                    addDetail(details, text, tr("REVEAL"), ReplayEventKind::Action, eventPlayerId, forcedByJudge);
                    break;
                }
                case GameEvent::SHUFFLE: {
                    const Event_Shuffle &shuffle = event.GetExtension(Event_Shuffle::ext);
                    addDetail(
                        details,
                        tr("shuffled %1.")
                            .arg(zonePhrase(eventPlayerId, QString::fromStdString(shuffle.zone_name()), eventPlayerId)),
                        tr("SHUFFLE"), ReplayEventKind::Action, eventPlayerId, forcedByJudge);
                    break;
                }
                case GameEvent::ROLL_DIE: {
                    const Event_RollDie &rollDie = event.GetExtension(Event_RollDie::ext);
                    QStringList rolls;
                    for (int j = 0; j < rollDie.values_size(); ++j) {
                        rolls.append(QString::number(rollDie.values(j)));
                    }
                    if (rolls.isEmpty() && rollDie.value() > 0) {
                        rolls.append(QString::number(rollDie.value()));
                    }
                    const QString sides = rollDie.sides() > 0 ? QString::number(rollDie.sides()) : QStringLiteral("?");
                    addDetail(details, tr("rolled a d%1: %2.").arg(sides, rolls.join(QStringLiteral(", "))), tr("DICE"),
                              ReplayEventKind::Action, eventPlayerId, forcedByJudge);
                    break;
                }
                case GameEvent::FLIP_CARD: {
                    const Event_FlipCard &flipCard = event.GetExtension(Event_FlipCard::ext);
                    const QString zone = QString::fromStdString(flipCard.zone_name());
                    if (flipCard.has_card_name()) {
                        rememberCard(eventPlayerId, zone, flipCard.card_id(),
                                     QString::fromStdString(flipCard.card_name()));
                    }
                    QString name = flipCard.has_card_name() ? QString::fromStdString(flipCard.card_name())
                                                            : cardName(eventPlayerId, zone, flipCard.card_id());
                    if (name.isEmpty()) {
                        name = tr("a card");
                    }
                    addDetail(details,
                              flipCard.face_down() ? tr("flipped %1 face down.").arg(name)
                                                   : tr("flipped %1 face up.").arg(name),
                              tr("FLIP"), ReplayEventKind::Action, eventPlayerId, forcedByJudge);
                    break;
                }
                case GameEvent::DESTROY_CARD: {
                    const Event_DestroyCard &destroyCard = event.GetExtension(Event_DestroyCard::ext);
                    const QString zone = QString::fromStdString(destroyCard.zone_name());
                    const QString name = cardName(eventPlayerId, zone, destroyCard.card_id());
                    forgetCard(eventPlayerId, zone, destroyCard.card_id());
                    addDetail(details, tr("destroyed %1.").arg(name.isEmpty() ? tr("a card") : name), tr("DESTROY"),
                              ReplayEventKind::Action, eventPlayerId, forcedByJudge);
                    break;
                }
                case GameEvent::ATTACH_CARD: {
                    const Event_AttachCard &attachCard = event.GetExtension(Event_AttachCard::ext);
                    const QString startZone = QString::fromStdString(attachCard.start_zone());
                    const QString source = cardName(eventPlayerId, startZone, attachCard.card_id());
                    const QString sourceText = source.isEmpty() ? tr("a card") : source;
                    // An unattach event carries no target at all, so the target fields default to player 0 / no zone
                    // / card 0; only a present target_player_id means the card was attached to something.
                    if (!attachCard.has_target_player_id()) {
                        addDetail(details, tr("unattached %1.").arg(sourceText), tr("ATTACH"), ReplayEventKind::Action,
                                  eventPlayerId, forcedByJudge);
                        break;
                    }
                    const QString targetZone = QString::fromStdString(attachCard.target_zone());
                    const int targetPlayerId =
                        attachCard.target_player_id() >= 0 ? attachCard.target_player_id() : eventPlayerId;
                    const QString target = cardName(targetPlayerId, targetZone, attachCard.target_card_id());
                    addDetail(details,
                              tr("attached %1 to %2.").arg(sourceText, target.isEmpty() ? tr("a card") : target),
                              tr("ATTACH"), ReplayEventKind::Action, eventPlayerId, forcedByJudge);
                    break;
                }
                case GameEvent::CREATE_TOKEN: {
                    const Event_CreateToken &createToken = event.GetExtension(Event_CreateToken::ext);
                    const QString name = QString::fromStdString(createToken.card_name());
                    rememberCard(eventPlayerId, QString::fromStdString(createToken.zone_name()), createToken.card_id(),
                                 name);
                    const QString pt = QString::fromStdString(createToken.pt());
                    addDetail(
                        details,
                        tr("created a token: %1%2.")
                            .arg(name.isEmpty() ? tr("token") : name, pt.isEmpty() ? QString() : tr(" (%1)").arg(pt)),
                        tr("TOKEN"), ReplayEventKind::Action, eventPlayerId, forcedByJudge);
                    break;
                }
                case GameEvent::SET_CARD_ATTR: {
                    const Event_SetCardAttr &setCardAttr = event.GetExtension(Event_SetCardAttr::ext);
                    addDetail(details, describeSetCardAttribute(setCardAttr, eventPlayerId),
                              badgeForCardAttribute(setCardAttr.attribute()), ReplayEventKind::Action, eventPlayerId,
                              forcedByJudge);
                    break;
                }
                case GameEvent::SET_CARD_COUNTER: {
                    const Event_SetCardCounter &setCardCounter = event.GetExtension(Event_SetCardCounter::ext);
                    const QString name = cardName(eventPlayerId, QString::fromStdString(setCardCounter.zone_name()),
                                                  setCardCounter.card_id());
                    addDetail(
                        details,
                        tr("set a counter on %1 to %2.")
                            .arg(name.isEmpty() ? tr("a card") : name, QString::number(setCardCounter.counter_value())),
                        tr("COUNTER"), ReplayEventKind::Action, eventPlayerId, forcedByJudge);
                    break;
                }
                case GameEvent::CREATE_COUNTER: {
                    const Event_CreateCounter &createCounter = event.GetExtension(Event_CreateCounter::ext);
                    const ServerInfo_Counter &counter = createCounter.counter_info();
                    rememberCounter(eventPlayerId, counter);
                    const QString name = QString::fromStdString(counter.name());
                    addDetail(details,
                              tr("created the %1 counter (%2).")
                                  .arg(name.isEmpty() ? tr("new") : name, QString::number(counter.count())),
                              tr("COUNTER"), ReplayEventKind::Action, eventPlayerId, forcedByJudge);
                    break;
                }
                case GameEvent::SET_COUNTER: {
                    const Event_SetCounter &setCounter = event.GetExtension(Event_SetCounter::ext);
                    const QString name = counterName(eventPlayerId, setCounter.counter_id());
                    addDetail(details,
                              tr("set %1 to %2.")
                                  .arg(name.isEmpty() ? tr("a counter") : tr("the %1 counter").arg(name),
                                       QString::number(setCounter.value())),
                              tr("COUNTER"), ReplayEventKind::Action, eventPlayerId, forcedByJudge);
                    break;
                }
                case GameEvent::DEL_COUNTER: {
                    const Event_DelCounter &delCounter = event.GetExtension(Event_DelCounter::ext);
                    const QString name = counterName(eventPlayerId, delCounter.counter_id());
                    counterNames.remove(counterKey(eventPlayerId, delCounter.counter_id()));
                    addDetail(details,
                              tr("removed %1.").arg(name.isEmpty() ? tr("a counter") : tr("the %1 counter").arg(name)),
                              tr("COUNTER"), ReplayEventKind::Action, eventPlayerId, forcedByJudge);
                    break;
                }
                case GameEvent::DUMP_ZONE: {
                    const Event_DumpZone &dumpZone = event.GetExtension(Event_DumpZone::ext);
                    const QString zoneOwnerPhrase = zonePhrase(
                        dumpZone.zone_owner_id(), QString::fromStdString(dumpZone.zone_name()), eventPlayerId);
                    if (dumpZone.number_cards() < 0) {
                        addDetail(details, tr("looked at %1.").arg(zoneOwnerPhrase), tr("LOOK"),
                                  ReplayEventKind::Action, eventPlayerId, forcedByJudge);
                    } else {
                        addDetail(details,
                                  tr("looked at %1 from %2.")
                                      .arg(formatCardList(QStringList(), dumpZone.number_cards()), zoneOwnerPhrase),
                                  tr("LOOK"), ReplayEventKind::Action, eventPlayerId, forcedByJudge);
                    }
                    break;
                }
                case GameEvent::CHANGE_ZONE_PROPERTIES: {
                    const Event_ChangeZoneProperties &changeZone = event.GetExtension(Event_ChangeZoneProperties::ext);
                    const QString zone = QString::fromStdString(changeZone.zone_name());
                    QStringList changes;
                    if (changeZone.always_reveal_top_card()) {
                        changes.append(tr("top card revealed to all"));
                    }
                    if (changeZone.always_look_at_top_card()) {
                        changes.append(tr("top card visible to its owner"));
                    }
                    addDetail(
                        details,
                        changes.isEmpty()
                            ? tr("changed the properties of %1.").arg(zonePhrase(eventPlayerId, zone, eventPlayerId))
                            : tr("changed %1: %2.")
                                  .arg(zonePhrase(eventPlayerId, zone, eventPlayerId),
                                       changes.join(QStringLiteral(", "))),
                        tr("ZONE"), ReplayEventKind::Action, eventPlayerId, forcedByJudge);
                    break;
                }
                case GameEvent::GAME_LOG_NOTICE: {
                    const Event_GameLogNotice &notice = event.GetExtension(Event_GameLogNotice::ext);
                    switch (notice.notice_type()) {
                        case Event_GameLogNotice::UNDO_DRAW_FAILED:
                            addDetail(details, tr("failed to undo a draw."), tr("NOTICE"), ReplayEventKind::System,
                                      eventPlayerId, forcedByJudge);
                            break;
                        default:
                            addDetail(details, tr("A game notice was logged."), tr("NOTICE"), ReplayEventKind::System,
                                      -1, forcedByJudge);
                            break;
                    }
                    break;
                }
                case GameEvent::CREATE_ARROW:
                case GameEvent::DELETE_ARROW:
                    // Arrows are transient visual annotations and would make the history very noisy.
                    break;
                default:
                    addDetail(details, tr("Unrecognized game event."), tr("EVENT"), ReplayEventKind::Action,
                              eventPlayerId, forcedByJudge);
                    break;
            }
        }

        return details;
    }
};
} // namespace

QList<ReplayEventEntry> buildReplayEventEntries(const GameReplay *replay, const QList<int> &timeline)
{
    if (!replay) {
        return {};
    }

    return ReplayEventIndexBuilder(replay, timeline).build();
}
