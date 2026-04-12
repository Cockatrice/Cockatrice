import { CardAttribute, PlayerInfo } from 'types';
import { gamesReducer } from './game.reducer';
import { Types } from './game.types';
import {
  makeArrow,
  makeCard,
  makeCounter,
  makeGameEntry,
  makePlayerEntry,
  makePlayerProperties,
  makeState,
  makeZoneEntry,
} from './__mocks__/fixtures';

// ── 2A: Initialisation & lifecycle ───────────────────────────────────────────

describe('2A: Initialisation & lifecycle', () => {
  it('returns initialState ({ games: {} }) when called with undefined state', () => {
    const result = gamesReducer(undefined, { type: '@@INIT' });
    expect(result).toEqual({ games: {} });
  });

  it('CLEAR_STORE → resets to initialState', () => {
    const state = makeState();
    const result = gamesReducer(state, { type: Types.CLEAR_STORE });
    expect(result).toEqual({ games: {} });
  });

  it('GAME_JOINED → inserts gameEntry keyed by gameId', () => {
    const entry = makeGameEntry({ gameId: 42 });
    const result = gamesReducer({ games: {} }, { type: Types.GAME_JOINED, gameId: 42, gameEntry: entry });
    expect(result.games[42]).toBe(entry);
  });

  it('GAME_LEFT → removes game by gameId', () => {
    const state = makeState();
    const result = gamesReducer(state, { type: Types.GAME_LEFT, gameId: 1 });
    expect(result.games[1]).toBeUndefined();
  });

  it('GAME_CLOSED → removes game by gameId', () => {
    const state = makeState();
    const result = gamesReducer(state, { type: Types.GAME_CLOSED, gameId: 1 });
    expect(result.games[1]).toBeUndefined();
  });

  it('KICKED → removes game by gameId', () => {
    const state = makeState();
    const result = gamesReducer(state, { type: Types.KICKED, gameId: 1 });
    expect(result.games[1]).toBeUndefined();
  });

  it('GAME_HOST_CHANGED → updates hostId on existing game', () => {
    const state = makeState();
    const result = gamesReducer(state, { type: Types.GAME_HOST_CHANGED, gameId: 1, hostId: 99 });
    expect(result.games[1].hostId).toBe(99);
    expect(result).not.toBe(state);
  });
});

// ── 2B: Game state & player management ───────────────────────────────────────

describe('2B: Game state & player management', () => {
  it('GAME_STATE_CHANGED with playerList → replaces players via normalizePlayers', () => {
    const state = makeState();
    const card = makeCard({ id: 5 });
    const counter = makeCounter({ id: 2 });
    const arrow = makeArrow({ id: 3 });
    const playerList: PlayerInfo[] = [
      {
        properties: makePlayerProperties({ playerId: 7 }),
        deckList: 'some deck',
        zoneList: [
          {
            name: 'hand',
            type: 1,
            withCoords: false,
            cardCount: 1,
            cardList: [card],
            alwaysRevealTopCard: false,
            alwaysLookAtTopCard: false,
          },
        ],
        counterList: [counter],
        arrowList: [arrow],
      },
    ];

    const result = gamesReducer(state, {
      type: Types.GAME_STATE_CHANGED,
      gameId: 1,
      data: { playerList },
    });

    const player = result.games[1].players[7];
    expect(player).toBeDefined();
    expect(player.zones['hand'].cards[0]).toEqual(card);
    expect(player.counters[2]).toEqual(counter);
    expect(player.arrows[3]).toEqual(arrow);
  });

  it('GAME_STATE_CHANGED with scalar fields → updates started, activePlayerId, activePhase, secondsElapsed', () => {
    const state = makeState();
    const result = gamesReducer(state, {
      type: Types.GAME_STATE_CHANGED,
      gameId: 1,
      data: {
        gameStarted: true,
        activePlayerId: 3,
        activePhase: 2,
        secondsElapsed: 60,
      },
    });

    expect(result.games[1].started).toBe(true);
    expect(result.games[1].activePlayerId).toBe(3);
    expect(result.games[1].activePhase).toBe(2);
    expect(result.games[1].secondsElapsed).toBe(60);
  });

  it('PLAYER_JOINED → adds new empty PlayerEntry keyed by playerId', () => {
    const state = makeState();
    const props = makePlayerProperties({ playerId: 5 });
    const result = gamesReducer(state, { type: Types.PLAYER_JOINED, gameId: 1, playerProperties: props });
    const newPlayer = result.games[1].players[5];
    expect(newPlayer).toBeDefined();
    expect(newPlayer.properties).toBe(props);
    expect(newPlayer.zones).toEqual({});
    expect(newPlayer.counters).toEqual({});
    expect(newPlayer.arrows).toEqual({});
  });

  it('PLAYER_LEFT → removes player from game.players', () => {
    const state = makeState();
    const result = gamesReducer(state, { type: Types.PLAYER_LEFT, gameId: 1, playerId: 1 });
    expect(result.games[1].players[1]).toBeUndefined();
  });

  it('PLAYER_PROPERTIES_CHANGED → replaces properties on existing player', () => {
    const state = makeState();
    const newProps = makePlayerProperties({ playerId: 1, conceded: true });
    const result = gamesReducer(state, {
      type: Types.PLAYER_PROPERTIES_CHANGED,
      gameId: 1,
      playerId: 1,
      properties: newProps,
    });
    expect(result.games[1].players[1].properties).toBe(newProps);
  });
});

// ── 2C: CARD_MOVED ────────────────────────────────────────────────────────────

