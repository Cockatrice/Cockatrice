import type { MessageInitShape } from '@bufbuild/protobuf';
import { Data } from '@app/types';
import { create } from '@bufbuild/protobuf';
import { GameEntry, GamesState, PlayerEntry, ZoneEntry } from '../game.interfaces';

export function makeCard(overrides: MessageInitShape<typeof Data.ServerInfo_CardSchema> = {}): Data.ServerInfo_Card {
  return create(Data.ServerInfo_CardSchema, {
    id: 1,
    name: 'Test Card',
    x: 0,
    y: 0,
    faceDown: false,
    tapped: false,
    attacking: false,
    color: '',
    pt: '',
    annotation: '',
    destroyOnZoneChange: false,
    doesntUntap: false,
    counterList: [],
    attachPlayerId: -1,
    attachZone: '',
    attachCardId: -1,
    providerId: '',
    ...overrides,
  });
}

export function makeCounter(overrides: MessageInitShape<typeof Data.ServerInfo_CounterSchema> = {}): Data.ServerInfo_Counter {
  return create(Data.ServerInfo_CounterSchema, {
    id: 1,
    name: 'Life',
    counterColor: create(Data.colorSchema, { r: 0, g: 0, b: 0, a: 255 }),
    radius: 1,
    count: 20,
    ...overrides,
  });
}

export function makeArrow(overrides: MessageInitShape<typeof Data.ServerInfo_ArrowSchema> = {}): Data.ServerInfo_Arrow {
  return create(Data.ServerInfo_ArrowSchema, {
    id: 1,
    startPlayerId: 1,
    startZone: 'table',
    startCardId: 1,
    targetPlayerId: 1,
    targetZone: 'table',
    targetCardId: 2,
    arrowColor: create(Data.colorSchema, { r: 255, g: 0, b: 0, a: 255 }),
    ...overrides,
  });
}

export function makeZoneEntry(overrides: Partial<ZoneEntry> = {}): ZoneEntry {
  return {
    name: 'hand',
    type: 1,
    withCoords: false,
    cardCount: 0,
    cards: [],
    alwaysRevealTopCard: false,
    alwaysLookAtTopCard: false,
    ...overrides,
  };
}

export function makePlayerProperties(
  overrides: MessageInitShape<typeof Data.ServerInfo_PlayerPropertiesSchema> = {},
): Data.ServerInfo_PlayerProperties {
  return create(Data.ServerInfo_PlayerPropertiesSchema, {
    playerId: 1,
    spectator: false,
    conceded: false,
    readyStart: false,
    deckHash: '',
    pingSeconds: 0,
    sideboardLocked: false,
    judge: false,
    ...overrides,
  });
}

export function makePlayerEntry(overrides: Partial<PlayerEntry> = {}): PlayerEntry {
  return {
    properties: makePlayerProperties(),
    deckList: '',
    zones: {
      hand: makeZoneEntry({ name: 'hand' }),
      deck: makeZoneEntry({ name: 'deck' }),
    },
    counters: {},
    arrows: {},
    ...overrides,
  };
}

export function makeGameEntry(overrides: Partial<GameEntry> = {}): GameEntry {
  return {
    gameId: 1,
    roomId: 1,
    description: 'Test Game',
    hostId: 1,
    localPlayerId: 1,
    spectator: false,
    judge: false,
    resuming: false,
    started: false,
    activePlayerId: 0,
    activePhase: 0,
    secondsElapsed: 0,
    reversed: false,
    players: {
      1: makePlayerEntry(),
    },
    messages: [],
    ...overrides,
  };
}

export function makeState(overrides: Partial<GamesState> = {}): GamesState {
  return {
    games: {
      1: makeGameEntry(),
    },
    ...overrides,
  };
}
