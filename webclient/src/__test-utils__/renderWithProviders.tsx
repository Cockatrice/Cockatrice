import { ReactElement } from 'react';
import { render, RenderOptions } from '@testing-library/react';
import { configureStore, EnhancedStore } from '@reduxjs/toolkit';
import { Provider } from 'react-redux';
import { MemoryRouter } from 'react-router-dom';
import { I18nextProvider } from 'react-i18next';
import i18n from 'i18next';
import { initReactI18next } from 'react-i18next';

import { gamesReducer } from '../store/game';
import { roomsReducer } from '../store/rooms';
import { serverReducer } from '../store/server';
import { actionReducer } from '../store/actions';
import { ToastProvider } from '../components/Toast/ToastContext';
import type { RootState } from '../store/store';

// Minimal i18n instance for tests — returns keys as-is. A non-empty
// `resources` entry is required so i18next registers `en-US` as a known
// language; otherwise `i18n.resolvedLanguage` stays `undefined`, which
// LanguageDropdown seeds into a MUI Select and MUI warns "out-of-range
// value `undefined`". Value is an empty translation map, since tests
// already assert on i18n keys directly.
const testI18n = i18n.createInstance();
testI18n.use(initReactI18next).init({
  lng: 'en-US',
  resources: { 'en-US': { translation: {} } },
  fallbackLng: 'en-US',
  interpolation: { escapeValue: false },
});

function createTestStore(preloadedState?: Partial<RootState>) {
  return configureStore({
    reducer: {
      games: gamesReducer,
      rooms: roomsReducer,
      server: serverReducer,
      action: actionReducer,
    },
    preloadedState: preloadedState as any,
  });
}

interface ExtendedRenderOptions extends Omit<RenderOptions, 'wrapper'> {
  preloadedState?: Partial<RootState>;
  store?: EnhancedStore;
  route?: string;
}

export function renderWithProviders(
  ui: ReactElement,
  {
    preloadedState,
    store = createTestStore(preloadedState),
    route = '/',
    ...renderOptions
  }: ExtendedRenderOptions = {},
) {
  function Wrapper({ children }: { children: React.ReactNode }) {
    return (
      <Provider store={store}>
        <I18nextProvider i18n={testI18n}>
          <ToastProvider>
            <MemoryRouter initialEntries={[route]}>
              {children}
            </MemoryRouter>
          </ToastProvider>
        </I18nextProvider>
      </Provider>
    );
  }

  return {
    store,
    ...render(ui, { wrapper: Wrapper, ...renderOptions }),
  };
}
