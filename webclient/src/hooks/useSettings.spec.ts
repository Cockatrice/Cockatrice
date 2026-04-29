import { renderHook, act, waitFor } from '@testing-library/react';

const mockSave = vi.fn();
let storedSetting: any = null;

vi.mock('@app/services', () => ({
  SettingDTO: class MockSettingDTO {
    user: string;
    autoConnect = false;
    constructor(user: string) {
      this.user = user;
    }
    save = mockSave;
    static get = vi.fn(() => Promise.resolve(storedSetting));
  },
}));

vi.mock('@app/types', () => ({
  App: { APP_USER: '*app' },
}));

// Each spec resets module state so the shared store starts fresh.
let useSettingsModule: typeof import('./useSettings');
let LoadingState: typeof import('./useSharedStore').LoadingState;

beforeEach(async () => {
  vi.resetModules();
  storedSetting = null;
  mockSave.mockClear();
  useSettingsModule = await import('./useSettings');
  ({ LoadingState } = await import('./useSharedStore'));
});

describe('useSettings', () => {
  test('starts in loading state, then resolves to the stored setting', async () => {
    storedSetting = { user: '*app', autoConnect: true, save: mockSave };

    const { result } = renderHook(() => useSettingsModule.useSettings());

    expect(result.current.status).toBe(LoadingState.LOADING);

    await waitFor(() => {
      expect(result.current.status).toBe(LoadingState.READY);
    });

    if (result.current.status === LoadingState.READY) {
      expect(result.current.value.autoConnect).toBe(true);
    }
  });

  test('creates and saves a new SettingDTO when none exists', async () => {
    storedSetting = null;

    const { result } = renderHook(() => useSettingsModule.useSettings());

    await waitFor(() => {
      expect(result.current.status).toBe(LoadingState.READY);
    });

    if (result.current.status === LoadingState.READY) {
      expect(result.current.value.autoConnect).toBe(false);
    }
    expect(mockSave).toHaveBeenCalledTimes(1);
  });

  test('update() persists the patch and re-renders with a new snapshot', async () => {
    storedSetting = { user: '*app', autoConnect: false, save: mockSave };

    const { result } = renderHook(() => useSettingsModule.useSettings());

    await waitFor(() => {
      expect(result.current.status).toBe(LoadingState.READY);
    });

    mockSave.mockClear();
    const before = result.current;

    await act(async () => {
      await result.current.update({ autoConnect: true });
    });

    expect(result.current).not.toBe(before);
    if (result.current.status === LoadingState.READY) {
      expect(result.current.value.autoConnect).toBe(true);
    }
    expect(mockSave).toHaveBeenCalledTimes(1);
  });

  test('does not re-save on the initial load when setting already exists', async () => {
    storedSetting = { user: '*app', autoConnect: true, save: mockSave };

    renderHook(() => useSettingsModule.useSettings());

    await waitFor(() => {
      expect(mockSave).not.toHaveBeenCalled();
    });
  });
});
