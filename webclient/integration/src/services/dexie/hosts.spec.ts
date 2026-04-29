// Real round-trip tests for HostDTO through Dexie into fake-indexeddb.
// Exercises the full static method surface (add, get, getAll, bulkAdd,
// delete) plus instance save().

import { beforeEach, describe, expect, it, vi } from 'vitest';

import { HostDTO } from '@app/services';
import type { App } from '@app/types';

import { resetDexie } from './resetDexie';

const makeRow = (overrides: Partial<App.Host> = {}): App.Host => ({
  name: 'Test',
  host: 'host.example',
  port: '4747',
  editable: false,
  ...overrides,
});

beforeEach(async () => {
  // Shared setup.ts installs fake timers for the websocket suite's
  // KeepAliveService; Dexie / fake-indexeddb need real timers.
  vi.useRealTimers();
  await resetDexie();
});

describe('HostDTO (real Dexie)', () => {
  it('getAll returns empty on a fresh store', async () => {
    const all = await HostDTO.getAll();
    expect(all).toEqual([]);
  });

  it('add returns an auto-incremented id and makes the row retrievable by get(id)', async () => {
    const id = (await HostDTO.add(makeRow({ name: 'A' }))) as number;
    expect(typeof id).toBe('number');

    const loaded = await HostDTO.get(id);
    expect(loaded).toBeDefined();
    expect(loaded!.name).toBe('A');
    expect(loaded!.id).toBe(id);
    expect(loaded).toBeInstanceOf(HostDTO);
  });

  it('bulkAdd seeds multiple rows and they are all retrievable via getAll', async () => {
    await HostDTO.bulkAdd([
      makeRow({ name: 'A' }),
      makeRow({ name: 'B' }),
      makeRow({ name: 'C' }),
    ]);

    const all = await HostDTO.getAll();
    expect(all.map((h) => h.name).sort()).toEqual(['A', 'B', 'C']);
  });

  it('save() on a loaded instance upserts the same row (does not duplicate)', async () => {
    const id = (await HostDTO.add(makeRow({ name: 'A', remember: false }))) as number;

    const loaded = await HostDTO.get(id);
    loaded!.remember = true;
    loaded!.userName = 'alice';
    loaded!.hashedPassword = 'stored';
    await loaded!.save();

    const all = await HostDTO.getAll();
    expect(all).toHaveLength(1);
    expect(all[0].remember).toBe(true);
    expect(all[0].userName).toBe('alice');
    expect(all[0].hashedPassword).toBe('stored');
  });

  it('delete removes the row by id', async () => {
    const idA = (await HostDTO.add(makeRow({ name: 'A' }))) as number;
    await HostDTO.add(makeRow({ name: 'B' }));

    await HostDTO.delete(idA as unknown as string);

    const all = await HostDTO.getAll();
    expect(all.map((h) => h.name)).toEqual(['B']);
  });

  it('lastSelected round-trips as a boolean column', async () => {
    const idA = (await HostDTO.add(makeRow({ name: 'A', lastSelected: true }))) as number;
    await HostDTO.add(makeRow({ name: 'B', lastSelected: false }));

    const all = await HostDTO.getAll();
    const selected = all.find((h) => h.id === idA)!;
    expect(selected.lastSelected).toBe(true);
    const other = all.find((h) => h.name === 'B')!;
    expect(other.lastSelected).toBe(false);
  });
});
