import { Selectors } from './game.selectors';
import { makeGameEntry, makePlayerEntry, makeState,
  makeZoneEntry, makeCard, makeCounter, makeArrow,
} from './__mocks__/fixtures';
import { GamesState } from './game.interfaces';

function rootState(games: GamesState) {
  return { games };
}

describe('Selectors', () => {
  it('getGames → returns the games map', () => {
    const state = makeState();
    expect(Selectors.getGames(rootState(state))).toBe(state.games);
  });

  it('getGame → returns the game entry for a given gameId', () => {
    const state = makeState();
    expect(Selectors.getGame(rootState(state), 1)).toBe(state.games[1]);
  });

  it('getGame → returns undefined for unknown gameId', () => {
    const state = makeState();
    expect(Selectors.getGame(rootState(state), 999)).toBeUndefined();
  });

  it('getPlayers → returns players map for a game', () => {
    const state = makeState();
    expect(Selectors.getPlayers(rootState(state), 1)).toBe(state.games[1].players);
  });

  it('getPlayers → returns undefined for unknown gameId', () => {
    const state = makeState();
    expect(Selectors.getPlayers(rootState(state), 999)).toBeUndefined();
  });

  it('getPlayer → returns a specific player', () => {
    const state = makeState();
    expect(Selectors.getPlayer(rootState(state), 1, 1)).toBe(state.games[1].players[1]);
  });

  it('getLocalPlayerId → returns localPlayerId from game', () => {
    const state = makeState({ games: { 1: makeGameEntry({ localPlayerId: 42 }) } });
    expect(Selectors.getLocalPlayerId(rootState(state), 1)).toBe(42);
  });

  it('getLocalPlayer → returns the player matching localPlayerId', () => {
    const state = makeState({ games: { 1: makeGameEntry({ localPlayerId: 1 }) } });
    const result = Selectors.getLocalPlayer(rootState(state), 1);
    expect(result).toBe(state.games[1].players[1]);
  });

  it('getLocalPlayer → returns undefined when game is not found', () => {
    const state = makeState();
    expect(Selectors.getLocalPlayer(rootState(state), 999)).toBeUndefined();
  });

  it('getZones → returns zones map for a player', () => {
    const state = makeState();
    expect(Selectors.getZones(rootState(state), 1, 1)).toBe(state.games[1].players[1].zones);
  });

  it('getZone → returns a specific zone', () => {
    const state = makeState();
    expect(Selectors.getZone(rootState(state), 1, 1, 'hand')).toBe(state.games[1].players[1].zones['hand']);
  });

  it('getCards → returns cards array for a zone', () => {
    const card = makeCard();
    const state = makeState({
      games: {
        1: makeGameEntry({
          players: {
            1: makePlayerEntry({
              zones: { hand: makeZoneEntry({ name: 'hand', cards: [card] }) },
            }),
          },
        }),
      },
    });
    expect(Selectors.getCards(rootState(state), 1, 1, 'hand')).toEqual([card]);
  });

  it('getCards → returns [] when zone not found', () => {
    const state = makeState();
    expect(Selectors.getCards(rootState(state), 1, 1, 'nonexistent')).toEqual([]);
  });

  it('getCounters → returns counters map for a player', () => {
    const counter = makeCounter({ id: 2 });
    const state = makeState({
      games: { 1: makeGameEntry({ players: { 1: makePlayerEntry({ counters: { 2: counter } }) } }) },
    });
    expect(Selectors.getCounters(rootState(state), 1, 1)).toEqual({ 2: counter });
  });

  it('getArrows → returns arrows map for a player', () => {
    const arrow = makeArrow({ id: 3 });
    const state = makeState({
      games: { 1: makeGameEntry({ players: { 1: makePlayerEntry({ arrows: { 3: arrow } }) } }) },
    });
    expect(Selectors.getArrows(rootState(state), 1, 1)).toEqual({ 3: arrow });
  });

  it('getActivePlayerId → returns activePlayerId from game', () => {
    const state = makeState({ games: { 1: makeGameEntry({ activePlayerId: 7 }) } });
    expect(Selectors.getActivePlayerId(rootState(state), 1)).toBe(7);
  });

  it('getActivePhase → returns activePhase from game', () => {
    const state = makeState({ games: { 1: makeGameEntry({ activePhase: 3 }) } });
    expect(Selectors.getActivePhase(rootState(state), 1)).toBe(3);
  });

  it('isStarted → returns true when game is started', () => {
    const state = makeState({ games: { 1: makeGameEntry({ started: true }) } });
    expect(Selectors.isStarted(rootState(state), 1)).toBe(true);
  });

  it('isStarted → returns false when game not found', () => {
    const state = makeState();
    expect(Selectors.isStarted(rootState(state), 999)).toBe(false);
  });

  it('isSpectator → returns spectator flag from game', () => {
    const state = makeState({ games: { 1: makeGameEntry({ spectator: true }) } });
    expect(Selectors.isSpectator(rootState(state), 1)).toBe(true);
  });

  it('isReversed → returns reversed flag from game', () => {
    const state = makeState({ games: { 1: makeGameEntry({ reversed: true }) } });
    expect(Selectors.isReversed(rootState(state), 1)).toBe(true);
  });

  it('getMessages → returns messages array from game', () => {
    const messages = [{ playerId: 1, message: 'hi', timeReceived: 100 }];
    const state = makeState({ games: { 1: makeGameEntry({ messages }) } });
    expect(Selectors.getMessages(rootState(state), 1)).toBe(messages);
  });

  it('getMessages → returns [] when game not found', () => {
    const state = makeState();
    expect(Selectors.getMessages(rootState(state), 999)).toEqual([]);
  });

  it('getActiveGameIds → returns numeric array of gameIds', () => {
    const state = makeState({
      games: {
        1: makeGameEntry(),
        2: makeGameEntry(),
      },
    });
    const ids = Selectors.getActiveGameIds(rootState(state));
    expect(ids).toEqual(expect.arrayContaining([1, 2]));
    expect(ids).toHaveLength(2);
  });
});
