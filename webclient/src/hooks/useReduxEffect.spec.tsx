import { renderHook, act } from '@testing-library/react';
import { configureStore, combineReducers } from '@reduxjs/toolkit';
import { Provider } from 'react-redux';
import { StrictMode, ReactNode } from 'react';
import { useReduxEffect } from './useReduxEffect';
import { actionReducer } from '../store/actions/actionReducer';

function makeStore() {
  return configureStore({
    reducer: combineReducers({ action: actionReducer }),
  });
}

function makeWrapper(store: ReturnType<typeof makeStore>) {
  return function Wrapper({ children }: { children: ReactNode }) {
    return <Provider store={store}>{children}</Provider>;
  };
}

describe('useReduxEffect', () => {
  test('fires callback when matching action type is dispatched', () => {
    const store = makeStore();
    const effect = vi.fn();

    renderHook(() => useReduxEffect(effect, 'TEST_ACTION'), {
      wrapper: makeWrapper(store),
    });

    act(() => {
      store.dispatch({ type: 'TEST_ACTION', payload: 'hello' });
    });

    expect(effect).toHaveBeenCalledTimes(1);
    expect(effect).toHaveBeenCalledWith(
      expect.objectContaining({ type: 'TEST_ACTION' }),
    );
  });

  test('does not fire for non-matching action types', () => {
    const store = makeStore();
    const effect = vi.fn();

    renderHook(() => useReduxEffect(effect, 'LISTEN_FOR'), {
      wrapper: makeWrapper(store),
    });

    act(() => {
      store.dispatch({ type: 'OTHER_ACTION' });
    });

    expect(effect).not.toHaveBeenCalled();
  });

  test('handles array of action types', () => {
    const store = makeStore();
    const effect = vi.fn();

    renderHook(() => useReduxEffect(effect, ['TYPE_A', 'TYPE_B']), {
      wrapper: makeWrapper(store),
    });

    act(() => {
      store.dispatch({ type: 'TYPE_A' });
    });
    act(() => {
      store.dispatch({ type: 'TYPE_B' });
    });
    act(() => {
      store.dispatch({ type: 'TYPE_C' });
    });

    expect(effect).toHaveBeenCalledTimes(2);
  });

  test('does not double-fire in StrictMode', () => {
    const store = makeStore();
    const effect = vi.fn();

    function StrictWrapper({ children }: { children: ReactNode }) {
      return (
        <StrictMode>
          <Provider store={store}>{children}</Provider>
        </StrictMode>
      );
    }

    renderHook(() => useReduxEffect(effect, 'TEST'), {
      wrapper: StrictWrapper,
    });

    act(() => {
      store.dispatch({ type: 'TEST' });
    });

    expect(effect).toHaveBeenCalledTimes(1);
  });

  test('catches action dispatched before mount via sync check', () => {
    const store = makeStore();
    const effect = vi.fn();

    // Dispatch before the hook mounts
    store.dispatch({ type: 'EARLY_ACTION' });

    renderHook(() => useReduxEffect(effect, 'EARLY_ACTION'), {
      wrapper: makeWrapper(store),
    });

    expect(effect).toHaveBeenCalledTimes(1);
    expect(effect).toHaveBeenCalledWith(
      expect.objectContaining({ type: 'EARLY_ACTION' }),
    );
  });

  test('uses latest effect callback via ref', () => {
    const store = makeStore();
    const effect1 = vi.fn();
    const effect2 = vi.fn();

    const { rerender } = renderHook(
      ({ cb }) => useReduxEffect(cb, 'TEST'),
      {
        wrapper: makeWrapper(store),
        initialProps: { cb: effect1 },
      },
    );

    rerender({ cb: effect2 });

    act(() => {
      store.dispatch({ type: 'TEST' });
    });

    expect(effect1).not.toHaveBeenCalled();
    expect(effect2).toHaveBeenCalledTimes(1);
  });

});
