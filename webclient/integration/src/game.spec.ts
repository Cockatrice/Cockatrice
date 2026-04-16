// Game scenarios — game join, state initialization, card operations,
// player counters, game chat, game close, and outbound game commands.

import { create } from '@bufbuild/protobuf';
import { describe, expect, it } from 'vitest';

import { Data } from '@app/types';
import { store } from '@app/store';
import { GameCommands, RoomCommands } from '@app/websocket';

import { connectAndHandshake, connectAndLogin } from './helpers/setup';
import {
  buildResponse,
  buildResponseMessage,
  buildSessionEventMessage,
  buildRoomEventMessage,
  buildGameEventMessage,
  deliverMessage,
} from './helpers/protobuf-builders';
import { findLastGameCommand, findLastRoomCommand, findLastSessionCommand } from './helpers/command-capture';

function joinGame(gameId: number): void {
  deliverMessage(buildSessionEventMessage(
    Data.Event_GameJoined_ext,
    create(Data.Event_GameJoinedSchema, {
      gameInfo: create(Data.ServerInfo_GameSchema, {
        gameId,
        description: 'Test Game',
        maxPlayers: 2,
        playerCount: 1,
      }),
      playerId: 1,
      hostId: 1,
      spectator: false,
      judge: false,
      resuming: false,
    })
  ));
}

function setupGameState(gameId: number): void {
  const deckCard = create(Data.ServerInfo_CardSchema, { id: 100, name: 'Forest' });
  const handCard = create(Data.ServerInfo_CardSchema, { id: 101, name: 'Lightning Bolt' });

  const deckZone = create(Data.ServerInfo_ZoneSchema, {
    name: 'deck',
    type: Data.ServerInfo_Zone_ZoneType.HiddenZone,
    cardList: [deckCard],
  });
  const handZone = create(Data.ServerInfo_ZoneSchema, {
    name: 'hand',
    type: Data.ServerInfo_Zone_ZoneType.HiddenZone,
    cardList: [handCard],
  });
  const tableZone = create(Data.ServerInfo_ZoneSchema, {
    name: 'table',
    type: Data.ServerInfo_Zone_ZoneType.PublicZone,
    withCoords: true,
    cardList: [],
  });

  const player = create(Data.ServerInfo_PlayerSchema, {
    properties: create(Data.ServerInfo_PlayerPropertiesSchema, {
      playerId: 1,
      userInfo: create(Data.ServerInfo_UserSchema, { name: 'alice' }),
    }),
    zoneList: [deckZone, handZone, tableZone],
    counterList: [],
    arrowList: [],
  });

  deliverMessage(buildGameEventMessage({
    gameId,
    playerId: -1,
    ext: Data.Event_GameStateChanged_ext,
    value: create(Data.Event_GameStateChangedSchema, {
      playerList: [player],
      gameStarted: true,
      activePlayerId: 1,
      activePhase: 0,
    }),
  }));
}

