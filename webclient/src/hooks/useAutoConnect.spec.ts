import { renderHook, act, waitFor } from '@testing-library/react';
import { useAutoConnect } from './useAutoConnect';

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

describe('useAutoConnect', () => {
  beforeEach(() => {
    storedSetting = null;
    mockSave.mockClear();
  });

  test('returns undefined initially, then resolves to stored autoConnect value', async () => {
    storedSetting = { user: '*app', autoConnect: true, save: mockSave };

    const { result } = renderHook(() => useAutoConnect());

    expect(result.current[0]).toBeUndefined();

    await waitFor(() => {
      expect(result.current[0]).toBe(true);
    });
  });

  test('creates and saves a new SettingDTO when none exists', async () => {
    storedSetting = null;

    const { result } = renderHook(() => useAutoConnect());

    await waitFor(() => {
      expect(result.current[0]).toBe(false);
    });

    // save() called once for the newly created setting
    expect(mockSave).toHaveBeenCalledTimes(1);
  });

  test('persists when setAutoConnect is called', async () => {
    storedSetting = { user: '*app', autoConnect: false, save: mockSave };

    const { result } = renderHook(() => useAutoConnect());

    await waitFor(() => {
      expect(result.current[0]).toBe(false);
    });

    mockSave.mockClear();

    act(() => {
      result.current[1](true);
    });

    await waitFor(() => {
      expect(result.current[0]).toBe(true);
    });

    expect(mockSave).toHaveBeenCalled();
  });

  test('does not save redundantly on initial mount when setting exists', async () => {
    storedSetting = { user: '*app', autoConnect: true, save: mockSave };

    renderHook(() => useAutoConnect());

    await waitFor(() => {
      expect(mockSave).not.toHaveBeenCalled();
    });
  });
});
