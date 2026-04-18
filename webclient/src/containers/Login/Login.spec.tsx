/**
 * Login auto-connect integration tests.
 *
 * Exercises the full wire from `useAutoLogin` through the Login container
 * into `webClient.request.authentication.login`. Scenarios mirror the user-
 * visible cycles we care about:
 *   - cold start with / without auto-connect
 *   - logout within the same session must NOT re-auto-connect
 *   - page refresh (fresh JS context) resets the gate
 *
 * The startup-check gate lives on the `autoLoginSession` object exported by
 * `useAutoLogin.ts`. Tests flip it back to false in `beforeEach` to stand in
 * for a page refresh between scenarios. `vi.resetModules()` would be the
 * natural equivalent but is prohibitively slow in the full suite because it
 * forces every imported module to re-evaluate.
 */

import { act, waitFor } from '@testing-library/react';

import { renderWithProviders, createMockWebClient, disconnectedState } from '../../__test-utils__';

// Lets pending microtasks resolve inside an act() scope so that the state
// updates they trigger (useFormState subscribers, useFireOnce state, etc.)
// are captured. Without this, useAutoLogin's Promise.all resolves *after*
// render returns, and React warns "update ... was not wrapped in act".
const flushEffects = async (): Promise<void> => {
  await act(async () => {
    await new Promise((resolve) => setTimeout(resolve, 0));
  });
};
import { makeSettings, makeSettingsHook } from '../../hooks/__mocks__/useSettings';
import { makeHost, makeKnownHostsHook } from '../../hooks/__mocks__/useKnownHosts';
import { autoLoginSession } from '../../hooks/useAutoLogin';
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
vi.mock('../../hooks/useWebClient', () => ({
  useWebClient: () => hoisted.mockWebClient,
  WebClientProvider: ({ children }: { children: any }) => children,
}));

beforeAll(() => {
  const client = createMockWebClient();
  (client.request.authentication as any).testConnection = vi.fn();
  hoisted.mockWebClient = client;
});

afterEach(async () => {
  // Absorb any state updates that lingered past the test body (e.g.
  // useAutoLogin's Promise.all resolving a moment too late) so they're
  // wrapped in act and don't trip React's warning during teardown.
  await flushEffects();
});

beforeEach(() => {
  // "Page refresh" between tests: reset the session gate that useAutoLogin
  // uses to prevent re-firing within a JS session. Production code only
  // writes this flag once, from inside the startup effect; tests flip it
  // back to false here to simulate a fresh browser tab.
  autoLoginSession.startupCheckRan = false;

  // clearAllMocks in the global afterEach only clears call history; mock
  // implementations (mockResolvedValue, mockReturnValue) persist. Reset
  // them explicitly so a previous test's arming doesn't leak into this one.
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

    // First mount: Login appears, useAutoLogin fires login.
    const first = renderWithProviders(<Login />, { preloadedState: disconnectedState });
    await waitFor(() => {
      expect(hoisted.mockWebClient.request.authentication.login).toHaveBeenCalledTimes(1);
    });

    // Simulate arriving at /server and then logging out: Login unmounts,
    // then a fresh Login mounts again with disconnected state.
    first.unmount();
    renderWithProviders(<Login />, { preloadedState: disconnectedState });

    await flushEffects();

    // No second login call — the session gate is latched.
    expect(hoisted.mockWebClient.request.authentication.login).toHaveBeenCalledTimes(1);
  });

  test('does not auto-connect when user enabled autoConnect mid-session and then logged out', async () => {
    // Scenario: user manually logs in with autoConnect=false. They tick the
    // auto-connect checkbox during that session (the setting flips true).
    // They log out. Returning to /login must NOT auto-connect — the setting
    // change was a preference for NEXT launch, not a signal to log in.

    // First mount: autoConnect=false, so the startup check runs and finds
    // nothing to do. The gate latches anyway.
    const first = renderWithProviders(<Login />, { preloadedState: disconnectedState });
    await flushEffects();
    expect(hoisted.mockWebClient.request.authentication.login).not.toHaveBeenCalled();

    first.unmount();

    // Mid-session, user ticked the checkbox. Future getSettings resolves
    // return the new value, but the session gate prevents a re-check.
    hoisted.getSettings.mockResolvedValue(makeSettings({ autoConnect: true }));

    renderWithProviders(<Login />, { preloadedState: disconnectedState });
    await flushEffects();
    expect(hoisted.mockWebClient.request.authentication.login).not.toHaveBeenCalled();
  });
});

describe('Login — refresh cycle', () => {
  // `beforeEach` flips autoLoginSession.startupCheckRan back to false, which
  // stands in for a page refresh. This test just re-asserts the positive
  // case: a refresh re-enables auto-connect when the persisted preference
  // still says yes.
  test('a fresh session gate re-fires auto-login when conditions still hold', async () => {
    armAutoConnect();

    renderWithProviders(<Login />, { preloadedState: disconnectedState });

    await waitFor(() => {
      expect(hoisted.mockWebClient.request.authentication.login).toHaveBeenCalledTimes(1);
    });
  });
});
