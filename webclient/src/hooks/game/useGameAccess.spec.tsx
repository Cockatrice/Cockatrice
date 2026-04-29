import { ReactNode } from 'react';
import { renderHook } from '@testing-library/react';
import { configureStore } from '@reduxjs/toolkit';
import { Provider } from 'react-redux';

// Deep imports are intentional here: pulling from `@app/store` eagerly
// evaluates `store/index.ts` → `configureStore(rootReducer)`, instantiating
// the real singleton store that would race with the test-local store
// created in makeWrapper. Importing the reducer + type directly side-steps
// that and keeps this hook spec self-contained.
import { gamesReducer } from '../../store/game/game.reducer';
import { makeGameEntry } from '../../store/game/__mocks__/fixtures';
import type { GamesState } from '../../store/game/game.interfaces';
import { useGameAccess } from './useGameAccess';

// `preloadedState` on configureStore is typed as `PreloadedState<S>` which
// narrows collection fields past GamesState; a single precise cast keeps
// the spec strict while avoiding the broader `as any`.
function makeWrapper(gamesState: GamesState) {
  const store = configureStore({
    reducer: { games: gamesReducer },
    preloadedState: { games: gamesState } as { games: GamesState },
  });
  return function Wrapper({ children }: { children: ReactNode }) {
    return <Provider store={store}>{children}</Provider>;
  };
}

describe('useGameAccess', () => {
  it('returns fully locked out when the game is missing', () => {
    const wrapper = makeWrapper({ games: {} });
    const { result } = renderHook(() => useGameAccess(1), { wrapper });
    expect(result.current).toEqual({ canAct: false, canView: false, isLocalPlayer: false });
  });

  it('grants full action rights when no target is given and viewer is a local (non-spectator) player', () => {
    const game = makeGameEntry({ localPlayerId: 5, spectator: false, judge: false });
    const wrapper = makeWrapper({ games: { 1: game } });

    const { result } = renderHook(() => useGameAccess(1), { wrapper });

    expect(result.current.canAct).toBe(true);
    expect(result.current.canView).toBe(true);
    expect(result.current.isLocalPlayer).toBe(false);
  });

  it('allows actions against the local player target', () => {
    const game = makeGameEntry({ localPlayerId: 5, spectator: false });
    const wrapper = makeWrapper({ games: { 1: game } });

    const { result } = renderHook(() => useGameAccess(1, 5), { wrapper });

    expect(result.current.canAct).toBe(true);
    expect(result.current.isLocalPlayer).toBe(true);
  });

  it('denies actions against another player target for a normal user', () => {
    const game = makeGameEntry({ localPlayerId: 5, spectator: false, judge: false });
    const wrapper = makeWrapper({ games: { 1: game } });

    const { result } = renderHook(() => useGameAccess(1, 9), { wrapper });

    expect(result.current.canAct).toBe(false);
    expect(result.current.isLocalPlayer).toBe(false);
    expect(result.current.canView).toBe(true);
  });

  it('locks out all actions in spectator mode (even self-targeted)', () => {
    const game = makeGameEntry({ localPlayerId: 5, spectator: true, judge: false });
    const wrapper = makeWrapper({ games: { 1: game } });

    const { result } = renderHook(() => useGameAccess(1, 5), { wrapper });

    expect(result.current.canAct).toBe(false);
    expect(result.current.canView).toBe(true);
  });

  it('allows judges to act on any player', () => {
    const game = makeGameEntry({ localPlayerId: 5, spectator: false, judge: true });
    const wrapper = makeWrapper({ games: { 1: game } });

    const { result } = renderHook(() => useGameAccess(1, 9), { wrapper });

    expect(result.current.canAct).toBe(true);
    expect(result.current.isLocalPlayer).toBe(false);
  });

  // Desktop parity: judge status wins over the spectator flag. Servatrice
  // sends judges with `spectator: true` on the wire; the hook must still
  // let them act.
  it('allows judges to act even when flagged spectator', () => {
    const game = makeGameEntry({ localPlayerId: 5, spectator: true, judge: true });
    const wrapper = makeWrapper({ games: { 1: game } });

    const { result } = renderHook(() => useGameAccess(1, 9), { wrapper });

    expect(result.current.canAct).toBe(true);
    expect(result.current.canView).toBe(true);
  });
});
