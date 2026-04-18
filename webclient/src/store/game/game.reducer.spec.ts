import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';
import { gamesReducer } from './game.reducer';
import { GamesState } from './game.interfaces';
import { Actions } from './game.actions';
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

/** Materialize a zone's ordered card array via its normalized {order, byId} shape. */
function cardsIn(state: GamesState, gameId: number, playerId: number, zoneName: string): Data.ServerInfo_Card[] {
  const zone = state.games[gameId]?.players[playerId]?.zones[zoneName];
  return zone ? zone.order.map(id => zone.byId[id]) : [];
}

// ── 2A: Initialisation & lifecycle ───────────────────────────────────────────

describe('2A: Initialisation & lifecycle', () => {
  it('returns initialState ({ games: {} }) when called with undefined state', () => {
    const result = gamesReducer(undefined, { type: '@@INIT' });
    expect(result).toEqual({ games: {} });
  });

  it('CLEAR_STORE → resets to initialState', () => {
    const state = makeState();
    const result = gamesReducer(state, Actions.clearStore());
    expect(result).toEqual({ games: {} });
  });

  it('GAME_JOINED → inserts gameEntry keyed by gameId', () => {
    const data = create(Data.Event_GameJoinedSchema, {
      gameInfo: create(Data.ServerInfo_GameSchema, { gameId: 42, roomId: 1, description: 'test' }),
      hostId: 5,
      playerId: 2,
      spectator: false,
      judge: false,
      resuming: false,
    });
    const result = gamesReducer({ games: {} }, Actions.gameJoined({ data }));
    const entry = result.games[42];
    expect(entry).toBeDefined();
    expect(entry.info.gameId).toBe(42);
    expect(entry.hostId).toBe(5);
    expect(entry.localPlayerId).toBe(2);
  });

  it('GAME_LEFT → removes game by gameId', () => {
    const state = makeState();
    const result = gamesReducer(state, Actions.gameLeft({ gameId: 1 }));
    expect(result.games[1]).toBeUndefined();
  });

  it('GAME_CLOSED → removes game by gameId', () => {
    const state = makeState();
    const result = gamesReducer(state, Actions.gameClosed({ gameId: 1 }));
    expect(result.games[1]).toBeUndefined();
  });

  it('KICKED → removes game by gameId', () => {
    const state = makeState();
    const result = gamesReducer(state, Actions.kicked({ gameId: 1 }));
    expect(result.games[1]).toBeUndefined();
  });

  it('GAME_HOST_CHANGED → updates hostId on existing game', () => {
    const state = makeState();
    const result = gamesReducer(state, Actions.gameHostChanged({ gameId: 1, hostId: 99 }));
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
    const playerList: Data.ServerInfo_Player[] = [
      create(Data.ServerInfo_PlayerSchema, {
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
      }),
    ];

    const result = gamesReducer(state, Actions.gameStateChanged({
      gameId: 1,
      data: { playerList },
    }));

    const player = result.games[1].players[7];
    expect(player).toBeDefined();
    expect(cardsIn(result, 1, 7, 'hand')[0]).toEqual(card);
    expect(player.counters[2]).toEqual(counter);
    expect(player.arrows[3]).toEqual(arrow);
  });

  it('GAME_STATE_CHANGED with scalar fields → updates started, activePlayerId, activePhase, secondsElapsed', () => {
    const state = makeState();
    const result = gamesReducer(state, Actions.gameStateChanged({
      gameId: 1,
      data: create(Data.Event_GameStateChangedSchema, {
        gameStarted: true,
        activePlayerId: 3,
        activePhase: 2,
        secondsElapsed: 60,
      }),
    }));

    expect(result.games[1].started).toBe(true);
    expect(result.games[1].activePlayerId).toBe(3);
    expect(result.games[1].activePhase).toBe(2);
    expect(result.games[1].secondsElapsed).toBe(60);
  });

  it('PLAYER_JOINED → adds new empty PlayerEntry keyed by playerId', () => {
    const state = makeState();
    const props = makePlayerProperties({ playerId: 5 });
    const result = gamesReducer(state, Actions.playerJoined({ gameId: 1, playerProperties: props }));
    const newPlayer = result.games[1].players[5];
    expect(newPlayer).toBeDefined();
    expect(newPlayer.properties).toBe(props);
    expect(newPlayer.zones).toEqual({});
    expect(newPlayer.counters).toEqual({});
    expect(newPlayer.arrows).toEqual({});
  });

  it('PLAYER_LEFT → removes player from game.players', () => {
    const state = makeState();
    const result = gamesReducer(state, Actions.playerLeft({ gameId: 1, playerId: 1 }));
    expect(result.games[1].players[1]).toBeUndefined();
  });

  it('PLAYER_PROPERTIES_CHANGED → replaces properties on existing player', () => {
    const state = makeState();
    const newProps = makePlayerProperties({ playerId: 1, conceded: true });
    const result = gamesReducer(state, Actions.playerPropertiesChanged({
      gameId: 1,
      playerId: 1,
      properties: newProps,
    }));
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
    const result = gamesReducer(state, Actions.cardMoved({
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
    }));

    expect(cardsIn(result, 1, 1, 'hand')).toHaveLength(0);
    expect(result.games[1].players[1].zones['hand'].cardCount).toBe(0);
    const movedCard = cardsIn(result, 1, 1, 'table')[0];
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

    const result = gamesReducer(state, Actions.cardMoved({
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
    }));

    expect(cardsIn(result, 1, 1, 'deck')).toHaveLength(0);
    expect(cardsIn(result, 1, 1, 'hand')[0].id).toBe(11);
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

    const result = gamesReducer(state, Actions.cardMoved({
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
    }));

    expect(result.games[1].players[1].zones['deck'].cardCount).toBe(4);
    const movedCard = cardsIn(result, 1, 1, 'hand')[0];
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

    const result = gamesReducer(state, Actions.cardMoved({
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
    }));

    expect(cardsIn(result, 1, 1, 'hand')).toHaveLength(0);
    expect(cardsIn(result, 1, 2, 'hand')[0].id).toBe(20);
  });

  it('assigns newCardId when newCardId ≥ 0', () => {
    const { state } = stateWithCard();
    const result = gamesReducer(state, Actions.cardMoved({
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
    }));

    expect(cardsIn(result, 1, 1, 'table')[0].id).toBe(999);
  });

  it('applies newCardProviderId and cardName to moved card', () => {
    const { state } = stateWithCard({ name: 'Old Name', providerId: 'old-prov' });
    const result = gamesReducer(state, Actions.cardMoved({
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
    }));

    const moved = cardsIn(result, 1, 1, 'table')[0];
    expect(moved.name).toBe('New Name');
    expect(moved.providerId).toBe('new-prov');
  });

  it('CARD_MOVED → no-ops when targetZone does not exist on player', () => {
    const { state } = stateWithCard();
    const result = gamesReducer(state, Actions.cardMoved({
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
    }));
    expect(cardsIn(result, 1, 1, 'hand')).toHaveLength(1);
    expect(result.games[1].players[1].zones['nonexistent']).toBeUndefined();
  });

  it('CARD_MOVED → no-ops when neither cardId nor position resolve and newCardId < 0', () => {
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

    const result = gamesReducer(state, Actions.cardMoved({
      gameId: 1,
      playerId: 1,
      data: {
        cardId: -1, cardName: '', startPlayerId: 1, startZone: 'deck',
        position: -1, targetPlayerId: 1, targetZone: 'hand',
        x: 0, y: 0, newCardId: -1, faceDown: false, newCardProviderId: '',
      },
    }));

    expect(result.games[1].players[1].zones['deck'].cardCount).toBe(5);
    expect(cardsIn(result, 1, 1, 'hand')).toHaveLength(0);
  });

  it('CARD_MOVED → deep-clones counterList so moved card is independent', () => {
    const cardCounter = create(Data.ServerInfo_CardCounterSchema, { id: 1, value: 3 });
    const card = makeCard({ id: 10, counterList: [cardCounter] });
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

    const result = gamesReducer(state, Actions.cardMoved({
      gameId: 1,
      playerId: 1,
      data: {
        cardId: 10, cardName: '', startPlayerId: 1, startZone: 'hand',
        position: -1, targetPlayerId: 1, targetZone: 'table',
        x: 0, y: 0, newCardId: -1, faceDown: false, newCardProviderId: '',
      },
    }));

    const movedCard = cardsIn(result, 1, 1, 'table')[0];
    expect(movedCard.counterList).toHaveLength(1);
    expect(movedCard.counterList).not.toBe(card.counterList);
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
    const result = gamesReducer(state, Actions.cardFlipped({
      gameId: 1,
      playerId: 1,
      data: { zoneName: 'hand', cardId: 5, cardName: 'Revealed', faceDown: true, cardProviderId: 'new-prov' },
    }));

    const card = cardsIn(result, 1, 1, 'hand')[0];
    expect(card.faceDown).toBe(true);
    expect(card.name).toBe('Revealed');
    expect(card.providerId).toBe('new-prov');
  });

  it('CARD_DESTROYED → removes card from zone and decrements cardCount', () => {
    const { state } = stateWithCardInZone('hand');
    const result = gamesReducer(state, Actions.cardDestroyed({
      gameId: 1,
      playerId: 1,
      data: { zoneName: 'hand', cardId: 5 },
    }));

    expect(cardsIn(result, 1, 1, 'hand')).toHaveLength(0);
    expect(result.games[1].players[1].zones['hand'].cardCount).toBe(0);
  });

  it('CARD_ATTACHED → sets attachPlayerId, attachZone, attachCardId on matched card', () => {
    const { state } = stateWithCardInZone('table');
    const result = gamesReducer(state, Actions.cardAttached({
      gameId: 1,
      playerId: 1,
      data: { startZone: 'table', cardId: 5, targetPlayerId: 2, targetZone: 'table', targetCardId: 99 },
    }));

    const card = cardsIn(result, 1, 1, 'table')[0];
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

    const result = gamesReducer(state, Actions.tokenCreated({
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
    }));

    const zone = result.games[1].players[1].zones['table'];
    expect(zone.cardCount).toBe(1);
    const tableCards = cardsIn(result, 1, 1, 'table');
    expect(tableCards[0].id).toBe(77);
    expect(tableCards[0].name).toBe('Goblin');
    expect(tableCards[0].destroyOnZoneChange).toBe(true);
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

  function dispatchAttr(state: ReturnType<typeof makeState>, attribute: Data.CardAttribute, attrValue: string) {
    return gamesReducer(state, Actions.cardAttrChanged({
      gameId: 1,
      playerId: 1,
      data: { zoneName: 'table', cardId: 3, attribute, attrValue },
    }));
  }

  it('AttrTapped (1) → card.tapped = true when attrValue is "1"', () => {
    const result = dispatchAttr(stateWithCard(), Data.CardAttribute.AttrTapped, '1');
    expect(cardsIn(result, 1, 1, 'table')[0].tapped).toBe(true);
  });

  it('AttrAttacking (2) → card.attacking = true when attrValue is "1"', () => {
    const result = dispatchAttr(stateWithCard(), Data.CardAttribute.AttrAttacking, '1');
    expect(cardsIn(result, 1, 1, 'table')[0].attacking).toBe(true);
  });

  it('AttrFaceDown (3) → card.faceDown = true when attrValue is "1"', () => {
    const result = dispatchAttr(stateWithCard(), Data.CardAttribute.AttrFaceDown, '1');
    expect(cardsIn(result, 1, 1, 'table')[0].faceDown).toBe(true);
  });

  it('AttrColor (4) → card.color = attrValue', () => {
    const result = dispatchAttr(stateWithCard(), Data.CardAttribute.AttrColor, 'red');
    expect(cardsIn(result, 1, 1, 'table')[0].color).toBe('red');
  });

  it('AttrPT (5) → card.pt = attrValue', () => {
    const result = dispatchAttr(stateWithCard(), Data.CardAttribute.AttrPT, '2/3');
    expect(cardsIn(result, 1, 1, 'table')[0].pt).toBe('2/3');
  });

  it('AttrAnnotation (6) → card.annotation = attrValue', () => {
    const result = dispatchAttr(stateWithCard(), Data.CardAttribute.AttrAnnotation, 'enchanted');
    expect(cardsIn(result, 1, 1, 'table')[0].annotation).toBe('enchanted');
  });

  it('AttrDoesntUntap (7) → card.doesntUntap = true when attrValue is "1"', () => {
    const result = dispatchAttr(stateWithCard(), Data.CardAttribute.AttrDoesntUntap, '1');
    expect(cardsIn(result, 1, 1, 'table')[0].doesntUntap).toBe(true);
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
    const result = gamesReducer(state, Actions.cardCounterChanged({
      gameId: 1,
      playerId: 1,
      data: { zoneName: 'table', cardId: 4, counterId: 1, counterValue: 3 },
    }));
    expect(cardsIn(result, 1, 1, 'table')[0].counterList).toEqual([expect.objectContaining({ id: 1, value: 3 })]);
  });

  it('updates existing counter value when counterId matches', () => {
    const state = stateWithCard([{ id: 1, value: 3 }]);
    const result = gamesReducer(state, Actions.cardCounterChanged({
      gameId: 1,
      playerId: 1,
      data: { zoneName: 'table', cardId: 4, counterId: 1, counterValue: 7 },
    }));
    expect(cardsIn(result, 1, 1, 'table')[0].counterList).toEqual([expect.objectContaining({ id: 1, value: 7 })]);
  });

  it('removes counter from counterList when counterValue ≤ 0', () => {
    const state = stateWithCard([{ id: 1, value: 3 }]);
    const result = gamesReducer(state, Actions.cardCounterChanged({
      gameId: 1,
      playerId: 1,
      data: { zoneName: 'table', cardId: 4, counterId: 1, counterValue: 0 },
    }));
    expect(cardsIn(result, 1, 1, 'table')[0].counterList).toEqual([]);
  });
});

// ── 2G: Arrows ────────────────────────────────────────────────────────────────

describe('2G: Arrows', () => {
  it('ARROW_CREATED → inserts arrowInfo into player.arrows keyed by id', () => {
    const state = makeState();
    const arrow = makeArrow({ id: 9 });
    const result = gamesReducer(state, Actions.arrowCreated({
      gameId: 1,
      playerId: 1,
      data: { arrowInfo: arrow },
    }));
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
    const result = gamesReducer(state, Actions.arrowDeleted({
      gameId: 1,
      playerId: 1,
      data: { arrowId: 9 },
    }));
    expect(result.games[1].players[1].arrows[9]).toBeUndefined();
  });
});

// ── 2H: Player counters ───────────────────────────────────────────────────────

describe('2H: Player counters', () => {
  it('COUNTER_CREATED → inserts counterInfo into player.counters keyed by id', () => {
    const state = makeState();
    const counter = makeCounter({ id: 5, name: 'Poison' });
    const result = gamesReducer(state, Actions.counterCreated({
      gameId: 1,
      playerId: 1,
      data: { counterInfo: counter },
    }));
    expect(result.games[1].players[1].counters[5]).toEqual(counter);
  });

  it('COUNTER_CREATED → clones counterInfo to prevent shared references', () => {
    const state = makeState();
    const counter = makeCounter({ id: 5, name: 'Life', count: 20 });
    const result = gamesReducer(state, Actions.counterCreated({
      gameId: 1,
      playerId: 1,
      data: { counterInfo: counter },
    }));
    expect(result.games[1].players[1].counters[5]).not.toBe(counter);
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
    const result = gamesReducer(state, Actions.counterSet({
      gameId: 1,
      playerId: 1,
      data: { counterId: 5, value: 14 },
    }));
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
    const result = gamesReducer(state, Actions.counterDeleted({
      gameId: 1,
      playerId: 1,
      data: { counterId: 5 },
    }));
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

    const result = gamesReducer(state, Actions.cardsDrawn({
      gameId: 1,
      playerId: 1,
      data: { number: 2, cards: [drawnCard] },
    }));

    expect(result.games[1].players[1].zones['deck'].cardCount).toBe(38);
    expect(cardsIn(result, 1, 1, 'hand')).toContainEqual(drawnCard);
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

    const result = gamesReducer(state, Actions.cardsDrawn({
      gameId: 1,
      playerId: 1,
      data: { number: 1, cards: [drawnCard] },
    }));

    expect(result.games[1].players[1].zones['hand'].cardCount).toBe(1);
    expect(cardsIn(result, 1, 1, 'hand')).toContainEqual(drawnCard);
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

    const result = gamesReducer(state, Actions.cardsRevealed({
      gameId: 1,
      playerId: 1,
      data: { zoneName: 'deck', cards: [{ ...existing, name: 'Revealed Name' }] },
    }));

    expect(cardsIn(result, 1, 1, 'deck')[0].name).toBe('Revealed Name');
    expect(cardsIn(result, 1, 1, 'deck')).toHaveLength(1);
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

    const result = gamesReducer(state, Actions.cardsRevealed({
      gameId: 1,
      playerId: 1,
      data: { zoneName: 'deck', cards: [newCard] },
    }));

    expect(cardsIn(result, 1, 1, 'deck')).toHaveLength(2);
    expect(cardsIn(result, 1, 1, 'deck')[1]).toEqual(newCard);
  });

  it('CARDS_REVEALED → clones counterList to prevent shared references', () => {
    const cardCounter = create(Data.ServerInfo_CardCounterSchema, { id: 1, value: 5 });
    const revealedCard = makeCard({ id: 3, counterList: [cardCounter] });
    const state = makeState({
      games: {
        1: makeGameEntry({
          players: {
            1: makePlayerEntry({
              zones: {
                deck: makeZoneEntry({ name: 'deck', cards: [], cardCount: 0 }),
              },
            }),
          },
        }),
      },
    });

    const result = gamesReducer(state, Actions.cardsRevealed({
      gameId: 1,
      playerId: 1,
      data: { zoneName: 'deck', cards: [revealedCard] },
    }));

    const stored = cardsIn(result, 1, 1, 'deck')[0];
    expect(stored.counterList).toEqual(revealedCard.counterList);
    expect(stored.counterList).not.toBe(revealedCard.counterList);
  });

  it('ZONE_PROPERTIES_CHANGED → sets alwaysRevealTopCard and alwaysLookAtTopCard', () => {
    const state = makeState();
    const result = gamesReducer(state, Actions.zonePropertiesChanged({
      gameId: 1,
      playerId: 1,
      data: create(Data.Event_ChangeZonePropertiesSchema, {
        zoneName: 'hand', alwaysRevealTopCard: true, alwaysLookAtTopCard: true,
      }),
    }));

    const zone = result.games[1].players[1].zones['hand'];
    expect(zone.alwaysRevealTopCard).toBe(true);
    expect(zone.alwaysLookAtTopCard).toBe(true);
  });
});

// ── 2J: Turn / phase / chat ───────────────────────────────────────────────────

describe('2J: Turn, phase, and chat', () => {
  it('ACTIVE_PLAYER_SET → sets game.activePlayerId', () => {
    const state = makeState();
    const result = gamesReducer(state, Actions.activePlayerSet({ gameId: 1, activePlayerId: 3 }));
    expect(result.games[1].activePlayerId).toBe(3);
  });

  it('ACTIVE_PHASE_SET → sets game.activePhase', () => {
    const state = makeState();
    const result = gamesReducer(state, Actions.activePhaseSet({ gameId: 1, phase: 5 }));
    expect(result.games[1].activePhase).toBe(5);
  });

  it('TURN_REVERSED → sets game.reversed', () => {
    const state = makeState();
    const result = gamesReducer(state, Actions.turnReversed({ gameId: 1, reversed: true }));
    expect(result.games[1].reversed).toBe(true);
  });

  it('GAME_SAY → appends message with timeReceived from payload', () => {
    const state = makeState();
    const result = gamesReducer(state, Actions.gameSay({
      gameId: 1,
      playerId: 2,
      message: 'gg',
      timeReceived: 123456789,
    }));

    expect(result.games[1].messages).toHaveLength(1);
    expect(result.games[1].messages[0]).toEqual({ playerId: 2, message: 'gg', timeReceived: 123456789 });
  });
});

// ── 2K: No-op / passthrough actions ──────────────────────────────────────────

describe('2K: No-op / passthrough actions', () => {
  it('ZONE_SHUFFLED → returns state unchanged (identity)', () => {
    const state = makeState();
    const result = gamesReducer(state, Actions.zoneShuffled({ gameId: 1, playerId: 1, data: {} }));
    expect(result).toBe(state);
  });

  it('ZONE_DUMPED → returns state unchanged (identity)', () => {
    const state = makeState();
    const result = gamesReducer(state, Actions.zoneDumped({ gameId: 1, playerId: 1, data: {} }));
    expect(result).toBe(state);
  });

  it('DIE_ROLLED → returns state unchanged (identity)', () => {
    const state = makeState();
    const result = gamesReducer(state, Actions.dieRolled({ gameId: 1, playerId: 1, data: {} }));
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
    expect(gamesReducer(state, Actions.gameHostChanged({ gameId: UNKNOWN_GAME, hostId: 1 }))).toBe(state);
  });

  it('GAME_STATE_CHANGED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.gameStateChanged({ gameId: UNKNOWN_GAME, data: {} }))).toBe(state);
  });

  it('PLAYER_JOINED with unknown gameId → state unchanged', () => {
    const state = makeState();
    const props = makePlayerProperties({ playerId: 5 });
    expect(gamesReducer(state, Actions.playerJoined({ gameId: UNKNOWN_GAME, playerProperties: props }))).toBe(state);
  });

  it('PLAYER_LEFT with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.playerLeft({ gameId: UNKNOWN_GAME, playerId: 1 }))).toBe(state);
  });

  it('updatePlayer guard: PLAYER_PROPERTIES_CHANGED with unknown gameId → state unchanged', () => {
    const state = makeState();
    const props = makePlayerProperties({ playerId: 1 });
    expect(gamesReducer(state, Actions.playerPropertiesChanged({
      gameId: UNKNOWN_GAME, playerId: 1, properties: props,
    }))).toBe(state);
  });

  it('updatePlayer guard: PLAYER_PROPERTIES_CHANGED with unknown playerId → state unchanged', () => {
    const state = makeState();
    const props = makePlayerProperties({ playerId: UNKNOWN_PLAYER });
    expect(gamesReducer(state, Actions.playerPropertiesChanged({
      gameId: 1, playerId: UNKNOWN_PLAYER, properties: props,
    }))).toBe(state);
  });

  it('CARD_MOVED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardMoved({
      gameId: UNKNOWN_GAME, playerId: 1,
      data: {
        cardId: 1, cardName: '', startPlayerId: 1, startZone: 'hand', position: -1,
        targetPlayerId: 1, targetZone: 'hand', x: 0, y: 0, newCardId: -1, faceDown: false, newCardProviderId: '',
      },
    }))).toBe(state);
  });

  it('CARD_MOVED with unknown sourcePlayer → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardMoved({
      gameId: 1, playerId: 1,
      data: {
        cardId: 1, cardName: '', startPlayerId: UNKNOWN_PLAYER, startZone: 'hand', position: -1,
        targetPlayerId: 1, targetZone: 'hand', x: 0, y: 0, newCardId: -1, faceDown: false, newCardProviderId: '',
      },
    }))).toBe(state);
  });

  it('CARD_MOVED with unknown sourceZone → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardMoved({
      gameId: 1, playerId: 1,
      data: {
        cardId: 1, cardName: '', startPlayerId: 1, startZone: 'nonexistent', position: -1,
        targetPlayerId: 1, targetZone: 'hand', x: 0, y: 0, newCardId: -1, faceDown: false, newCardProviderId: '',
      },
    }))).toBe(state);
  });

  it('CARD_FLIPPED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardFlipped({
      gameId: UNKNOWN_GAME, playerId: 1,
      data: { zoneName: 'hand', cardId: 1, cardName: '', faceDown: false, cardProviderId: '' },
    }))).toBe(state);
  });

  it('CARD_FLIPPED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardFlipped({
      gameId: 1, playerId: UNKNOWN_PLAYER,
      data: { zoneName: 'hand', cardId: 1, cardName: '', faceDown: false, cardProviderId: '' },
    }))).toBe(state);
  });

  it('CARD_FLIPPED with unknown zone → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardFlipped({
      gameId: 1, playerId: 1,
      data: { zoneName: 'nonexistent', cardId: 1, cardName: '', faceDown: false, cardProviderId: '' },
    }))).toBe(state);
  });

  it('CARD_FLIPPED with unknown cardId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardFlipped({
      gameId: 1, playerId: 1,
      data: { zoneName: 'hand', cardId: 9999, cardName: '', faceDown: false, cardProviderId: '' },
    }))).toBe(state);
  });

  it('CARD_DESTROYED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardDestroyed({
      gameId: UNKNOWN_GAME, playerId: 1,
      data: { zoneName: 'hand', cardId: 1 },
    }))).toBe(state);
  });

  it('CARD_DESTROYED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardDestroyed({
      gameId: 1, playerId: UNKNOWN_PLAYER,
      data: { zoneName: 'hand', cardId: 1 },
    }))).toBe(state);
  });

  it('CARD_DESTROYED with unknown zone → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardDestroyed({
      gameId: 1, playerId: 1,
      data: { zoneName: 'nonexistent', cardId: 1 },
    }))).toBe(state);
  });

  it('CARD_ATTACHED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardAttached({
      gameId: UNKNOWN_GAME, playerId: 1,
      data: { startZone: 'hand', cardId: 1, targetPlayerId: 1, targetZone: 'hand', targetCardId: 1 },
    }))).toBe(state);
  });

  it('CARD_ATTACHED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardAttached({
      gameId: 1, playerId: UNKNOWN_PLAYER,
      data: { startZone: 'hand', cardId: 1, targetPlayerId: 1, targetZone: 'hand', targetCardId: 1 },
    }))).toBe(state);
  });

  it('CARD_ATTACHED with unknown zone → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardAttached({
      gameId: 1, playerId: 1,
      data: { startZone: 'nonexistent', cardId: 1, targetPlayerId: 1, targetZone: 'hand', targetCardId: 1 },
    }))).toBe(state);
  });

  it('CARD_ATTACHED with unknown cardId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardAttached({
      gameId: 1, playerId: 1,
      data: { startZone: 'hand', cardId: 9999, targetPlayerId: 1, targetZone: 'hand', targetCardId: 1 },
    }))).toBe(state);
  });

  it('TOKEN_CREATED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.tokenCreated({
      gameId: UNKNOWN_GAME, playerId: 1,
      data: {
        zoneName: 'hand', cardId: 1, cardName: 'T', color: '', pt: '', annotation: '',
        destroyOnZoneChange: false, x: 0, y: 0, cardProviderId: '', faceDown: false,
      },
    }))).toBe(state);
  });

  it('TOKEN_CREATED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.tokenCreated({
      gameId: 1, playerId: UNKNOWN_PLAYER,
      data: {
        zoneName: 'hand', cardId: 1, cardName: 'T', color: '', pt: '', annotation: '',
        destroyOnZoneChange: false, x: 0, y: 0, cardProviderId: '', faceDown: false,
      },
    }))).toBe(state);
  });

  it('TOKEN_CREATED with unknown zone → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.tokenCreated({
      gameId: 1, playerId: 1,
      data: {
        zoneName: 'nonexistent', cardId: 1, cardName: 'T', color: '', pt: '', annotation: '',
        destroyOnZoneChange: false, x: 0, y: 0, cardProviderId: '', faceDown: false,
      },
    }))).toBe(state);
  });

  it('CARD_ATTR_CHANGED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardAttrChanged({
      gameId: UNKNOWN_GAME, playerId: 1,
      data: { zoneName: 'hand', cardId: 1, attribute: 1, attrValue: '1' },
    }))).toBe(state);
  });

  it('CARD_ATTR_CHANGED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardAttrChanged({
      gameId: 1, playerId: UNKNOWN_PLAYER,
      data: { zoneName: 'hand', cardId: 1, attribute: 1, attrValue: '1' },
    }))).toBe(state);
  });

  it('CARD_ATTR_CHANGED with unknown zone → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardAttrChanged({
      gameId: 1, playerId: 1,
      data: { zoneName: 'nonexistent', cardId: 1, attribute: 1, attrValue: '1' },
    }))).toBe(state);
  });

  it('CARD_ATTR_CHANGED with unknown cardId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardAttrChanged({
      gameId: 1, playerId: 1,
      data: { zoneName: 'hand', cardId: 9999, attribute: 1, attrValue: '1' },
    }))).toBe(state);
  });

  it('CARD_COUNTER_CHANGED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardCounterChanged({
      gameId: UNKNOWN_GAME, playerId: 1,
      data: { zoneName: 'hand', cardId: 1, counterId: 1, counterValue: 1 },
    }))).toBe(state);
  });

  it('CARD_COUNTER_CHANGED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardCounterChanged({
      gameId: 1, playerId: UNKNOWN_PLAYER,
      data: { zoneName: 'hand', cardId: 1, counterId: 1, counterValue: 1 },
    }))).toBe(state);
  });

  it('CARD_COUNTER_CHANGED with unknown zone → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardCounterChanged({
      gameId: 1, playerId: 1,
      data: { zoneName: 'nonexistent', cardId: 1, counterId: 1, counterValue: 1 },
    }))).toBe(state);
  });

  it('CARD_COUNTER_CHANGED with unknown cardId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardCounterChanged({
      gameId: 1, playerId: 1,
      data: { zoneName: 'hand', cardId: 9999, counterId: 1, counterValue: 1 },
    }))).toBe(state);
  });

  it('ARROW_CREATED with unknown gameId → state unchanged', () => {
    const state = makeState();
    const arrow = makeArrow({ id: 1 });
    expect(gamesReducer(state, Actions.arrowCreated({ gameId: UNKNOWN_GAME, playerId: 1, data: { arrowInfo: arrow } }))).toBe(state);
  });

  it('ARROW_CREATED with unknown playerId → state unchanged', () => {
    const state = makeState();
    const arrow = makeArrow({ id: 1 });
    expect(gamesReducer(state, Actions.arrowCreated({ gameId: 1, playerId: UNKNOWN_PLAYER, data: { arrowInfo: arrow } }))).toBe(state);
  });

  it('ARROW_DELETED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.arrowDeleted({ gameId: UNKNOWN_GAME, playerId: 1, data: { arrowId: 1 } }))).toBe(state);
  });

  it('ARROW_DELETED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.arrowDeleted({ gameId: 1, playerId: UNKNOWN_PLAYER, data: { arrowId: 1 } }))).toBe(state);
  });

  it('COUNTER_CREATED with unknown gameId → state unchanged', () => {
    const state = makeState();
    const counter = makeCounter({ id: 1 });
    expect(gamesReducer(state, Actions.counterCreated({
      gameId: UNKNOWN_GAME, playerId: 1, data: { counterInfo: counter },
    }))).toBe(state);
  });

  it('COUNTER_CREATED with unknown playerId → state unchanged', () => {
    const state = makeState();
    const counter = makeCounter({ id: 1 });
    expect(gamesReducer(state, Actions.counterCreated({
      gameId: 1, playerId: UNKNOWN_PLAYER, data: { counterInfo: counter },
    }))).toBe(state);
  });

  it('COUNTER_SET with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.counterSet({
      gameId: UNKNOWN_GAME, playerId: 1, data: { counterId: 1, value: 5 },
    }))).toBe(state);
  });

  it('COUNTER_SET with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.counterSet({
      gameId: 1, playerId: UNKNOWN_PLAYER, data: { counterId: 1, value: 5 },
    }))).toBe(state);
  });

  it('COUNTER_SET with unknown counterId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.counterSet({ gameId: 1, playerId: 1, data: { counterId: 9999, value: 5 } }))).toBe(state);
  });

  it('COUNTER_DELETED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.counterDeleted({ gameId: UNKNOWN_GAME, playerId: 1, data: { counterId: 1 } }))).toBe(state);
  });

  it('COUNTER_DELETED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.counterDeleted({ gameId: 1, playerId: UNKNOWN_PLAYER, data: { counterId: 1 } }))).toBe(state);
  });

  it('CARDS_DRAWN with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardsDrawn({
      gameId: UNKNOWN_GAME, playerId: 1, data: { number: 1, cards: [] },
    }))).toBe(state);
  });

  it('CARDS_DRAWN with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardsDrawn({
      gameId: 1, playerId: UNKNOWN_PLAYER, data: { number: 1, cards: [] }
    }))).toBe(state);
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
    expect(gamesReducer(state, Actions.cardsDrawn({ gameId: 1, playerId: 1, data: { number: 1, cards: [] } }))).toBe(state);
  });

  it('CARDS_REVEALED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardsRevealed({
      gameId: UNKNOWN_GAME, playerId: 1, data: { zoneName: 'hand', cards: [] },
    }))).toBe(state);
  });

  it('CARDS_REVEALED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardsRevealed({
      gameId: 1, playerId: UNKNOWN_PLAYER, data: { zoneName: 'hand', cards: [] },
    }))).toBe(state);
  });

  it('CARDS_REVEALED with unknown zone → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.cardsRevealed({
      gameId: 1, playerId: 1, data: { zoneName: 'nonexistent', cards: [] },
    }))).toBe(state);
  });

  it('updateZone guard: ZONE_PROPERTIES_CHANGED with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.zonePropertiesChanged({
      gameId: UNKNOWN_GAME, playerId: 1,
      data: { zoneName: 'hand', alwaysRevealTopCard: true, alwaysLookAtTopCard: true },
    }))).toBe(state);
  });

  it('updateZone guard: ZONE_PROPERTIES_CHANGED with unknown playerId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.zonePropertiesChanged({
      gameId: 1, playerId: UNKNOWN_PLAYER,
      data: { zoneName: 'hand', alwaysRevealTopCard: true, alwaysLookAtTopCard: true },
    }))).toBe(state);
  });

  it('updateZone guard: ZONE_PROPERTIES_CHANGED with unknown zone → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.zonePropertiesChanged({
      gameId: 1, playerId: 1,
      data: { zoneName: 'nonexistent', alwaysRevealTopCard: true, alwaysLookAtTopCard: true },
    }))).toBe(state);
  });

  it('GAME_SAY with unknown gameId → state unchanged', () => {
    const state = makeState();
    expect(gamesReducer(state, Actions.gameSay({ gameId: UNKNOWN_GAME, playerId: 1, message: 'hi', timeReceived: 0 }))).toBe(state);
  });
});
