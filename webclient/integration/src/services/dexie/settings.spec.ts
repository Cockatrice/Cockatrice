// Real round-trip tests for SettingDTO through Dexie into fake-indexeddb.
// Nothing is mocked past the IndexedDB boundary — the DTO class, the Dexie
// schema, and the table's put/where/first pipeline all run as shipped code.

import { beforeEach, describe, expect, it, vi } from 'vitest';

import { SettingDTO } from '@app/services';
import { App } from '@app/types';

import { resetDexie } from './resetDexie';

beforeEach(async () => {
  // Shared setup.ts installs vi.useFakeTimers() for the websocket suite's
  // KeepAliveService needs. Dexie + fake-indexeddb rely on real microtasks
  // and will hang under fake timers, so flip back here.
  vi.useRealTimers();
  await resetDexie();
});

describe('SettingDTO (real Dexie)', () => {
  it('returns undefined for a user with no row yet', async () => {
    const loaded = await SettingDTO.get(App.APP_USER);
    expect(loaded).toBeUndefined();
  });

  it('round-trips a fresh setting via save()', async () => {
    const dto = new SettingDTO(App.APP_USER);
    dto.autoConnect = true;
    await dto.save();

    const loaded = await SettingDTO.get(App.APP_USER);
    expect(loaded).toBeDefined();
    expect(loaded!.user).toBe(App.APP_USER);
    expect(loaded!.autoConnect).toBe(true);
  });

  it('upserts on repeated save for the same user key', async () => {
    const first = new SettingDTO(App.APP_USER);
    first.autoConnect = false;
    await first.save();

    const loaded = await SettingDTO.get(App.APP_USER);
    loaded!.autoConnect = true;
    await loaded!.save();

    const reloaded = await SettingDTO.get(App.APP_USER);
    expect(reloaded!.autoConnect).toBe(true);
  });

  it('matches user lookups case-insensitively (equalsIgnoreCase in DTO.get)', async () => {
    const dto = new SettingDTO(App.APP_USER);
    await dto.save();

    const loaded = await SettingDTO.get(App.APP_USER.toUpperCase());
    expect(loaded).toBeDefined();
    expect(loaded!.user).toBe(App.APP_USER);
  });

  it('preserves the SettingDTO class on load (mapToClass binding)', async () => {
    const dto = new SettingDTO(App.APP_USER);
    await dto.save();

    const loaded = await SettingDTO.get(App.APP_USER);
    expect(loaded).toBeInstanceOf(SettingDTO);
    // The save() instance method must be present on the retrieved row so
    // call sites (useSettings.update) can round-trip without reinstantiation.
    expect(typeof loaded!.save).toBe('function');
  });
});