describe('2C: CARD_MOVED', () => {
  function stateWithCard(cardOverrides: Parameters<typeof makeCard>[0] = {}) {
    const card = makeCard({ id: 10, ...cardOverrides });
    const state = makeState({
      games: {
        1: makeGameEntry({
          players: {
            1: makePlayerEntry({
              zones: {
                hand: makeZoneEntry({ name: 'hand', cards: [card], cardCount: 1 }),
                table: makeZoneEntry({ name: 'table', cardCount: 0 }),
              },
            }),
          },
        }),
      },
    });
    return { state, card };
  }

  it('moves card by cardId ≥ 0 from source to target zone', () => {
    const { state } = stateWithCard();
    const result = gamesReducer(state, {
      type: Types.CARD_MOVED,
      gameId: 1,
      playerId: 1,
      data: {
        cardId: 10,
        cardName: '',
        startPlayerId: 1,
        startZone: 'hand',
        position: -1,
        targetPlayerId: 1,
        targetZone: 'table',
        x: 5,
        y: 7,
        newCardId: -1,
        faceDown: false,
        newCardProviderId: '',
      },
    });

    expect(result.games[1].players[1].zones['hand'].cards).toHaveLength(0);
    expect(result.games[1].players[1].zones['hand'].cardCount).toBe(0);
    const movedCard = result.games[1].players[1].zones['table'].cards[0];
    expect(movedCard.id).toBe(10);
    expect(movedCard.x).toBe(5);
    expect(movedCard.y).toBe(7);
    expect(result.games[1].players[1].zones['table'].cardCount).toBe(1);
  });

  it('moves card by position index when cardId < 0', () => {
    const card = makeCard({ id: 11 });
    const state = makeState({
      games: {
        1: makeGameEntry({
          players: {
            1: makePlayerEntry({
              zones: {
                deck: makeZoneEntry({ name: 'deck', cards: [card], cardCount: 1 }),
                hand: makeZoneEntry({ name: 'hand', cardCount: 0 }),
              },
            }),
          },
        }),
      },
    });

    const result = gamesReducer(state, {
      type: Types.CARD_MOVED,
      gameId: 1,
      playerId: 1,
      data: {
        cardId: -1,
        cardName: '',
        startPlayerId: 1,
        startZone: 'deck',
        position: 0,
        targetPlayerId: 1,
        targetZone: 'hand',
        x: 0,
        y: 0,
        newCardId: -1,
        faceDown: false,
        newCardProviderId: '',
      },
    });

    expect(result.games[1].players[1].zones['deck'].cards).toHaveLength(0);
    expect(result.games[1].players[1].zones['hand'].cards[0].id).toBe(11);
  });

  it('hidden-zone move: cardId < 0, position out of range → decrements source cardCount, builds empty card in target', () => {
    const state = makeState({
      games: {
        1: makeGameEntry({
          players: {
            1: makePlayerEntry({
              zones: {
                deck: makeZoneEntry({ name: 'deck', cards: [], cardCount: 5 }),
                hand: makeZoneEntry({ name: 'hand', cards: [], cardCount: 0 }),
              },
            }),
          },
        }),
      },
    });

    const result = gamesReducer(state, {
      type: Types.CARD_MOVED,
      gameId: 1,
      playerId: 1,
      data: {
        cardId: -1,
        cardName: 'Hidden',
        startPlayerId: 1,
        startZone: 'deck',
        position: 99,
        targetPlayerId: 1,
        targetZone: 'hand',
        x: 0,
        y: 0,
        newCardId: 7,
        faceDown: true,
        newCardProviderId: 'prov',
      },
    });

    expect(result.games[1].players[1].zones['deck'].cardCount).toBe(4);
    const movedCard = result.games[1].players[1].zones['hand'].cards[0];
    expect(movedCard.id).toBe(7);
    expect(movedCard.name).toBe('Hidden');
    expect(movedCard.faceDown).toBe(true);
  });

  it('cross-player move: card leaves source player zone and enters target player zone', () => {
    const card = makeCard({ id: 20 });
    const state = makeState({
      games: {
        1: makeGameEntry({
          players: {
            1: makePlayerEntry({
              zones: {
                hand: makeZoneEntry({ name: 'hand', cards: [card], cardCount: 1 }),
              },
            }),
            2: makePlayerEntry({
              properties: makePlayerProperties({ playerId: 2 }),
              zones: {
                hand: makeZoneEntry({ name: 'hand', cardCount: 0 }),
              },
            }),
          },
        }),
      },
    });

    const result = gamesReducer(state, {
      type: Types.CARD_MOVED,
      gameId: 1,
      playerId: 1,
      data: {
        cardId: 20,
        cardName: '',
        startPlayerId: 1,
        startZone: 'hand',
        position: -1,
        targetPlayerId: 2,
        targetZone: 'hand',
        x: 0,
        y: 0,
        newCardId: -1,
        faceDown: false,
        newCardProviderId: '',
      },
    });

    expect(result.games[1].players[1].zones['hand'].cards).toHaveLength(0);
    expect(result.games[1].players[2].zones['hand'].cards[0].id).toBe(20);
  });

  it('assigns newCardId when newCardId ≥ 0', () => {
    const { state } = stateWithCard();
    const result = gamesReducer(state, {
      type: Types.CARD_MOVED,
      gameId: 1,
      playerId: 1,
      data: {
        cardId: 10,
        cardName: '',
        startPlayerId: 1,
        startZone: 'hand',
        position: -1,
        targetPlayerId: 1,
        targetZone: 'table',
        x: 0,
        y: 0,
        newCardId: 999,
        faceDown: false,
        newCardProviderId: '',
      },
    });

    expect(result.games[1].players[1].zones['table'].cards[0].id).toBe(999);
  });

  it('applies newCardProviderId and cardName to moved card', () => {
    const { state } = stateWithCard({ name: 'Old Name', providerId: 'old-prov' });
    const result = gamesReducer(state, {
      type: Types.CARD_MOVED,
      gameId: 1,
      playerId: 1,
      data: {
        cardId: 10,
        cardName: 'New Name',
        startPlayerId: 1,
        startZone: 'hand',
        position: -1,
        targetPlayerId: 1,
        targetZone: 'table',
        x: 0,
        y: 0,
        newCardId: -1,
        faceDown: false,
        newCardProviderId: 'new-prov',
      },
    });

    const moved = result.games[1].players[1].zones['table'].cards[0];
    expect(moved.name).toBe('New Name');
    expect(moved.providerId).toBe('new-prov');
  });

  it('CARD_MOVED → returns newState (card removed from source) when targetZone does not exist on player', () => {
    const { state } = stateWithCard();
    const result = gamesReducer(state, {
      type: Types.CARD_MOVED,
      gameId: 1,
      playerId: 1,
      data: {
        cardId: 10,
        cardName: '',
        startPlayerId: 1,
        startZone: 'hand',
        position: -1,
        targetPlayerId: 1,
        targetZone: 'nonexistent',
        x: 0,
        y: 0,
        newCardId: -1,
        faceDown: false,
        newCardProviderId: '',
      },
    });
    expect(result.games[1].players[1].zones['hand'].cards).toHaveLength(0);
    expect(result.games[1].players[1].zones['nonexistent']).toBeUndefined();
  });
});

