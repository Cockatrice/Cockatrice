// Shared render helper for the app integration suite.
//
// Two non-obvious choices:
//
// 1. WebClientContext is provided directly (not via production
//    <WebClientProvider>) because the shared integration setup.ts already
//    instantiates the WebClient singleton in beforeEach. The production
//    provider would `new WebClient(...)` a second time and throw.
//
// 2. We pass the REAL Redux store from @app/store — not renderWithProviders'
//    default test-local store. The real WebClient dispatches against the
//    real store (that's what createWebClientResponse wires to). Asserting
//    against a different in-memory store would silently miss every
//    dispatch. setup.ts's resetAll + afterEach clears the real store
//    between tests, so each test still starts from a clean slate.

import { ReactElement } from 'react';

import { renderWithProviders } from '../../../src/__test-utils__';
import { store } from '@app/store';
import { WebClientContext } from '@app/hooks';
import { WebClient } from '@app/websocket';

export function renderAppScreen(ui: ReactElement) {
  return renderWithProviders(
    <WebClientContext.Provider value={WebClient.instance}>
      {ui}
    </WebClientContext.Provider>,
    { store }
  );
}

export { store };
