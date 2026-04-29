import { SettingDTO } from '@app/services';
import { App } from '@app/types';

import { createSharedStore, Loadable, useSharedStore } from './useSharedStore';

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

export const getSettings = (): Promise<SettingDTO> => store.whenReady();