// ── 2D: Card mutations ────────────────────────────────────────────────────────

describe('2D: Card mutations', () => {
  function stateWithCardInZone(zoneName: string) {
    const card = makeCard({ id: 5, name: 'Old', providerId: 'old', faceDown: false });
    return {
      card,
      state: makeState({
        games: {
          1: makeGameEntry({
            players: {
              1: makePlayerEntry({
                zones: {
                  [zoneName]: makeZoneEntry({ name: zoneName, cards: [card], cardCount: 1 }),
                },
              }),
            },
          }),
        },
      }),
    };
  }

  it('CARD_FLIPPED → updates faceDown, name, and providerId', () => {
    const { state } = stateWithCardInZone('hand');
    const result = gamesReducer(state, {
      type: Types.CARD_FLIPPED,
      gameId: 1,
      playerId: 1,
      data: { zoneName: 'hand', cardId: 5, cardName: 'Revealed', faceDown: true, cardProviderId: 'new-prov' },
    });

    const card = result.games[1].players[1].zones['hand'].cards[0];
    expect(card.faceDown).toBe(true);
    expect(card.name).toBe('Revealed');
    expect(card.providerId).toBe('new-prov');
  });

  it('CARD_DESTROYED → removes card from zone and decrements cardCount', () => {
    const { state } = stateWithCardInZone('hand');
    const result = gamesReducer(state, {
      type: Types.CARD_DESTROYED,
      gameId: 1,
      playerId: 1,
      data: { zoneName: 'hand', cardId: 5 },
    });

    expect(result.games[1].players[1].zones['hand'].cards).toHaveLength(0);
    expect(result.games[1].players[1].zones['hand'].cardCount).toBe(0);
  });

  it('CARD_ATTACHED → sets attachPlayerId, attachZone, attachCardId on matched card', () => {
    const { state } = stateWithCardInZone('table');
    const result = gamesReducer(state, {
      type: Types.CARD_ATTACHED,
      gameId: 1,
      playerId: 1,
      data: { startZone: 'table', cardId: 5, targetPlayerId: 2, targetZone: 'table', targetCardId: 99 },
    });

    const card = result.games[1].players[1].zones['table'].cards[0];
    expect(card.attachPlayerId).toBe(2);
    expect(card.attachZone).toBe('table');
    expect(card.attachCardId).toBe(99);
  });

  it('TOKEN_CREATED → builds full CardInfo, appends to zone, increments cardCount', () => {
    const state = makeState({
      games: {
        1: makeGameEntry({
          players: {
            1: makePlayerEntry({
              zones: {
                table: makeZoneEntry({ name: 'table', cards: [], cardCount: 0 }),
              },
            }),
          },
        }),
      },
    });

    const result = gamesReducer(state, {
      type: Types.TOKEN_CREATED,
      gameId: 1,
      playerId: 1,
      data: {
        zoneName: 'table',
        cardId: 77,
        cardName: 'Goblin',
        color: 'red',
        pt: '1/1',
        annotation: '',
        destroyOnZoneChange: true,
        x: 3,
        y: 4,
        cardProviderId: 'prov',
        faceDown: false,
      },
    });

    const zone = result.games[1].players[1].zones['table'];
    expect(zone.cardCount).toBe(1);
    expect(zone.cards[0].id).toBe(77);
    expect(zone.cards[0].name).toBe('Goblin');
    expect(zone.cards[0].destroyOnZoneChange).toBe(true);
  });
});

// ── 2E: CARD_ATTR_CHANGED ─────────────────────────────────────────────────────

