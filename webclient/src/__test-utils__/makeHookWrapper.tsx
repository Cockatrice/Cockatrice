import { ReactNode } from 'react';
import { Provider } from 'react-redux';
import { configureStore, Reducer } from '@reduxjs/toolkit';

import { WebClientContext } from '../hooks/useWebClient';
import type { WebClient } from '../websocket';
import { createMockWebClient } from './mockWebClient';

// Minimal Provider wrapper for hook-only tests. Use this instead of
// `renderWithProviders` when you need `renderHook` — the full provider tree
// auto-instantiates the singleton store via `@app/store`, which races with
// any test-local store you preload. Deep-import the reducer(s) you need and
// pass them here (see useCurrentGame.spec.tsx for the canonical pattern).

export function makeReduxHookWrapper<S>(
  reducer: Reducer<S>,
  preloadedState: S,
) {
  const store = configureStore({
    reducer,
    preloadedState: preloadedState as Parameters<typeof configureStore>[0]['preloadedState'],
  });
  function Wrapper({ children }: { children: ReactNode }) {
    return <Provider store={store}>{children}</Provider>;
  }
  return { Wrapper, store };
}

export interface MakeReduxWebClientHookWrapperOptions<S> {
  reducer: Reducer<S>;
  preloadedState: S;
  webClient?: WebClient;
}

export function makeReduxWebClientHookWrapper<S>({
  reducer,
  preloadedState,
  webClient,
}: MakeReduxWebClientHookWrapperOptions<S>) {
  const store = configureStore({
    reducer,
    preloadedState: preloadedState as Parameters<typeof configureStore>[0]['preloadedState'],
  });
  const client = webClient ?? createMockWebClient();
  function Wrapper({ children }: { children: ReactNode }) {
    return (
      <Provider store={store}>
        <WebClientContext value={client}>{children}</WebClientContext>
      </Provider>
    );
  }
  return { Wrapper, store, webClient: client };
}
