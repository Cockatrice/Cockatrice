import { create } from '@bufbuild/protobuf';
import { Data, Enriched } from '@app/types';

export const MAX_GAME_MESSAGES = 1000;

// Mirrors Event_Leave.LeaveReason values (1=OTHER, 2=USER_KICKED,
// 3=USER_LEFT, 4=USER_DISCONNECTED); kept in sync with desktop
// `GameEventHandler::getLeaveReason` in game_event_handler.cpp.
const LEAVE_REASON_MESSAGES: Record<number, string> = {
  1: 'reason unknown',
  2: 'kicked by game host or moderator',
  3: 'player left the game',
  4: 'player disconnected from server',
};

export function formatLeaveMessage(playerName: string, reason: number): string {
  const reasonText = LEAVE_REASON_MESSAGES[reason] ?? LEAVE_REASON_MESSAGES[1];
  return `${playerName} has left the game (${reasonText}).`;
}

/** Timestamp source for event-log entries. Isolated so tests can mock it. */
export function eventTimestamp(): number {
  return Date.now();
}

/** Push a formatted event-log message onto the game's message list, truncating to MAX_GAME_MESSAGES. */
export function pushEventMessage(
  game: Enriched.GameEntry,
  playerId: number,
  message: string | null | undefined,
): void {
  if (!message) {
    return;
  }
  if (game.messages.length >= MAX_GAME_MESSAGES) {
    game.messages = game.messages.slice(game.messages.length - MAX_GAME_MESSAGES + 1);
  }
  game.messages.push({
    playerId,
    message,
    timeReceived: eventTimestamp(),
    kind: 'event',
  });
}

/** Converts the proto ServerInfo_Player[] array into the keyed PlayerEntry map. */
export function normalizePlayers(playerList: Data.ServerInfo_Player[]): { [playerId: number]: Enriched.PlayerEntry } {
  const players: { [playerId: number]: Enriched.PlayerEntry } = {};
  for (const player of playerList) {
    const playerId = player.properties.playerId;

    const zones: { [zoneName: string]: Enriched.ZoneEntry } = {};
    for (const zone of player.zoneList) {
      const order: number[] = [];
      const byId: { [id: number]: Data.ServerInfo_Card } = {};
      for (const card of zone.cardList) {
        order.push(card.id);
        byId[card.id] = card;
      }
      zones[zone.name] = {
        name: zone.name as Enriched.ZoneNameValue,
        type: zone.type,
        withCoords: zone.withCoords,
        cardCount: zone.cardCount,
        order,
        byId,
        alwaysRevealTopCard: zone.alwaysRevealTopCard,
        alwaysLookAtTopCard: zone.alwaysLookAtTopCard,
      };
    }

    const counters: { [counterId: number]: Data.ServerInfo_Counter } = {};
    for (const counter of player.counterList) {
      counters[counter.id] = counter;
    }

    const arrows: { [arrowId: number]: Data.ServerInfo_Arrow } = {};
    for (const arrow of player.arrowList) {
      arrows[arrow.id] = arrow;
    }

    players[playerId] = {
      properties: player.properties,
      deckList: player.deckList,
      zones,
      counters,
      arrows,
    };
  }
  return players;
}

export function buildEmptyCard(
  id: number,
  name: string,
  x: number,
  y: number,
  faceDown: boolean,
  providerId: string
): Data.ServerInfo_Card {
  return create(Data.ServerInfo_CardSchema, {
    id, name, x, y, faceDown,
    tapped: false, attacking: false, color: '', pt: '', annotation: '',
    destroyOnZoneChange: false, doesntUntap: false, counterList: [],
    attachPlayerId: -1, attachZone: '', attachCardId: -1, providerId,
  });
}