describe('2E: CARD_ATTR_CHANGED', () => {
  function stateWithCard() {
    const card = makeCard({
      id: 3,
      tapped: false,
      attacking: false,
      faceDown: false,
      color: '',
      pt: '',
      annotation: '',
      doesntUntap: false,
    });
    return makeState({
      games: {
        1: makeGameEntry({
          players: {
            1: makePlayerEntry({
              zones: {
                table: makeZoneEntry({ name: 'table', cards: [card], cardCount: 1 }),
              },
            }),
          },
        }),
      },
    });
  }

  function dispatchAttr(state: ReturnType<typeof makeState>, attribute: CardAttribute, attrValue: string) {
    return gamesReducer(state, {
      type: Types.CARD_ATTR_CHANGED,
      gameId: 1,
      playerId: 1,
      data: { zoneName: 'table', cardId: 3, attribute, attrValue },
    });
  }

  it('AttrTapped (1) → card.tapped = true when attrValue is "1"', () => {
    const result = dispatchAttr(stateWithCard(), CardAttribute.AttrTapped, '1');
    expect(result.games[1].players[1].zones['table'].cards[0].tapped).toBe(true);
  });

  it('AttrAttacking (2) → card.attacking = true when attrValue is "1"', () => {
    const result = dispatchAttr(stateWithCard(), CardAttribute.AttrAttacking, '1');
    expect(result.games[1].players[1].zones['table'].cards[0].attacking).toBe(true);
  });

  it('AttrFaceDown (3) → card.faceDown = true when attrValue is "1"', () => {
    const result = dispatchAttr(stateWithCard(), CardAttribute.AttrFaceDown, '1');
    expect(result.games[1].players[1].zones['table'].cards[0].faceDown).toBe(true);
  });

  it('AttrColor (4) → card.color = attrValue', () => {
    const result = dispatchAttr(stateWithCard(), CardAttribute.AttrColor, 'red');
    expect(result.games[1].players[1].zones['table'].cards[0].color).toBe('red');
  });

  it('AttrPT (5) → card.pt = attrValue', () => {
    const result = dispatchAttr(stateWithCard(), CardAttribute.AttrPT, '2/3');
    expect(result.games[1].players[1].zones['table'].cards[0].pt).toBe('2/3');
  });

  it('AttrAnnotation (6) → card.annotation = attrValue', () => {
    const result = dispatchAttr(stateWithCard(), CardAttribute.AttrAnnotation, 'enchanted');
    expect(result.games[1].players[1].zones['table'].cards[0].annotation).toBe('enchanted');
  });

  it('AttrDoesntUntap (7) → card.doesntUntap = true when attrValue is "1"', () => {
    const result = dispatchAttr(stateWithCard(), CardAttribute.AttrDoesntUntap, '1');
    expect(result.games[1].players[1].zones['table'].cards[0].doesntUntap).toBe(true);
  });
});

// ── 2F: CARD_COUNTER_CHANGED ─────────────────────────────────────────────────

describe('2F: CARD_COUNTER_CHANGED', () => {
  function stateWithCard(existingCounters: any[] = []) {
    const card = makeCard({ id: 4, counterList: existingCounters });
    return makeState({
      games: {
        1: makeGameEntry({
          players: {
            1: makePlayerEntry({
              zones: {
                table: makeZoneEntry({ name: 'table', cards: [card], cardCount: 1 }),
              },
            }),
          },
        }),
      },
    });
  }

  it('adds new counter to counterList when counterId not present and counterValue > 0', () => {
    const state = stateWithCard([]);
    const result = gamesReducer(state, {
      type: Types.CARD_COUNTER_CHANGED,
      gameId: 1,
      playerId: 1,
      data: { zoneName: 'table', cardId: 4, counterId: 1, counterValue: 3 },
    });
    expect(result.games[1].players[1].zones['table'].cards[0].counterList).toEqual([{ id: 1, value: 3 }]);
  });

  it('updates existing counter value when counterId matches', () => {
    const state = stateWithCard([{ id: 1, value: 3 }]);
    const result = gamesReducer(state, {
      type: Types.CARD_COUNTER_CHANGED,
      gameId: 1,
      playerId: 1,
      data: { zoneName: 'table', cardId: 4, counterId: 1, counterValue: 7 },
    });
    expect(result.games[1].players[1].zones['table'].cards[0].counterList).toEqual([{ id: 1, value: 7 }]);
  });

  it('removes counter from counterList when counterValue ≤ 0', () => {
    const state = stateWithCard([{ id: 1, value: 3 }]);
    const result = gamesReducer(state, {
      type: Types.CARD_COUNTER_CHANGED,
      gameId: 1,
      playerId: 1,
      data: { zoneName: 'table', cardId: 4, counterId: 1, counterValue: 0 },
    });
    expect(result.games[1].players[1].zones['table'].cards[0].counterList).toEqual([]);
  });
});

// ── 2G: Arrows ────────────────────────────────────────────────────────────────

describe('2G: Arrows', () => {
  it('ARROW_CREATED → inserts arrowInfo into player.arrows keyed by id', () => {
    const state = makeState();
    const arrow = makeArrow({ id: 9 });
    const result = gamesReducer(state, {
      type: Types.ARROW_CREATED,
      gameId: 1,
      playerId: 1,
      data: { arrowInfo: arrow },
    });
    expect(result.games[1].players[1].arrows[9]).toEqual(arrow);
  });

  it('ARROW_DELETED → removes arrow from player.arrows by arrowId', () => {
    const arrow = makeArrow({ id: 9 });
    const state = makeState({
      games: {
        1: makeGameEntry({
          players: {
            1: makePlayerEntry({ arrows: { 9: arrow } }),
          },
        }),
      },
    });
    const result = gamesReducer(state, {
      type: Types.ARROW_DELETED,
      gameId: 1,
      playerId: 1,
      data: { arrowId: 9 },
    });
    expect(result.games[1].players[1].arrows[9]).toBeUndefined();
  });
});

// ── 2H: Player counters ───────────────────────────────────────────────────────

