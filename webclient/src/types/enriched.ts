import type {
  Event_RoomSay,
  ServerInfo_Arrow,
  ServerInfo_Card,
  ServerInfo_ChatMessage,
  ServerInfo_Counter,
  ServerInfo_Game,
  ServerInfo_PlayerProperties,
  ServerInfo_Room,
  ServerInfo_User,
} from '@app/generated';

// @critical `info` is the wire snapshot; repeated collections on it go stale. Read normalized siblings.
// See .github/instructions/webclient.instructions.md#data-structure-invariants.

export interface GametypeMap { [index: number]: string }

export interface Room {
  info: ServerInfo_Room;
  gametypeMap: GametypeMap;
  order: number;
  games: { [gameId: number]: Game };
  users: { [userName: string]: ServerInfo_User };
}

export interface Game {
  info: ServerInfo_Game;
  gameType: string;
}

export type Message = Event_RoomSay & {
  timeReceived: number;
};

// @critical `info` = wire snapshot at join time; top-level twins hold live values updated by game events.
// See .github/instructions/webclient.instructions.md#data-structure-invariants.
export interface GameEntry {
  info: ServerInfo_Game;

  hostId: number;
  localPlayerId: number;
  spectator: boolean;
  judge: boolean;
  resuming: boolean;

  started: boolean;
  activePlayerId: number;
  activePhase: number;
  secondsElapsed: number;
  reversed: boolean;

  players: { [playerId: number]: PlayerEntry };
  messages: GameMessage[];
}

export interface PlayerEntry {
  properties: ServerInfo_PlayerProperties;
  deckList: string;
  zones: { [zoneName: string]: ZoneEntry };
  counters: { [counterId: number]: ServerInfo_Counter };
  arrows: { [arrowId: number]: ServerInfo_Arrow };
}

// Canonical wire values for `ZoneEntry.name`. Server-defined and stable.
export const ZoneName = {
  TABLE: 'table',
  GRAVE: 'grave',
  EXILE: 'rfg',
  HAND: 'hand',
  DECK: 'deck',
  SIDEBOARD: 'sb',
  STACK: 'stack',
} as const;

export type ZoneNameValue = typeof ZoneName[keyof typeof ZoneName];

export interface ZoneEntry {
  name: string;
  /** ZoneType enum value (0=Private, 1=Public, 2=Hidden). */
  type: number;
  withCoords: boolean;
  /** Authoritative count; for hidden zones this may exceed `order.length`. */
  cardCount: number;
  order: number[];
  byId: { [cardId: number]: ServerInfo_Card };
  alwaysRevealTopCard: boolean;
  alwaysLookAtTopCard: boolean;
}

export interface GameMessage {
  playerId: number;
  message: string;
  timeReceived: number;
  /** Defaults to 'chat'. Event messages are rendered without a leading speaker label. */
  kind?: 'chat' | 'event';
}

export interface LogGroups {
  room: ServerInfo_ChatMessage[];
  game: ServerInfo_ChatMessage[];
  chat: ServerInfo_ChatMessage[];
}
