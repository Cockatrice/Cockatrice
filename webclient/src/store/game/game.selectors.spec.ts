import { App } from '@app/types';

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

  it('getHostId → returns hostId from game', () => {
    const state = makeState({ games: { 1: makeGameEntry({ hostId: 7 }) } });
    expect(Selectors.getHostId(rootState(state), 1)).toBe(7);
  });

  it('getHostId → returns undefined for unknown gameId', () => {
    const state = makeState();
    expect(Selectors.getHostId(rootState(state), 999)).toBeUndefined();
  });

  it('getSecondsElapsed → returns secondsElapsed from game', () => {
    const state = makeState({ games: { 1: makeGameEntry({ secondsElapsed: 314 }) } });
    expect(Selectors.getSecondsElapsed(rootState(state), 1)).toBe(314);
  });

  it('getJudge → returns judge flag from game', () => {
    const state = makeState({ games: { 1: makeGameEntry({ judge: true }) } });
    expect(Selectors.getJudge(rootState(state), 1)).toBe(true);
  });

  it('getJudge → returns false when game not found', () => {
    const state = makeState();
    expect(Selectors.getJudge(rootState(state), 999)).toBe(false);
  });

  it('getResuming → returns resuming flag from game', () => {
    const state = makeState({ games: { 1: makeGameEntry({ resuming: true }) } });
    expect(Selectors.getResuming(rootState(state), 1)).toBe(true);
  });

  it('getResuming → returns false when game not found', () => {
    const state = makeState();
    expect(Selectors.getResuming(rootState(state), 999)).toBe(false);
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

  describe('getAttachmentsByParent', () => {
    function stateWithTable(cards: ReturnType<typeof makeCard>[]): GamesState {
      return makeState({
        games: {
          1: makeGameEntry({
            players: {
              1: makePlayerEntry({
                zones: {
                  [App.ZoneName.TABLE]: makeZoneEntry({
                    name: App.ZoneName.TABLE,
                    withCoords: true,
                    cardCount: cards.length,
                    cards,
                  }),
                },
              }),
            },
          }),
        },
      });
    }

    it('returns an empty map when the TABLE zone is absent', () => {
      const state = makeState();
      const result = Selectors.getAttachmentsByParent(rootState(state), 1, 1);
      expect(result.size).toBe(0);
    });

    it('returns an empty map when no cards are attached', () => {
      const cards = [
        makeCard({ id: 1, name: 'Creature A' }),
        makeCard({ id: 2, name: 'Creature B' }),
      ];
      const state = stateWithTable(cards);
      const result = Selectors.getAttachmentsByParent(rootState(state), 1, 1);
      expect(result.size).toBe(0);
    });

    it('buckets a single attached child under its parent id', () => {
      const cards = [
        makeCard({ id: 10, name: 'Creature' }),
        makeCard({
          id: 11, name: 'Aura',
          attachPlayerId: 1, attachZone: App.ZoneName.TABLE, attachCardId: 10,
        }),
      ];
      const state = stateWithTable(cards);
      const result = Selectors.getAttachmentsByParent(rootState(state), 1, 1);
      expect(result.size).toBe(1);
      expect(result.get(10)?.map((c) => c.name)).toEqual(['Aura']);
    });

    it('buckets multiple children under the same parent, sorted by id', () => {
      const cards = [
        makeCard({ id: 5, name: 'Creature' }),
        makeCard({
          id: 30, name: 'Aura C',
          attachPlayerId: 1, attachZone: App.ZoneName.TABLE, attachCardId: 5,
        }),
        makeCard({
          id: 10, name: 'Aura A',
          attachPlayerId: 1, attachZone: App.ZoneName.TABLE, attachCardId: 5,
        }),
        makeCard({
          id: 20, name: 'Aura B',
          attachPlayerId: 1, attachZone: App.ZoneName.TABLE, attachCardId: 5,
        }),
      ];
      const state = stateWithTable(cards);
      const result = Selectors.getAttachmentsByParent(rootState(state), 1, 1);
      expect(result.get(5)?.map((c) => c.name)).toEqual(['Aura A', 'Aura B', 'Aura C']);
    });

    it('ignores attachments pointing to a different player', () => {
      const cards = [
        makeCard({ id: 1, name: 'Creature' }),
        makeCard({
          id: 2, name: 'Cross-player ref',
          attachPlayerId: 99, attachZone: App.ZoneName.TABLE, attachCardId: 1,
        }),
      ];
      const state = stateWithTable(cards);
      const result = Selectors.getAttachmentsByParent(rootState(state), 1, 1);
      expect(result.size).toBe(0);
    });

    it('ignores attachments pointing to a non-TABLE zone', () => {
      const cards = [
        makeCard({ id: 1, name: 'Creature' }),
        makeCard({
          id: 2, name: 'Non-table ref',
          attachPlayerId: 1, attachZone: App.ZoneName.HAND, attachCardId: 1,
        }),
      ];
      const state = stateWithTable(cards);
      const result = Selectors.getAttachmentsByParent(rootState(state), 1, 1);
      expect(result.size).toBe(0);
    });

    it('returns a stable Map reference for the same zone object', () => {
      const cards = [
        makeCard({ id: 1, name: 'Creature' }),
        makeCard({
          id: 2, name: 'Aura',
          attachPlayerId: 1, attachZone: App.ZoneName.TABLE, attachCardId: 1,
        }),
      ];
      const state = stateWithTable(cards);
      const a = Selectors.getAttachmentsByParent(rootState(state), 1, 1);
      const b = Selectors.getAttachmentsByParent(rootState(state), 1, 1);
      expect(a).toBe(b);
    });
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

  it('getActiveGames → returns the full GameEntry array', () => {
    const e1 = makeGameEntry();
    const e2 = makeGameEntry();
    const state = makeState({ games: { 1: e1, 2: e2 } });
    const games = Selectors.getActiveGames(rootState(state));
    expect(games).toHaveLength(2);
    expect(games).toEqual(expect.arrayContaining([e1, e2]));
  });

  it('getActiveGames → returns empty array when no games are active', () => {
    const state = makeState({ games: {} });
    expect(Selectors.getActiveGames(rootState(state))).toHaveLength(0);
  });
});