describe('2H: Player counters', () => {
  it('COUNTER_CREATED → inserts counterInfo into player.counters keyed by id', () => {
    const state = makeState();
    const counter = makeCounter({ id: 5, name: 'Poison' });
    const result = gamesReducer(state, {
      type: Types.COUNTER_CREATED,
      gameId: 1,
      playerId: 1,
      data: { counterInfo: counter },
    });
    expect(result.games[1].players[1].counters[5]).toEqual(counter);
  });

  it('COUNTER_SET → updates counter.count to new value', () => {
    const counter = makeCounter({ id: 5, count: 20 });
    const state = makeState({
      games: {
        1: makeGameEntry({
          players: {
            1: makePlayerEntry({ counters: { 5: counter } }),
          },
        }),
      },
    });
    const result = gamesReducer(state, {
      type: Types.COUNTER_SET,
      gameId: 1,
      playerId: 1,
      data: { counterId: 5, value: 14 },
    });
    expect(result.games[1].players[1].counters[5].count).toBe(14);
  });

  it('COUNTER_DELETED → removes counter from player.counters by counterId', () => {
    const counter = makeCounter({ id: 5 });
    const state = makeState({
      games: {
        1: makeGameEntry({
          players: {
            1: makePlayerEntry({ counters: { 5: counter } }),
          },
        }),
      },
    });
    const result = gamesReducer(state, {
      type: Types.COUNTER_DELETED,
      gameId: 1,
      playerId: 1,
      data: { counterId: 5 },
    });
    expect(result.games[1].players[1].counters[5]).toBeUndefined();
  });
});

// ── 2I: Zone operations ───────────────────────────────────────────────────────

describe('2I: Zone operations', () => {
  it('CARDS_DRAWN → decrements deck.cardCount, appends cards to hand, increments hand.cardCount', () => {
    const drawnCard = makeCard({ id: 9 });
    const state = makeState({
      games: {
        1: makeGameEntry({
          players: {
            1: makePlayerEntry({
              zones: {
                deck: makeZoneEntry({ name: 'deck', cardCount: 40 }),
                hand: makeZoneEntry({ name: 'hand', cards: [], cardCount: 0 }),
              },
            }),
          },
        }),
      },
    });

    const result = gamesReducer(state, {
      type: Types.CARDS_DRAWN,
      gameId: 1,
      playerId: 1,
      data: { number: 2, cards: [drawnCard] },
    });

    expect(result.games[1].players[1].zones['deck'].cardCount).toBe(38);
    expect(result.games[1].players[1].zones['hand'].cards).toContainEqual(drawnCard);
    expect(result.games[1].players[1].zones['hand'].cardCount).toBe(2);
  });

  it('CARDS_DRAWN → works when no deck zone exists (only updates hand)', () => {
    const drawnCard = makeCard({ id: 9 });
    const state = makeState({
      games: {
        1: makeGameEntry({
          players: {
            1: makePlayerEntry({
              zones: {
                hand: makeZoneEntry({ name: 'hand', cards: [], cardCount: 0 }),
              },
            }),
          },
        }),
      },
    });

    const result = gamesReducer(state, {
      type: Types.CARDS_DRAWN,
      gameId: 1,
      playerId: 1,
      data: { number: 1, cards: [drawnCard] },
    });

    expect(result.games[1].players[1].zones['hand'].cardCount).toBe(1);
    expect(result.games[1].players[1].zones['hand'].cards).toContainEqual(drawnCard);
  });

  it('CARDS_REVEALED (update path) → merges revealed cards into existing zone cards', () => {
    const existing = makeCard({ id: 2, name: 'Old Name' });
    const state = makeState({
      games: {
        1: makeGameEntry({
          players: {
            1: makePlayerEntry({
              zones: {
                deck: makeZoneEntry({ name: 'deck', cards: [existing], cardCount: 1 }),
              },
            }),
          },
        }),
      },
    });

    const result = gamesReducer(state, {
      type: Types.CARDS_REVEALED,
      gameId: 1,
      playerId: 1,
      data: { zoneName: 'deck', cards: [{ ...existing, name: 'Revealed Name' }] },
    });

    expect(result.games[1].players[1].zones['deck'].cards[0].name).toBe('Revealed Name');
    expect(result.games[1].players[1].zones['deck'].cards).toHaveLength(1);
  });

  it('CARDS_REVEALED (append path) → appends new cards whose ids are not already in the zone', () => {
    const existing = makeCard({ id: 1 });
    const newCard = makeCard({ id: 2 });
    const state = makeState({
      games: {
        1: makeGameEntry({
          players: {
            1: makePlayerEntry({
              zones: {
                deck: makeZoneEntry({ name: 'deck', cards: [existing], cardCount: 1 }),
              },
            }),
          },
        }),
      },
    });

    const result = gamesReducer(state, {
      type: Types.CARDS_REVEALED,
      gameId: 1,
      playerId: 1,
      data: { zoneName: 'deck', cards: [newCard] },
    });

    expect(result.games[1].players[1].zones['deck'].cards).toHaveLength(2);
    expect(result.games[1].players[1].zones['deck'].cards[1]).toEqual(newCard);
  });

  it('ZONE_PROPERTIES_CHANGED → sets alwaysRevealTopCard and alwaysLookAtTopCard', () => {
    const state = makeState();
    const result = gamesReducer(state, {
      type: Types.ZONE_PROPERTIES_CHANGED,
      gameId: 1,
      playerId: 1,
      data: { zoneName: 'hand', alwaysRevealTopCard: true, alwaysLookAtTopCard: true },
    });

    const zone = result.games[1].players[1].zones['hand'];
    expect(zone.alwaysRevealTopCard).toBe(true);
    expect(zone.alwaysLookAtTopCard).toBe(true);
  });
});

// ── 2J: Turn / phase / chat ───────────────────────────────────────────────────

