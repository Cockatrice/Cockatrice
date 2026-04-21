import { renderWithProviders, createMockWebClient, disconnectedState } from '../../__test-utils__';
import { makeSettingsHook, makeSettings } from '../../hooks/__mocks__/useSettings';
import { makeKnownHostsHook, makeHost } from '../../hooks/__mocks__/useKnownHosts';

const hoisted = vi.hoisted(() => ({
  mockWebClient: undefined as any,
  mockUseSettings: vi.fn(),
  mockUseKnownHosts: vi.fn(),
}));

vi.mock('@app/hooks', async (importOriginal) => {
  const actual = await importOriginal<typeof import('@app/hooks')>();
  return {
    ...actual,
    useWebClient: () => hoisted.mockWebClient,
    useSettings: hoisted.mockUseSettings,
    useKnownHosts: hoisted.mockUseKnownHosts,
  };
});

import LoginForm from './LoginForm';
import { LoadingState } from '@app/hooks';

beforeAll(() => {
  const client = createMockWebClient();
  (client.request.authentication as any).testConnection = vi.fn();
  hoisted.mockWebClient = client;
});

describe('LoginForm — regression: settings.autoConnect is not clobbered by host state', () => {
  test('selecting a hashed-capable host with remember=false does NOT call settings.update', () => {
    const update = vi.fn().mockResolvedValue(undefined);

    hoisted.mockUseSettings.mockReturnValue(
      makeSettingsHook({
        status: LoadingState.READY,
        value: makeSettings({ autoConnect: true }),
        update,
      })
    );

    const host = makeHost({
      id: 1,
      remember: false,
      userName: undefined,
      hashedPassword: undefined,
      supportsHashedPassword: true,
      lastSelected: true,
    });
    hoisted.mockUseKnownHosts.mockReturnValue(
      makeKnownHostsHook({
        status: LoadingState.READY,
        value: { hosts: [host], selectedHost: host },
      })
    );

    renderWithProviders(
      <LoginForm onSubmit={vi.fn()} disableSubmitButton={false} onResetPassword={vi.fn()} />,
      { preloadedState: disconnectedState }
    );

    expect(update).not.toHaveBeenCalled();
  });

  test('auto-login never fires from the form; that is now the container concern', () => {
    const onSubmit = vi.fn();
    const update = vi.fn().mockResolvedValue(undefined);

    hoisted.mockUseSettings.mockReturnValue(
      makeSettingsHook({
        status: LoadingState.READY,
        value: makeSettings({ autoConnect: true }),
        update,
      })
    );

    const host = makeHost({
      id: 1,
      remember: true,
      userName: 'joe',
      hashedPassword: 'abc',
      supportsHashedPassword: true,
      lastSelected: true,
    });
    hoisted.mockUseKnownHosts.mockReturnValue(
      makeKnownHostsHook({
        status: LoadingState.READY,
        value: { hosts: [host], selectedHost: host },
      })
    );

    renderWithProviders(
      <LoginForm onSubmit={onSubmit} disableSubmitButton={false} onResetPassword={vi.fn()} />,
      { preloadedState: disconnectedState }
    );

    expect(onSubmit).not.toHaveBeenCalled();
  });
});

describe('LoginForm — hashed-password gating', () => {
  // The gate requires BOTH a successful test-connection AND host.supportsHashedPassword=true;
  // preload testConnectionStatus='success' so the host flag is the only lever under test.
  const testedState = {
    ...disconnectedState,
    server: { ...(disconnectedState.server as any), testConnectionStatus: 'success' as const },
  };

  const renderWith = (host: ReturnType<typeof makeHost>) => {
    hoisted.mockUseSettings.mockReturnValue(
      makeSettingsHook({
        status: LoadingState.READY,
        value: makeSettings({ autoConnect: false }),
        update: vi.fn().mockResolvedValue(undefined),
      })
    );
    hoisted.mockUseKnownHosts.mockReturnValue(
      makeKnownHostsHook({
        status: LoadingState.READY,
        value: { hosts: [host], selectedHost: host },
      })
    );
    return renderWithProviders(
      <LoginForm onSubmit={vi.fn()} disableSubmitButton={false} onResetPassword={vi.fn()} />,
      { preloadedState: testedState }
    );
  };

  const hasRemember = (root: HTMLElement) => Boolean(root.querySelector('input[name="remember"]'));
  const hasAutoConnect = (root: HTMLElement) =>
    Boolean(Array.from(root.querySelectorAll('.MuiFormControlLabel-root')).find((n) =>
      n.textContent?.includes('LoginForm.label.autoConnect'),
    ));

  test('hides Remember + Auto Connect when the host does not support hashed passwords', () => {
    const { container } = renderWith(
      makeHost({ id: 1, supportsHashedPassword: false, lastSelected: true })
    );
    expect(hasRemember(container)).toBe(false);
    expect(hasAutoConnect(container)).toBe(false);
  });

  test('hides Remember + Auto Connect when the host has never been test-connected', () => {
    const { container } = renderWith(
      makeHost({ id: 1, supportsHashedPassword: undefined, lastSelected: true })
    );
    expect(hasRemember(container)).toBe(false);
    expect(hasAutoConnect(container)).toBe(false);
  });

  test('shows Remember + Auto Connect when the host supports hashed passwords', () => {
    const { container } = renderWith(
      makeHost({ id: 1, supportsHashedPassword: true, lastSelected: true })
    );
    expect(hasRemember(container)).toBe(true);
    expect(hasAutoConnect(container)).toBe(true);
  });

  test('clears settings.autoConnect when selecting a naked-password host with it previously on', () => {
    const update = vi.fn().mockResolvedValue(undefined);
    hoisted.mockUseSettings.mockReturnValue(
      makeSettingsHook({
        status: LoadingState.READY,
        value: makeSettings({ autoConnect: true }),
        update,
      })
    );
    const host = makeHost({
      id: 1,
      supportsHashedPassword: false,
      remember: true,
      lastSelected: true,
    });
    hoisted.mockUseKnownHosts.mockReturnValue(
      makeKnownHostsHook({
        status: LoadingState.READY,
        value: { hosts: [host], selectedHost: host },
      })
    );
    renderWithProviders(
      <LoginForm onSubmit={vi.fn()} disableSubmitButton={false} onResetPassword={vi.fn()} />,
      { preloadedState: disconnectedState }
    );
    expect(update).toHaveBeenCalledWith({ autoConnect: false });
  });
});
