import { act, fireEvent, waitFor } from '@testing-library/react';

import { renderWithProviders, createMockWebClient, disconnectedState } from '../../__test-utils__';

const flushEffects = async (): Promise<void> => {
  await act(async () => {
    await new Promise((resolve) => setTimeout(resolve, 0));
  });
};
import { makeSettings, makeSettingsHook } from '../../hooks/__mocks__/useSettings';
import { makeHost, makeKnownHostsHook } from '../../hooks/__mocks__/useKnownHosts';
import { autoLoginGate } from '../../hooks/useAutoLogin';
import { LoadingState } from '@app/hooks';
import Login from './Login';

const hoisted = vi.hoisted(() => ({
  mockWebClient: undefined as any,
  getSettings: vi.fn(),
  getKnownHosts: vi.fn(),
  useSettings: vi.fn(),
  useKnownHosts: vi.fn(),
}));

vi.mock('../../hooks/useSettings', () => ({
  useSettings: hoisted.useSettings,
  getSettings: hoisted.getSettings,
}));
vi.mock('../../hooks/useKnownHosts', () => ({
  useKnownHosts: hoisted.useKnownHosts,
  getKnownHosts: hoisted.getKnownHosts,
}));
vi.mock('../../hooks/useWebClient', async (importOriginal) => {
  const actual = await importOriginal<typeof import('../../hooks/useWebClient')>();
  return {
    ...actual,
    useWebClient: () => hoisted.mockWebClient,
    WebClientProvider: ({ children }: { children: any }) => children,
  };
});

beforeAll(() => {
  const client = createMockWebClient();
  (client.request.authentication as any).testConnection = vi.fn();
  hoisted.mockWebClient = client;
});

afterEach(async () => {
  await flushEffects();
});

beforeEach(() => {
  autoLoginGate.hasChecked = false;

  hoisted.getSettings.mockReset();
  hoisted.getKnownHosts.mockReset();
  hoisted.useSettings.mockReset();
  hoisted.useKnownHosts.mockReset();

  const defaultHost = makeHost({
    id: 1,
    remember: true,
    userName: 'alice',
    hashedPassword: 'stored-hash',
    lastSelected: true,
  });

  hoisted.useSettings.mockReturnValue(
    makeSettingsHook({
      status: LoadingState.READY,
      value: makeSettings({ autoConnect: false }),
      update: vi.fn().mockResolvedValue(undefined),
    })
  );
  hoisted.useKnownHosts.mockReturnValue(
    makeKnownHostsHook({
      status: LoadingState.READY,
      value: { hosts: [defaultHost], selectedHost: defaultHost },
    })
  );
  hoisted.getSettings.mockResolvedValue(makeSettings({ autoConnect: false }));
  hoisted.getKnownHosts.mockResolvedValue({
    hosts: [defaultHost],
    selectedHost: defaultHost,
  });
});

const armAutoConnect = () => {
  const host = makeHost({
    id: 1,
    remember: true,
    userName: 'alice',
    hashedPassword: 'stored-hash',
    lastSelected: true,
  });
  hoisted.getSettings.mockResolvedValue(makeSettings({ autoConnect: true }));
  hoisted.getKnownHosts.mockResolvedValue({ hosts: [host], selectedHost: host });
};

describe('Login — auto-connect cold start', () => {
  test('fires login when settings + host say go', async () => {
    armAutoConnect();

    renderWithProviders(<Login />, { preloadedState: disconnectedState });

    await waitFor(() => {
      expect(hoisted.mockWebClient.request.authentication.login).toHaveBeenCalledTimes(1);
    });
    expect(hoisted.mockWebClient.request.authentication.login.mock.calls[0][0]).toMatchObject({
      userName: 'alice',
      hashedPassword: 'stored-hash',
    });
  });

  test('does not fire when autoConnect setting is off', async () => {
    renderWithProviders(<Login />, { preloadedState: disconnectedState });

    await flushEffects();
    expect(hoisted.mockWebClient.request.authentication.login).not.toHaveBeenCalled();
  });

  test('does not fire when selected host has no stored credentials', async () => {
    const host = makeHost({
      id: 1,
      remember: false,
      userName: undefined,
      hashedPassword: undefined,
      lastSelected: true,
    });
    hoisted.getSettings.mockResolvedValue(makeSettings({ autoConnect: true }));
    hoisted.getKnownHosts.mockResolvedValue({ hosts: [host], selectedHost: host });

    renderWithProviders(<Login />, { preloadedState: disconnectedState });

    await flushEffects();
    expect(hoisted.mockWebClient.request.authentication.login).not.toHaveBeenCalled();
  });
});