describe('2J: Turn, phase, and chat', () => {
  it('ACTIVE_PLAYER_SET → sets game.activePlayerId', () => {
    const state = makeState();
    const result = gamesReducer(state, { type: Types.ACTIVE_PLAYER_SET, gameId: 1, activePlayerId: 3 });
    expect(result.games[1].activePlayerId).toBe(3);
  });

  it('ACTIVE_PHASE_SET → sets game.activePhase', () => {
    const state = makeState();
    const result = gamesReducer(state, { type: Types.ACTIVE_PHASE_SET, gameId: 1, phase: 5 });
    expect(result.games[1].activePhase).toBe(5);
  });

  it('TURN_REVERSED → sets game.reversed', () => {
    const state = makeState();
    const result = gamesReducer(state, { type: Types.TURN_REVERSED, gameId: 1, reversed: true });
    expect(result.games[1].reversed).toBe(true);
  });

  it('GAME_SAY → appends message with mocked Date.now() as timeReceived', () => {
    const state = makeState();
    jest.spyOn(Date, 'now').mockReturnValue(123456789);
    const result = gamesReducer(state, {
      type: Types.GAME_SAY,
      gameId: 1,
      playerId: 2,
      message: 'gg',
    });
    jest.restoreAllMocks();

    expect(result.games[1].messages).toHaveLength(1);
    expect(result.games[1].messages[0]).toEqual({ playerId: 2, message: 'gg', timeReceived: 123456789 });
  });
});

// ── 2K: No-op / passthrough actions ──────────────────────────────────────────

describe('2K: No-op / passthrough actions', () => {
  it('ZONE_SHUFFLED → returns state unchanged (identity)', () => {
    const state = makeState();
    const result = gamesReducer(state, { type: Types.ZONE_SHUFFLED, gameId: 1, playerId: 1 });
    expect(result).toBe(state);
  });

  it('ZONE_DUMPED → returns state unchanged (identity)', () => {
    const state = makeState();
    const result = gamesReducer(state, { type: Types.ZONE_DUMPED, gameId: 1, playerId: 1 });
    expect(result).toBe(state);
  });

  it('DIE_ROLLED → returns state unchanged (identity)', () => {
    const state = makeState();
    const result = gamesReducer(state, { type: Types.DIE_ROLLED, gameId: 1, playerId: 1 });
    expect(result).toBe(state);
  });

  it('unknown action type → returns state unchanged (identity)', () => {
    const state = makeState();
    const result = gamesReducer(state, { type: 'UNKNOWN_ACTION_COMPLETELY' });
    expect(result).toBe(state);
  });
});

// ── 2L: Null-guard / missing entity early-returns ─────────────────────────────
// Each test dispatches an action with a non-existent gameId (999) or playerId/zone
// to exercise the `if (!game) return state` / `if (!player) return state` guards.