describe('game', () => {
  it('initializes game state from Event_GameJoined + Event_GameStateChanged', () => {
    connectAndLogin();
    joinGame(42);

    const game = store.getState().games.games[42];
    expect(game).toBeDefined();
    expect(game.info.description).toBe('Test Game');
    expect(game.localPlayerId).toBe(1);

    setupGameState(42);

    const updated = store.getState().games.games[42];
    expect(updated.started).toBe(true);
    expect(updated.activePlayerId).toBe(1);
    expect(updated.players[1]).toBeDefined();
    expect(updated.players[1].zones.hand).toBeDefined();
    expect(updated.players[1].zones.deck).toBeDefined();
    expect(updated.players[1].zones.hand.order).toContain(101);
    expect(updated.players[1].zones.deck.order).toContain(100);
  });

  it('draws cards from deck to hand on Event_DrawCards', () => {
    connectAndLogin();
    joinGame(42);
    setupGameState(42);

    const drawnCard = create(Data.ServerInfo_CardSchema, { id: 200, name: 'Mountain' });
    deliverMessage(buildGameEventMessage({
      gameId: 42,
      playerId: 1,
      ext: Data.Event_DrawCards_ext,
      value: create(Data.Event_DrawCardsSchema, {
        number: 1,
        cards: [drawnCard],
      }),
    }));

    const player = store.getState().games.games[42].players[1];
    expect(player.zones.hand.order).toContain(200);
    expect(player.zones.hand.byId[200]?.name).toBe('Mountain');
  });

  it('appends chat messages on Event_GameSay', () => {
    connectAndLogin();
    joinGame(42);

    deliverMessage(buildGameEventMessage({
      gameId: 42,
      playerId: 1,
      ext: Data.Event_GameSay_ext,
      value: create(Data.Event_GameSaySchema, { message: 'good game' }),
    }));

    const messages = store.getState().games.games[42].messages;
    expect(messages).toHaveLength(1);
    expect(messages[0].message).toBe('good game');
    expect(messages[0].playerId).toBe(1);
  });

  it('removes game from store on Event_GameClosed', () => {
    connectAndLogin();
    joinGame(42);

    expect(store.getState().games.games[42]).toBeDefined();

    deliverMessage(buildGameEventMessage({
      gameId: 42,
      playerId: -1,
      ext: Data.Event_GameClosed_ext,
      value: create(Data.Event_GameClosedSchema),
    }));

    expect(store.getState().games.games[42]).toBeUndefined();
  });

  it('sends outbound Command_GameSay with correct gameId and message', () => {
    connectAndLogin();
    joinGame(42);

    GameCommands.gameSay(42, { message: 'hello opponent' });

    const { value, cmdId } = findLastGameCommand(Data.Command_GameSay_ext);
    expect(value.message).toBe('hello opponent');
    expect(cmdId).toBeGreaterThan(0);
  });

  it('moves a card from hand to table on Event_MoveCard', () => {
    connectAndLogin();
    joinGame(42);
    setupGameState(42);

    deliverMessage(buildGameEventMessage({
      gameId: 42,
      playerId: 1,
      ext: Data.Event_MoveCard_ext,
      value: create(Data.Event_MoveCardSchema, {
        cardId: 101,
        cardName: 'Lightning Bolt',
        startPlayerId: 1,
        startZone: 'hand',
        targetPlayerId: 1,
        targetZone: 'table',
        x: 100,
        y: 200,
        faceDown: false,
        newCardId: 101,
      }),
    }));

    const player = store.getState().games.games[42].players[1];
    expect(player.zones.hand.order).not.toContain(101);
    expect(player.zones.table.order).toContain(101);
    expect(player.zones.table.byId[101]?.name).toBe('Lightning Bolt');
    expect(player.zones.table.byId[101]?.x).toBe(100);
  });

  it('creates and updates player counters', () => {
    connectAndLogin();
    joinGame(42);
    setupGameState(42);

    const counterInfo = create(Data.ServerInfo_CounterSchema, {
      id: 1,
      name: 'Life',
      count: 20,
      radius: 1,
    });

    deliverMessage(buildGameEventMessage({
      gameId: 42,
      playerId: 1,
      ext: Data.Event_CreateCounter_ext,
      value: create(Data.Event_CreateCounterSchema, { counterInfo }),
    }));

    const player = store.getState().games.games[42].players[1];
    expect(player.counters[1]).toBeDefined();
    expect(player.counters[1].name).toBe('Life');
    expect(player.counters[1].count).toBe(20);

    deliverMessage(buildGameEventMessage({
      gameId: 42,
      playerId: 1,
      ext: Data.Event_SetCounter_ext,
      value: create(Data.Event_SetCounterSchema, { counterId: 1, value: 17 }),
    }));

    expect(store.getState().games.games[42].players[1].counters[1].count).toBe(17);
  });

  it('full lifecycle: create → join → deck select → draw → chat → discard → concede → leave', () => {
    connectAndHandshake();

    // ── Setup: join a room so we can create a game in it ──────────────────
    deliverMessage(buildSessionEventMessage(
      Data.Event_ListRooms_ext,
      create(Data.Event_ListRoomsSchema, {
        roomList: [create(Data.ServerInfo_RoomSchema, { roomId: 1, autoJoin: true, gameList: [], userList: [], gametypeList: [] })],
      })
    ));
    const roomJoin = findLastSessionCommand(Data.Command_JoinRoom_ext);
    deliverMessage(buildResponseMessage(buildResponse({
      cmdId: roomJoin.cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
      ext: Data.Response_JoinRoom_ext,
      value: create(Data.Response_JoinRoomSchema, {
        roomInfo: create(Data.ServerInfo_RoomSchema, { roomId: 1, gameList: [], userList: [], gametypeList: [] }),
      }),
    })));

    // ── 1. Create game ───────────────────────────────────────────────────
    RoomCommands.createGame(1, { description: 'Ranked Match', maxPlayers: 2 });
    const createCmd = findLastRoomCommand(Data.Command_CreateGame_ext);
    deliverMessage(buildResponseMessage(buildResponse({
      cmdId: createCmd.cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
    })));

    // ── 2. Join game ─────────────────────────────────────────────────────
    RoomCommands.joinGame(1, { gameId: 99 });
    const joinCmd = findLastRoomCommand(Data.Command_JoinGame_ext);
    deliverMessage(buildResponseMessage(buildResponse({
      cmdId: joinCmd.cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
    })));
    expect(store.getState().rooms.joinedGameIds[1]?.[99]).toBe(true);

    // Server sends Event_GameJoined (session event)
    deliverMessage(buildSessionEventMessage(
      Data.Event_GameJoined_ext,
      create(Data.Event_GameJoinedSchema, {
        gameInfo: create(Data.ServerInfo_GameSchema, { gameId: 99, description: 'Ranked Match', maxPlayers: 2 }),
        playerId: 1,
        hostId: 1,
        spectator: false,
        judge: false,
        resuming: false,
      })
    ));
    expect(store.getState().games.games[99]).toBeDefined();

    // ── 3. Select deck ───────────────────────────────────────────────────
    GameCommands.deckSelect(99, { deck: '4 Lightning Bolt\n20 Mountain\n4 Goblin Guide' });
    const deckCmd = findLastGameCommand(Data.Command_DeckSelect_ext);
    expect(deckCmd.value.deck).toContain('Lightning Bolt');

    // Server responds with full game state (deck in zones)
    const deckCards = [
      create(Data.ServerInfo_CardSchema, { id: 1, name: 'Lightning Bolt' }),
      create(Data.ServerInfo_CardSchema, { id: 2, name: 'Mountain' }),
      create(Data.ServerInfo_CardSchema, { id: 3, name: 'Goblin Guide' }),
    ];
    deliverMessage(buildGameEventMessage({
      gameId: 99,
      playerId: -1,
      ext: Data.Event_GameStateChanged_ext,
      value: create(Data.Event_GameStateChangedSchema, {
        playerList: [create(Data.ServerInfo_PlayerSchema, {
          properties: create(Data.ServerInfo_PlayerPropertiesSchema, {
            playerId: 1,
            userInfo: create(Data.ServerInfo_UserSchema, { name: 'alice' }),
          }),
          zoneList: [
            create(Data.ServerInfo_ZoneSchema, { name: 'deck', type: Data.ServerInfo_Zone_ZoneType.HiddenZone, cardList: deckCards, cardCount: 3 }),
            create(Data.ServerInfo_ZoneSchema, { name: 'hand', type: Data.ServerInfo_Zone_ZoneType.HiddenZone, cardList: [], cardCount: 0 }),
            create(Data.ServerInfo_ZoneSchema, { name: 'table', type: Data.ServerInfo_Zone_ZoneType.PublicZone, withCoords: true, cardList: [], cardCount: 0 }),
            create(Data.ServerInfo_ZoneSchema, { name: 'grave', type: Data.ServerInfo_Zone_ZoneType.PublicZone, cardList: [], cardCount: 0 }),
          ],
          counterList: [],
          arrowList: [],
        })],
        gameStarted: true,
        activePlayerId: 1,
        activePhase: 0,
      }),
    }));

    const gameAfterDeck = store.getState().games.games[99];
    expect(gameAfterDeck.players[1].zones.deck.order).toHaveLength(3);
    expect(gameAfterDeck.players[1].zones.hand.order).toHaveLength(0);

    // ── 4. Draw cards ────────────────────────────────────────────────────
    deliverMessage(buildGameEventMessage({
      gameId: 99,
      playerId: 1,
      ext: Data.Event_DrawCards_ext,
      value: create(Data.Event_DrawCardsSchema, {
        number: 2,
        cards: [
          create(Data.ServerInfo_CardSchema, { id: 1, name: 'Lightning Bolt' }),
          create(Data.ServerInfo_CardSchema, { id: 2, name: 'Mountain' }),
        ],
      }),
    }));

    const afterDraw = store.getState().games.games[99].players[1];
    expect(afterDraw.zones.hand.order).toHaveLength(2);
    expect(afterDraw.zones.hand.order).toContain(1);
    expect(afterDraw.zones.hand.order).toContain(2);
    expect(afterDraw.zones.deck.cardCount).toBe(1);

    // ── 5. Send game message ─────────────────────────────────────────────
    GameCommands.gameSay(99, { message: 'good luck!' });
    const sayCmd = findLastGameCommand(Data.Command_GameSay_ext);
    expect(sayCmd.value.message).toBe('good luck!');

    // Server echoes the message back
    deliverMessage(buildGameEventMessage({
      gameId: 99,
      playerId: 1,
      ext: Data.Event_GameSay_ext,
      value: create(Data.Event_GameSaySchema, { message: 'good luck!' }),
    }));
    expect(store.getState().games.games[99].messages).toHaveLength(1);

    // ── 6. Discard (move card from hand to graveyard) ────────────────────
    deliverMessage(buildGameEventMessage({
      gameId: 99,
      playerId: 1,
      ext: Data.Event_MoveCard_ext,
      value: create(Data.Event_MoveCardSchema, {
        cardId: 1,
        cardName: 'Lightning Bolt',
        startPlayerId: 1,
        startZone: 'hand',
        targetPlayerId: 1,
        targetZone: 'grave',
        faceDown: false,
        newCardId: 1,
      }),
    }));

    const afterDiscard = store.getState().games.games[99].players[1];
    expect(afterDiscard.zones.hand.order).not.toContain(1);
    expect(afterDiscard.zones.grave.order).toContain(1);
    expect(afterDiscard.zones.grave.byId[1]?.name).toBe('Lightning Bolt');

    // ── 7. Concede ───────────────────────────────────────────────────────
    GameCommands.concede(99);
    expect(() => findLastGameCommand(Data.Command_Concede_ext)).not.toThrow();

    // Server confirms concession
    deliverMessage(buildGameEventMessage({
      gameId: 99,
      playerId: 1,
      ext: Data.Event_PlayerPropertiesChanged_ext,
      value: create(Data.Event_PlayerPropertiesChangedSchema, {
        playerProperties: create(Data.ServerInfo_PlayerPropertiesSchema, {
          playerId: 1,
          conceded: true,
          userInfo: create(Data.ServerInfo_UserSchema, { name: 'alice' }),
        }),
      }),
    }));
    expect(store.getState().games.games[99].players[1].properties.conceded).toBe(true);

    // ── 8. Leave game ────────────────────────────────────────────────────
    GameCommands.leaveGame(99);
    expect(() => findLastGameCommand(Data.Command_LeaveGame_ext)).not.toThrow();

    // Server confirms player left
    deliverMessage(buildGameEventMessage({
      gameId: 99,
      playerId: 1,
      ext: Data.Event_Leave_ext,
      value: create(Data.Event_LeaveSchema, { reason: Data.Event_Leave_LeaveReason.USER_LEFT }),
    }));

    expect(store.getState().games.games[99].players[1]).toBeUndefined();
  });
});