describe('Login — logout cycle (same JS session)', () => {
  test('does not re-auto-connect after first auto-login + logout', async () => {
    armAutoConnect();

    const first = renderWithProviders(<Login />, { preloadedState: disconnectedState });
    await waitFor(() => {
      expect(hoisted.mockWebClient.request.authentication.login).toHaveBeenCalledTimes(1);
    });

    first.unmount();
    renderWithProviders(<Login />, { preloadedState: disconnectedState });

    await flushEffects();

    expect(hoisted.mockWebClient.request.authentication.login).toHaveBeenCalledTimes(1);
  });

  test('does not auto-connect when user enabled autoConnect mid-session and then logged out', async () => {
    const first = renderWithProviders(<Login />, { preloadedState: disconnectedState });
    await flushEffects();
    expect(hoisted.mockWebClient.request.authentication.login).not.toHaveBeenCalled();

    first.unmount();

    hoisted.getSettings.mockResolvedValue(makeSettings({ autoConnect: true }));

    renderWithProviders(<Login />, { preloadedState: disconnectedState });
    await flushEffects();
    expect(hoisted.mockWebClient.request.authentication.login).not.toHaveBeenCalled();
  });

  test('submits with the restored host after a logout→remount without Required error', async () => {
    const first = renderWithProviders(<Login />, { preloadedState: disconnectedState });
    await flushEffects();
    first.unmount();

    // Submit button stays disabled until testConnectionStatus resolves to 'success';
    // preload it so the click actually dispatches.
    const { getByRole, queryByText } = renderWithProviders(<Login />, {
      preloadedState: {
        ...disconnectedState,
        server: { ...(disconnectedState.server as any), testConnectionStatus: 'success' },
      },
    });
    await flushEffects();

    fireEvent.click(getByRole('button', { name: /LoginForm\.label\.login/ }));
    await flushEffects();

    expect(queryByText(/required/i)).toBeNull();
    expect(hoisted.mockWebClient.request.authentication.login).toHaveBeenCalledTimes(1);
    expect(hoisted.mockWebClient.request.authentication.login.mock.calls[0][0]).toMatchObject({
      userName: 'alice',
    });
  });
});

describe('Login — refresh cycle', () => {
  test('a fresh session gate re-fires auto-login when conditions still hold', async () => {
    armAutoConnect();

    renderWithProviders(<Login />, { preloadedState: disconnectedState });

    await waitFor(() => {
      expect(hoisted.mockWebClient.request.authentication.login).toHaveBeenCalledTimes(1);
    });
  });
});

// End-to-end regression: the symptom reported on this branch was "save-password
// checkbox shows, login succeeds, but HostDTO.hashedPassword stays empty in
// Dexie". These tests wire the full chain — auto-login fires onSubmitLogin
// (capturing the form in rememberLoginRef), then a store.dispatch of
// LOGIN_SUCCESSFUL drives the useReduxEffect that calls knownHosts.update.
describe('Login — LOGIN_SUCCESSFUL → knownHosts persistence', () => {
  const armWithUpdate = () => {
    const update = vi.fn().mockResolvedValue(undefined);
    const host = makeHost({
      id: 7,
      remember: true,
      userName: 'alice',
      hashedPassword: 'stored-hash',
      supportsHashedPassword: true,
      lastSelected: true,
    });
    hoisted.useSettings.mockReturnValue(
      makeSettingsHook({
        status: LoadingState.READY,
        value: makeSettings({ autoConnect: true }),
        update: vi.fn().mockResolvedValue(undefined),
      })
    );
    hoisted.useKnownHosts.mockReturnValue(
      makeKnownHostsHook({
        status: LoadingState.READY,
        value: { hosts: [host], selectedHost: host },
        update,
      })
    );
    hoisted.getSettings.mockResolvedValue(makeSettings({ autoConnect: true }));
    hoisted.getKnownHosts.mockResolvedValue({ hosts: [host], selectedHost: host });
    return { update, hostId: host.id! };
  };

  test('persists userName + hashedPassword when loginSuccessful carries a real hash', async () => {
    const { update, hostId } = armWithUpdate();

    const { store } = renderWithProviders(<Login />, {
      preloadedState: {
        ...disconnectedState,
        server: { ...(disconnectedState.server as any), testConnectionStatus: 'success' },
      },
    });
    await waitFor(() => {
      expect(hoisted.mockWebClient.request.authentication.login).toHaveBeenCalledTimes(1);
    });

    store.dispatch({
      type: 'server/loginSuccessful',
      payload: { options: { hashedPassword: 'real-hash-xyz' } },
    });
    await flushEffects();

    expect(update).toHaveBeenCalledWith(hostId, {
      remember: true,
      userName: 'alice',
      hashedPassword: 'real-hash-xyz',
    });
  });

  test('does not persist credentials when hashedPassword is empty (empty-salt fallback)', async () => {
    const { update, hostId } = armWithUpdate();

    const { store } = renderWithProviders(<Login />, {
      preloadedState: {
        ...disconnectedState,
        server: { ...(disconnectedState.server as any), testConnectionStatus: 'success' },
      },
    });
    await waitFor(() => {
      expect(hoisted.mockWebClient.request.authentication.login).toHaveBeenCalledTimes(1);
    });

    // Empty-salt fallback path: login went through as plain password, so the
    // response layer has no hash to carry forward.
    store.dispatch({
      type: 'server/loginSuccessful',
      payload: { options: { hashedPassword: undefined } },
    });
    await flushEffects();

    // Guard in useLogin.updateHost clears remember+credentials so next load
    // reflects that save-password wasn't honoured — no stale "checked" checkbox
    // sitting against a null hash.
    expect(update).toHaveBeenCalledWith(hostId, {
      remember: false,
      userName: null,
      hashedPassword: null,
    });
  });
});
