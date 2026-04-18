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
  test('selecting a host with remember=false does NOT call settings.update', () => {
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

    // After mount + all host-sync effects settle, the form has updated its
    // local fields to reflect the selected host. What MUST NOT happen is a
    // write to the persisted autoConnect setting.
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
