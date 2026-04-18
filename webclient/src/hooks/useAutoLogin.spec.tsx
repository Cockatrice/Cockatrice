import { renderHook, waitFor } from '@testing-library/react';

vi.mock('./useSettings');
vi.mock('./useKnownHosts');

type AnyRecord = Record<string, any>;

let useAutoLoginModule: typeof import('./useAutoLogin');
let getSettingsMock: any;
let getKnownHostsMock: any;
let makeSettings: (o?: AnyRecord) => AnyRecord;
let makeHost: (o?: AnyRecord) => AnyRecord;

beforeEach(async () => {
  // Fresh module graph per test so the module-level hasFiredThisSession flag resets.
  vi.resetModules();
  useAutoLoginModule = await import('./useAutoLogin');
  const settingsMockModule = await import('./__mocks__/useSettings');
  const hostsMockModule = await import('./__mocks__/useKnownHosts');
  getSettingsMock = settingsMockModule.getSettings;
  getKnownHostsMock = hostsMockModule.getKnownHosts;
  makeSettings = settingsMockModule.makeSettings as any;
  makeHost = hostsMockModule.makeHost as any;
});

interface ConfigureOptions {
  autoConnect?: boolean;
  remember?: boolean;
  hashedPassword?: string;
  userName?: string;
}

const configure = ({
  autoConnect = false,
  remember = false,
  hashedPassword = undefined,
  userName = 'joe',
}: ConfigureOptions) => {
  const settings = makeSettings({ autoConnect });
  const host = makeHost({ remember, hashedPassword, userName, lastSelected: true });

  getSettingsMock.mockResolvedValue(settings);
  getKnownHostsMock.mockResolvedValue({ hosts: [host], selectedHost: host });
};

describe('useAutoLogin', () => {
  test('fires onLogin when all conditions are met', async () => {
    const onLogin = vi.fn();
    configure({ autoConnect: true, remember: true, hashedPassword: 'hp', userName: 'joe' });

    renderHook(() => useAutoLoginModule.useAutoLogin(onLogin, false));

    await waitFor(() => expect(onLogin).toHaveBeenCalledTimes(1));
    expect(onLogin.mock.calls[0][0]).toMatchObject({
      userName: 'joe',
      remember: true,
      password: '',
    });
  });

  test('does not fire when settings.autoConnect is false', async () => {
    const onLogin = vi.fn();
    configure({ autoConnect: false, remember: true, hashedPassword: 'hp' });

    renderHook(() => useAutoLoginModule.useAutoLogin(onLogin, false));

    // Let the pending promise flush.
    await Promise.resolve();
    await Promise.resolve();
    expect(onLogin).not.toHaveBeenCalled();
  });

  test('does not fire when host lacks remember flag', async () => {
    const onLogin = vi.fn();
    configure({ autoConnect: true, remember: false, hashedPassword: 'hp' });

    renderHook(() => useAutoLoginModule.useAutoLogin(onLogin, false));

    await Promise.resolve();
    await Promise.resolve();
    expect(onLogin).not.toHaveBeenCalled();
  });

  test('does not fire when host lacks hashedPassword', async () => {
    const onLogin = vi.fn();
    configure({ autoConnect: true, remember: true, hashedPassword: undefined });

    renderHook(() => useAutoLoginModule.useAutoLogin(onLogin, false));

    await Promise.resolve();
    await Promise.resolve();
    expect(onLogin).not.toHaveBeenCalled();
  });

  test('does not fire when a connection attempt is already in flight', async () => {
    const onLogin = vi.fn();
    configure({ autoConnect: true, remember: true, hashedPassword: 'hp' });

    renderHook(() => useAutoLoginModule.useAutoLogin(onLogin, true));

    await Promise.resolve();
    await Promise.resolve();
    expect(onLogin).not.toHaveBeenCalled();
  });

  test('fires at most once per session, even across unmount + remount', async () => {
    const onLogin = vi.fn();
    configure({ autoConnect: true, remember: true, hashedPassword: 'hp' });

    const { unmount } = renderHook(() => useAutoLoginModule.useAutoLogin(onLogin, false));
    await waitFor(() => expect(onLogin).toHaveBeenCalledTimes(1));

    unmount();
    renderHook(() => useAutoLoginModule.useAutoLogin(onLogin, false));

    await Promise.resolve();
    await Promise.resolve();
    expect(onLogin).toHaveBeenCalledTimes(1);
  });

  test('manual login then logout does NOT auto-connect on return to /login', async () => {
    // Regression: the flag tracks whether the startup check RAN, not whether
    // it FIRED. Without that distinction, a first-session manual login (where
    // the hook saw conditions unmet) would leave the flag unset, and the
    // next mount (after logout) would find conditions met and auto-connect.
    const onLogin = vi.fn();

    // First mount: autoConnect=false, so the check runs but doesn't fire.
    configure({ autoConnect: false, remember: true, hashedPassword: 'hp' });
    const { unmount } = renderHook(() => useAutoLoginModule.useAutoLogin(onLogin, false));
    await Promise.resolve();
    await Promise.resolve();
    expect(onLogin).not.toHaveBeenCalled();

    // User logs in manually and later hits logout; Login re-mounts with
    // autoConnect now flipped on (they ticked the box during the session).
    unmount();
    configure({ autoConnect: true, remember: true, hashedPassword: 'hp' });
    renderHook(() => useAutoLoginModule.useAutoLogin(onLogin, false));

    await Promise.resolve();
    await Promise.resolve();
    expect(onLogin).not.toHaveBeenCalled();
  });

  test('ticking the auto-connect checkbox after mount does NOT trigger a login', async () => {
    // This is the specific regression: editing the persisted preference is a
    // settings write, not a "log in now" signal. Because useAutoLogin reads
    // via whenReady (one-shot) instead of subscribing, a subsequent settings
    // change cannot re-run the orchestrator.
    const onLogin = vi.fn();
    configure({ autoConnect: false, remember: true, hashedPassword: 'hp' });

    const { rerender } = renderHook(() => useAutoLoginModule.useAutoLogin(onLogin, false));
    await Promise.resolve();
    await Promise.resolve();
    expect(onLogin).not.toHaveBeenCalled();

    // Swap to a "settings.autoConnect=true" world and rerender. Since
    // getSettings is a one-shot that already resolved with the old value,
    // changing its mockResolvedValue doesn't retroactively matter.
    configure({ autoConnect: true, remember: true, hashedPassword: 'hp' });
    rerender();

    await Promise.resolve();
    await Promise.resolve();
    expect(onLogin).not.toHaveBeenCalled();
  });
});
