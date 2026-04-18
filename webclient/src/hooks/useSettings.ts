import { SettingDTO } from '@app/services';
import { App } from '@app/types';

import { createSharedStore, Loadable, useSharedStore } from './useSharedStore';

// First-time bootstrap: SettingDTO.get returns undefined when no row exists
// for the app user yet (fresh install, or a user who has never hit the
// settings code path before). We materialize a default DTO and persist it so
// subsequent loads always see a non-null row.
//
// Exported as `settingsStore` so integration tests can call
// `settingsStore.reset()` between scenarios — the module cache would
// otherwise serve stale data across per-test Dexie resets. Production code
// goes through `useSettings()` / `getSettings()` and doesn't touch this.
export const settingsStore = createSharedStore<SettingDTO>(async () => {
  let loaded = await SettingDTO.get(App.APP_USER);
  if (!loaded) {
    loaded = new SettingDTO(App.APP_USER);
    await loaded.save();
  }
  return loaded;
});
const store = settingsStore;

export type SettingsHook = Loadable<SettingDTO> & {
  update: (patch: Partial<SettingDTO>) => Promise<void>;
};

export function useSettings(): SettingsHook {
  const state = useSharedStore(store);

  const update = async (patch: Partial<SettingDTO>) => {
    // Fail-fast if a caller tries to write before the initial load resolves.
    // Shouldn't happen in normal flow (the checkbox is gated on the hook's
    // ready status), so surface the bug loudly instead of silently no-oping.
    const current = store.peek();
    if (!current) {
      throw new Error('useSettings.update called before settings loaded');
    }
    Object.assign(current, patch);
    await current.save();
    store.setValue(current);
  };

  return { ...state, update };
}

// Non-reactive one-shot accessor. Use this from code that wants the loaded
// value exactly once and does NOT want to re-run when the user subsequently
// edits their settings — e.g. the auto-login orchestrator, which consults
// the persisted preference at startup only.
export const getSettings = (): Promise<SettingDTO> => store.whenReady();
