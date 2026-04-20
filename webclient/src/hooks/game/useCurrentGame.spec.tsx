import { ReactNode } from 'react';
import { renderHook } from '@testing-library/react';
import { configureStore } from '@reduxjs/toolkit';
import { Provider } from 'react-redux';

// Deep imports are intentional here: see the comment in
// useGameAccess.spec.tsx. `@app/store` would spin up the singleton store
// and race with makeWrapper's test-local store.
import { gamesReducer } from '../../store/game/game.reducer';
import { makeGameEntry, makePlayerEntry, makePlayerProperties } from '../../store/game/__mocks__/fixtures';
import type { GamesState } from '../../store/game/game.interfaces';
import { useCurrentGame } from './useCurrentGame';

function makeWrapper(gamesState: GamesState) {
  const store = configureStore({
    reducer: { games: gamesReducer },
    preloadedState: { games: gamesState } as { games: GamesState },
  });
  return function Wrapper({ children }: { children: ReactNode }) {
    return <Provider store={store}>{children}</Provider>;
  };
}

describe('useCurrentGame', () => {
  it('returns an empty result when no games are active', () => {
    const { result } = renderHook(() => useCurrentGame(), {
      wrapper: makeWrapper({ games: {} }),
    });
    expect(result.current.gameId).toBeUndefined();
    expect(result.current.game).toBeUndefined();
    expect(result.current.isSpectator).toBe(false);
  });

  it('defaults to the first active game when gameId is omitted', () => {
    const game = makeGameEntry({
      localPlayerId: 7,
      hostId: 7,
      started: true,
      players: { 7: makePlayerEntry({ properties: makePlayerProperties({ playerId: 7 }) }) },
    });
    const wrapper = makeWrapper({ games: { 42: { ...game, info: { ...game.info, gameId: 42 } } } });

    const { result } = renderHook(() => useCurrentGame(), { wrapper });

    expect(result.current.gameId).toBe(42);
    expect(result.current.game).toBeDefined();
    expect(result.current.localPlayer).toBeDefined();
    expect(result.current.isStarted).toBe(true);
    expect(result.current.isHost).toBe(true);
    expect(result.current.isSpectator).toBe(false);
  });

  it('honors an explicit gameId over the default', () => {
    const g1 = makeGameEntry({ started: false });
    const g2 = makeGameEntry({ started: true });
    const wrapper = makeWrapper({ games: { 1: g1, 2: g2 } });

    const { result } = renderHook(() => useCurrentGame(2), { wrapper });

    expect(result.current.gameId).toBe(2);
    expect(result.current.isStarted).toBe(true);
  });

  it('reports spectator and judge flags from game entry', () => {
    const game = makeGameEntry({ spectator: true, judge: true, hostId: 99 });
    const wrapper = makeWrapper({ games: { 1: game } });

    const { result } = renderHook(() => useCurrentGame(), { wrapper });

    expect(result.current.isSpectator).toBe(true);
    expect(result.current.isJudge).toBe(true);
    expect(result.current.isHost).toBe(false);
  });

  it('returns empty shape when the requested gameId is missing', () => {
    const game = makeGameEntry();
    const wrapper = makeWrapper({ games: { 1: game } });

    const { result } = renderHook(() => useCurrentGame(999), { wrapper });

    expect(result.current.gameId).toBe(999);
    expect(result.current.game).toBeUndefined();
    expect(result.current.localPlayer).toBeUndefined();
  });
});
