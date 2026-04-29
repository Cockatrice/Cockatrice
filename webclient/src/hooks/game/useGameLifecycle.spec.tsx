import { ReactNode } from 'react';
import { act, renderHook } from '@testing-library/react';
import { combineReducers, configureStore } from '@reduxjs/toolkit';
import { Provider } from 'react-redux';

import { actionReducer } from '../../store/actions/actionReducer';
import { gamesReducer } from '../../store/game/game.reducer';
import { Types as GameTypes } from '../../store/game/game.types';

import { useGameLifecycle } from './useGameLifecycle';

function makeStore() {
  return configureStore({
    reducer: combineReducers({ games: gamesReducer, action: actionReducer }),
  });
}

function wrap(store: ReturnType<typeof makeStore>) {
  return function Wrapper({ children }: { children: ReactNode }) {
    return <Provider store={store}>{children}</Provider>;
  };
}

describe('useGameLifecycle', () => {
  it('invokes onKicked when a kicked action targets this gameId', () => {
    const store = makeStore();
    const onKicked = vi.fn();
    const onGameClosed = vi.fn();

    renderHook(
      () => useGameLifecycle(42, { onKicked, onGameClosed }),
      { wrapper: wrap(store) },
    );

    act(() => {
      store.dispatch({ type: GameTypes.KICKED, payload: { gameId: 42 } });
    });

    expect(onKicked).toHaveBeenCalledTimes(1);
    expect(onGameClosed).not.toHaveBeenCalled();
  });

  it('invokes onGameClosed when a gameClosed action targets this gameId', () => {
    const store = makeStore();
    const onKicked = vi.fn();
    const onGameClosed = vi.fn();

    renderHook(
      () => useGameLifecycle(42, { onKicked, onGameClosed }),
      { wrapper: wrap(store) },
    );

    act(() => {
      store.dispatch({ type: GameTypes.GAME_CLOSED, payload: { gameId: 42 } });
    });

    expect(onGameClosed).toHaveBeenCalledTimes(1);
    expect(onKicked).not.toHaveBeenCalled();
  });

  it('ignores kicked/closed actions for a different gameId', () => {
    const store = makeStore();
    const onKicked = vi.fn();
    const onGameClosed = vi.fn();

    renderHook(
      () => useGameLifecycle(42, { onKicked, onGameClosed }),
      { wrapper: wrap(store) },
    );

    act(() => {
      store.dispatch({ type: GameTypes.KICKED, payload: { gameId: 7 } });
      store.dispatch({ type: GameTypes.GAME_CLOSED, payload: { gameId: 7 } });
    });

    expect(onKicked).not.toHaveBeenCalled();
    expect(onGameClosed).not.toHaveBeenCalled();
  });

  it('is inert when gameId is undefined', () => {
    const store = makeStore();
    const onKicked = vi.fn();
    const onGameClosed = vi.fn();

    renderHook(
      () => useGameLifecycle(undefined, { onKicked, onGameClosed }),
      { wrapper: wrap(store) },
    );

    act(() => {
      store.dispatch({ type: GameTypes.KICKED, payload: { gameId: 42 } });
      store.dispatch({ type: GameTypes.GAME_CLOSED, payload: { gameId: 42 } });
    });

    expect(onKicked).not.toHaveBeenCalled();
    expect(onGameClosed).not.toHaveBeenCalled();
  });

  it('ignores unrelated action types', () => {
    const store = makeStore();
    const onKicked = vi.fn();
    const onGameClosed = vi.fn();

    renderHook(
      () => useGameLifecycle(42, { onKicked, onGameClosed }),
      { wrapper: wrap(store) },
    );

    act(() => {
      store.dispatch({ type: GameTypes.GAME_HOST_CHANGED, payload: { gameId: 42, hostId: 7 } });
    });

    expect(onKicked).not.toHaveBeenCalled();
    expect(onGameClosed).not.toHaveBeenCalled();
  });
});
