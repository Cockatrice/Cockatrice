import { ReactElement } from 'react';
import { render, RenderOptions } from '@testing-library/react';
import { configureStore, EnhancedStore } from '@reduxjs/toolkit';
import { Provider } from 'react-redux';
import { MemoryRouter } from 'react-router-dom';
import { I18nextProvider } from 'react-i18next';
import i18n from 'i18next';
import { initReactI18next } from 'react-i18next';
import { DndContext } from '@dnd-kit/core';
import { createTheme, ThemeProvider } from '@mui/material/styles';

// Disables MUI's ripple animation in tests. The ripple fires a deferred
// state update after clicks/focus that would otherwise trigger a noisy
// "update to ForwardRef(TouchRipple) was not wrapped in act(...)" warning.
const testTheme = createTheme({
  components: {
    MuiButtonBase: { defaultProps: { disableRipple: true } },
  },
});

import { WebClientContext } from '../hooks/useWebClient';
import type { WebClient } from '../websocket';
import rootReducer from '../store/rootReducer';
import { ToastProvider } from '../components/Toast/ToastContext';
import { storeMiddlewareOptions } from '../store/store';
import type { RootState } from '../store/store';
import { createMockWebClient } from './mockWebClient';

// Non-empty `resources` registers en-US so `resolvedLanguage` is defined;
// without it MUI warns about out-of-range Select values.
const testI18n = i18n.createInstance();
testI18n.use(initReactI18next).init({
  lng: 'en-US',
  resources: { 'en-US': { translation: {} } },
  fallbackLng: 'en-US',
  interpolation: { escapeValue: false },
});

// `configureStore`'s `preloadedState` wants `PreloadedState<CombinedState<…>>`
// which narrows collection types past our slice interfaces. A single cast
// here keeps the test harness loose (each test injects only the slices it
// cares about) while specs themselves stay strict via `makeStoreState`.
function createTestStore(preloadedState?: Partial<RootState>) {
  return configureStore({
    reducer: rootReducer,
    preloadedState: preloadedState as Parameters<typeof configureStore>[0]['preloadedState'],
    // Share the production middleware config so the serializableCheck
    // tolerates protobuf messages (isMessage) the same way the real store
    // does — otherwise every proto-payload dispatch in tests spams stderr.
    middleware: (getDefaultMiddleware) => getDefaultMiddleware(storeMiddlewareOptions),
  });
}

interface ExtendedRenderOptions extends Omit<RenderOptions, 'wrapper'> {
  preloadedState?: Partial<RootState>;
  store?: EnhancedStore;
  route?: string;
  webClient?: WebClient;
}

export function renderWithProviders(
  ui: ReactElement,
  {
    preloadedState,
    store = createTestStore(preloadedState),
    route = '/',
    webClient = createMockWebClient(),
    ...renderOptions
  }: ExtendedRenderOptions = {},
) {
  function Wrapper({ children }: { children: React.ReactNode }) {
    return (
      <Provider store={store}>
        <I18nextProvider i18n={testI18n}>
          <ThemeProvider theme={testTheme}>
            <ToastProvider>
              <MemoryRouter initialEntries={[route]}>
                <WebClientContext value={webClient}>
                  <DndContext
                    accessibility={{
                      screenReaderInstructions: { draggable: '' },
                    }}
                  >
                    {children}
                  </DndContext>
                </WebClientContext>
              </MemoryRouter>
            </ToastProvider>
          </ThemeProvider>
        </I18nextProvider>
      </Provider>
    );
  }

  return {
    store,
    webClient,
    ...render(ui, { wrapper: Wrapper, ...renderOptions }),
  };
}