describe('2L: Null-guard / missing entity early-returns', () => {
  const UNKNOWN_GAME = 999;
  const UNKNOWN_PLAYER = 999;

  it('updateGame guard: GAME_HOST_CHANGED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, { type: Types.GAME_HOST_CHANGED, gameId: UNKNOWN_GAME, hostId: 1 })).toBe(state);
  });

  it('GAME_STATE_CHANGED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, { type: Types.GAME_STATE_CHANGED, gameId: UNKNOWN_GAME, data: {} })).toBe(state);
  });

  it('PLAYER_JOINED with unknown gameId → state unchanged', () => {
    const state = makeState();
    const props = makePlayerProperties({ playerId: 5 });
    expect(gamesReducer(state, { type: Types.PLAYER_JOINED, gameId: UNKNOWN_GAME, playerProperties: props })).toBe(state);
  });

  it('PLAYER_LEFT with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, { type: Types.PLAYER_LEFT, gameId: UNKNOWN_GAME, playerId: 1 })).toBe(state);
  });

  it('updatePlayer guard: PLAYER_PROPERTIES_CHANGED with unknown gameId → state unchanged', () => {
    const state = makeState();
    const props = makePlayerProperties({ playerId: 1 });
    expect(gamesReducer(state, {
      type: Types.PLAYER_PROPERTIES_CHANGED, gameId: UNKNOWN_GAME, playerId: 1, properties: props,
    })).toBe(state);
  });

  it('updatePlayer guard: PLAYER_PROPERTIES_CHANGED with unknown playerId → state unchanged', () => {
    const state = makeState();
    const props = makePlayerProperties({ playerId: UNKNOWN_PLAYER });
    expect(gamesReducer(state, {
      type: Types.PLAYER_PROPERTIES_CHANGED, gameId: 1, playerId: UNKNOWN_PLAYER, properties: props,
    })).toBe(state);
  });

  it('CARD_MOVED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_MOVED, gameId: UNKNOWN_GAME, playerId: 1,
      data: {
        cardId: 1, cardName: '', startPlayerId: 1, startZone: 'hand', position: -1,
        targetPlayerId: 1, targetZone: 'hand', x: 0, y: 0, newCardId: -1, faceDown: false, newCardProviderId: '',
      },
    })).toBe(state);
  });

  it('CARD_MOVED with unknown sourcePlayer → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_MOVED, gameId: 1, playerId: 1,
      data: {
        cardId: 1, cardName: '', startPlayerId: UNKNOWN_PLAYER, startZone: 'hand', position: -1,
        targetPlayerId: 1, targetZone: 'hand', x: 0, y: 0, newCardId: -1, faceDown: false, newCardProviderId: '',
      },
    })).toBe(state);
  });

  it('CARD_MOVED with unknown sourceZone → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_MOVED, gameId: 1, playerId: 1,
      data: {
        cardId: 1, cardName: '', startPlayerId: 1, startZone: 'nonexistent', position: -1,
        targetPlayerId: 1, targetZone: 'hand', x: 0, y: 0, newCardId: -1, faceDown: false, newCardProviderId: '',
      },
    })).toBe(state);
  });

  it('CARD_FLIPPED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_FLIPPED, gameId: UNKNOWN_GAME, playerId: 1,
      data: { zoneName: 'hand', cardId: 1, cardName: '', faceDown: false, cardProviderId: '' },
    })).toBe(state);
  });

  it('CARD_FLIPPED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_FLIPPED, gameId: 1, playerId: UNKNOWN_PLAYER,
      data: { zoneName: 'hand', cardId: 1, cardName: '', faceDown: false, cardProviderId: '' },
    })).toBe(state);
  });

  it('CARD_FLIPPED with unknown zone → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_FLIPPED, gameId: 1, playerId: 1,
      data: { zoneName: 'nonexistent', cardId: 1, cardName: '', faceDown: false, cardProviderId: '' },
    })).toBe(state);
  });

  it('CARD_FLIPPED with unknown cardId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_FLIPPED, gameId: 1, playerId: 1,
      data: { zoneName: 'hand', cardId: 9999, cardName: '', faceDown: false, cardProviderId: '' },
    })).toBe(state);
  });

  it('CARD_DESTROYED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_DESTROYED, gameId: UNKNOWN_GAME, playerId: 1,
      data: { zoneName: 'hand', cardId: 1 },
    })).toBe(state);
  });

  it('CARD_DESTROYED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_DESTROYED, gameId: 1, playerId: UNKNOWN_PLAYER,
      data: { zoneName: 'hand', cardId: 1 },
    })).toBe(state);
  });

  it('CARD_DESTROYED with unknown zone → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_DESTROYED, gameId: 1, playerId: 1,
      data: { zoneName: 'nonexistent', cardId: 1 },
    })).toBe(state);
  });

  it('CARD_ATTACHED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_ATTACHED, gameId: UNKNOWN_GAME, playerId: 1,
      data: { startZone: 'hand', cardId: 1, targetPlayerId: 1, targetZone: 'hand', targetCardId: 1 },
    })).toBe(state);
  });

  it('CARD_ATTACHED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_ATTACHED, gameId: 1, playerId: UNKNOWN_PLAYER,
      data: { startZone: 'hand', cardId: 1, targetPlayerId: 1, targetZone: 'hand', targetCardId: 1 },
    })).toBe(state);
  });

  it('CARD_ATTACHED with unknown zone → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_ATTACHED, gameId: 1, playerId: 1,
      data: { startZone: 'nonexistent', cardId: 1, targetPlayerId: 1, targetZone: 'hand', targetCardId: 1 },
    })).toBe(state);
  });

  it('CARD_ATTACHED with unknown cardId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_ATTACHED, gameId: 1, playerId: 1,
      data: { startZone: 'hand', cardId: 9999, targetPlayerId: 1, targetZone: 'hand', targetCardId: 1 },
    })).toBe(state);
  });

  it('TOKEN_CREATED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.TOKEN_CREATED, gameId: UNKNOWN_GAME, playerId: 1,
      data: {
        zoneName: 'hand', cardId: 1, cardName: 'T', color: '', pt: '', annotation: '',
        destroyOnZoneChange: false, x: 0, y: 0, cardProviderId: '', faceDown: false,
      },
    })).toBe(state);
  });

  it('TOKEN_CREATED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.TOKEN_CREATED, gameId: 1, playerId: UNKNOWN_PLAYER,
      data: {
        zoneName: 'hand', cardId: 1, cardName: 'T', color: '', pt: '', annotation: '',
        destroyOnZoneChange: false, x: 0, y: 0, cardProviderId: '', faceDown: false,
      },
    })).toBe(state);
  });

  it('TOKEN_CREATED with unknown zone → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.TOKEN_CREATED, gameId: 1, playerId: 1,
      data: {
        zoneName: 'nonexistent', cardId: 1, cardName: 'T', color: '', pt: '', annotation: '',
        destroyOnZoneChange: false, x: 0, y: 0, cardProviderId: '', faceDown: false,
      },
    })).toBe(state);
  });

  it('CARD_ATTR_CHANGED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_ATTR_CHANGED, gameId: UNKNOWN_GAME, playerId: 1,
      data: { zoneName: 'hand', cardId: 1, attribute: 1, attrValue: '1' },
    })).toBe(state);
  });

  it('CARD_ATTR_CHANGED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_ATTR_CHANGED, gameId: 1, playerId: UNKNOWN_PLAYER,
      data: { zoneName: 'hand', cardId: 1, attribute: 1, attrValue: '1' },
    })).toBe(state);
  });

  it('CARD_ATTR_CHANGED with unknown zone → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_ATTR_CHANGED, gameId: 1, playerId: 1,
      data: { zoneName: 'nonexistent', cardId: 1, attribute: 1, attrValue: '1' },
    })).toBe(state);
  });

  it('CARD_ATTR_CHANGED with unknown cardId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_ATTR_CHANGED, gameId: 1, playerId: 1,
      data: { zoneName: 'hand', cardId: 9999, attribute: 1, attrValue: '1' },
    })).toBe(state);
  });

  it('CARD_COUNTER_CHANGED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_COUNTER_CHANGED, gameId: UNKNOWN_GAME, playerId: 1,
      data: { zoneName: 'hand', cardId: 1, counterId: 1, counterValue: 1 },
    })).toBe(state);
  });

  it('CARD_COUNTER_CHANGED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_COUNTER_CHANGED, gameId: 1, playerId: UNKNOWN_PLAYER,
      data: { zoneName: 'hand', cardId: 1, counterId: 1, counterValue: 1 },
    })).toBe(state);
  });

  it('CARD_COUNTER_CHANGED with unknown zone → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_COUNTER_CHANGED, gameId: 1, playerId: 1,
      data: { zoneName: 'nonexistent', cardId: 1, counterId: 1, counterValue: 1 },
    })).toBe(state);
  });

  it('CARD_COUNTER_CHANGED with unknown cardId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARD_COUNTER_CHANGED, gameId: 1, playerId: 1,
      data: { zoneName: 'hand', cardId: 9999, counterId: 1, counterValue: 1 },
    })).toBe(state);
  });

  it('ARROW_CREATED with unknown gameId → state unchanged', () => {
    const state = makeState();
    const arrow = makeArrow({ id: 1 });
    expect(gamesReducer(state, { type: Types.ARROW_CREATED, gameId: UNKNOWN_GAME, playerId: 1, data: { arrowInfo: arrow } })).toBe(state);
  });

  it('ARROW_CREATED with unknown playerId → state unchanged', () => {
    const state = makeState();
    const arrow = makeArrow({ id: 1 });
    expect(gamesReducer(state, { type: Types.ARROW_CREATED, gameId: 1, playerId: UNKNOWN_PLAYER, data: { arrowInfo: arrow } })).toBe(state);
  });

  it('ARROW_DELETED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, { type: Types.ARROW_DELETED, gameId: UNKNOWN_GAME, playerId: 1, data: { arrowId: 1 } })).toBe(state);
  });

  it('ARROW_DELETED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, { type: Types.ARROW_DELETED, gameId: 1, playerId: UNKNOWN_PLAYER, data: { arrowId: 1 } })).toBe(state);
  });

  it('COUNTER_CREATED with unknown gameId → state unchanged', () => {
    const state = makeState();
    const counter = makeCounter({ id: 1 });
    expect(gamesReducer(state, {
      type: Types.COUNTER_CREATED, gameId: UNKNOWN_GAME, playerId: 1, data: { counterInfo: counter },
    })).toBe(state);
  });

  it('COUNTER_CREATED with unknown playerId → state unchanged', () => {
    const state = makeState();
    const counter = makeCounter({ id: 1 });
    expect(gamesReducer(state, {
      type: Types.COUNTER_CREATED, gameId: 1, playerId: UNKNOWN_PLAYER, data: { counterInfo: counter },
    })).toBe(state);
  });

  it('COUNTER_SET with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.COUNTER_SET, gameId: UNKNOWN_GAME, playerId: 1, data: { counterId: 1, value: 5 },
    })).toBe(state);
  });

  it('COUNTER_SET with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.COUNTER_SET, gameId: 1, playerId: UNKNOWN_PLAYER, data: { counterId: 1, value: 5 },
    })).toBe(state);
  });

  it('COUNTER_SET with unknown counterId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, { type: Types.COUNTER_SET, gameId: 1, playerId: 1, data: { counterId: 9999, value: 5 } })).toBe(state);
  });

  it('COUNTER_DELETED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, { type: Types.COUNTER_DELETED, gameId: UNKNOWN_GAME, playerId: 1, data: { counterId: 1 } })).toBe(state);
  });

  it('COUNTER_DELETED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, { type: Types.COUNTER_DELETED, gameId: 1, playerId: UNKNOWN_PLAYER, data: { counterId: 1 } })).toBe(state);
  });

  it('CARDS_DRAWN with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARDS_DRAWN, gameId: UNKNOWN_GAME, playerId: 1, data: { number: 1, cards: [] },
    })).toBe(state);
  });

  it('CARDS_DRAWN with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARDS_DRAWN, gameId: 1, playerId: UNKNOWN_PLAYER, data: { number: 1, cards: [] }
    })).toBe(state);
  });

  it('CARDS_DRAWN with no hand zone → state unchanged', () => {
    const state = makeState({
      games: {
        1: makeGameEntry({
          players: {
            1: makePlayerEntry({ zones: { deck: makeZoneEntry({ name: 'deck', cardCount: 10 }) } }),
          },
        }),
      },
    });
    expect(gamesReducer(state, { type: Types.CARDS_DRAWN, gameId: 1, playerId: 1, data: { number: 1, cards: [] } })).toBe(state);
  });

  it('CARDS_REVEALED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARDS_REVEALED, gameId: UNKNOWN_GAME, playerId: 1, data: { zoneName: 'hand', cards: [] },
    })).toBe(state);
  });

  it('CARDS_REVEALED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARDS_REVEALED, gameId: 1, playerId: UNKNOWN_PLAYER, data: { zoneName: 'hand', cards: [] },
    })).toBe(state);
  });

  it('CARDS_REVEALED with unknown zone → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.CARDS_REVEALED, gameId: 1, playerId: 1, data: { zoneName: 'nonexistent', cards: [] },
    })).toBe(state);
  });

  it('updateZone guard: ZONE_PROPERTIES_CHANGED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.ZONE_PROPERTIES_CHANGED, gameId: UNKNOWN_GAME, playerId: 1,
      data: { zoneName: 'hand', alwaysRevealTopCard: true, alwaysLookAtTopCard: true },
    })).toBe(state);
  });

  it('updateZone guard: ZONE_PROPERTIES_CHANGED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.ZONE_PROPERTIES_CHANGED, gameId: 1, playerId: UNKNOWN_PLAYER,
      data: { zoneName: 'hand', alwaysRevealTopCard: true, alwaysLookAtTopCard: true },
    })).toBe(state);
  });

  it('updateZone guard: ZONE_PROPERTIES_CHANGED with unknown zone → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, {
      type: Types.ZONE_PROPERTIES_CHANGED, gameId: 1, playerId: 1,
      data: { zoneName: 'nonexistent', alwaysRevealTopCard: true, alwaysLookAtTopCard: true },
    })).toBe(state);
  });

  it('GAME_SAY with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, { type: Types.GAME_SAY, gameId: UNKNOWN_GAME, playerId: 1, message: 'hi' })).toBe(state);
  });
});
