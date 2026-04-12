import { ArrowInfo, CardInfo, CounterInfo, PlayerProperties } from 'types';

export interface GamesState {
  games: { [gameId: number]: GameEntry };
}

/**
 * Full runtime state for a single active game (played or spectated).
 * Keyed by gameId in GamesState so multiple concurrent games are supported.
 */
export interface GameEntry {
  gameId: number;
  roomId: number;
  description: string;
  hostId: number;
  /** The playerId assigned to the local user in this game. */
  localPlayerId: number;
  spectator: boolean;
  judge: boolean;
  started: boolean;
  activePlayerId: number;
  activePhase: number;
  secondsElapsed: number;
  reversed: boolean;
  players: { [playerId: number]: PlayerEntry };
  messages: GameMessage[];
}

/** Normalized from ServerInfo_Player — keyed collections for O(1) lookup. */
export interface PlayerEntry {
  properties: PlayerProperties;
  deckList: string;
  /** Zones keyed by zone name (e.g. "hand", "deck", "table"). */
  zones: { [zoneName: string]: ZoneEntry };
  /** Player-level counters (e.g. life) keyed by counter id. */
  counters: { [counterId: number]: CounterInfo };
  /** Arrows keyed by arrow id. */
  arrows: { [arrowId: number]: ArrowInfo };
}

/** Normalized from ServerInfo_Zone — card list is an ordered array matching proto. */
export interface ZoneEntry {
  name: string;
  /** ZoneType enum value (0=Private, 1=Public, 2=Hidden). */
  type: number;
  withCoords: boolean;
  /** Authoritative card count (used for hidden zones where cardList may be empty). */
  cardCount: number;
  /** Ordered card list; may be empty for hidden zones with no dump active. */
  cards: CardInfo[];
  alwaysRevealTopCard: boolean;
  alwaysLookAtTopCard: boolean;
}

export interface GameMessage {
  playerId: number;
  message: string;
  timeReceived: number;
}
