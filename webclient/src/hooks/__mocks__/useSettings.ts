import type { SettingDTO } from '@app/services';
import { LoadingState } from '../useSharedStore';
import type { SettingsHook } from '../useSettings';

export const makeSettings = (overrides: Partial<SettingDTO> = {}): SettingDTO =>
  ({ user: '*app', autoConnect: false, save: vi.fn(), ...overrides }) as SettingDTO;

export const makeSettingsHook = (overrides: Partial<SettingsHook> = {}): SettingsHook =>
  ({
    status: LoadingState.READY,
    value: makeSettings(),
    update: vi.fn().mockResolvedValue(undefined),
    ...overrides,
  }) as SettingsHook;

export const useSettings = vi.fn<() => SettingsHook>(() => makeSettingsHook());

export const getSettings = vi.fn<() => Promise<SettingDTO>>(() =>
  Promise.resolve(makeSettings())
);
