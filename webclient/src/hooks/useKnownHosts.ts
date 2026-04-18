import { DefaultHosts, HostDTO } from '@app/services';
import { App } from '@app/types';

import { createSharedStore, Loadable, useSharedStore } from './useSharedStore';

export interface KnownHostsValue {
  hosts: HostDTO[];
  selectedHost: HostDTO;
}

const loadAll = async (): Promise<HostDTO[]> => {
  let hosts = await HostDTO.getAll();
  if (!hosts?.length) {
    await HostDTO.bulkAdd(DefaultHosts);
    hosts = await HostDTO.getAll();
  }
  return hosts;
};

const normalize = async (hosts: HostDTO[]): Promise<KnownHostsValue> => {
  const existing = hosts.find((h) => h.lastSelected);
  if (existing) {
    return { hosts, selectedHost: existing };
  }

  const selected = hosts[0];
  selected.lastSelected = true;
  await selected.save();
  return { hosts, selectedHost: selected };
};

export const knownHostsStore = createSharedStore<KnownHostsValue>(async () => {
  const hosts = await loadAll();
  return normalize(hosts);
});
const store = knownHostsStore;

export type KnownHostsHook = Loadable<KnownHostsValue> & {
  select: (id: number) => Promise<void>;
  add: (host: App.Host) => Promise<HostDTO>;
  update: (id: number, patch: Partial<HostDTO>) => Promise<HostDTO>;
  remove: (id: number) => Promise<void>;
};

const requireValue = (method: string): KnownHostsValue => {
  const current = store.peek();
  if (!current) {
    throw new Error(`useKnownHosts.${method} called before hosts loaded`);
  }
  return current;
};

const select = async (id: number): Promise<void> => {
  const { hosts } = requireValue('select');
  const target = hosts.find((h) => h.id === id);
  if (!target) {
    throw new Error(`useKnownHosts.select: unknown host id ${id}`);
  }

  const writes: Promise<unknown>[] = [];
  for (const h of hosts) {
    if (h === target) {
      if (!h.lastSelected) {
        h.lastSelected = true;
        writes.push(h.save());
      }
    } else if (h.lastSelected) {
      h.lastSelected = false;
      writes.push(h.save());
    }
  }
  await Promise.all(writes);

  store.setValue({ hosts: [...hosts], selectedHost: target });
};

const add = async (host: App.Host): Promise<HostDTO> => {
  const { hosts, selectedHost } = requireValue('add');
  const created = await HostDTO.get((await HostDTO.add(host)) as number);
  store.setValue({ hosts: [...hosts, created], selectedHost });
  return created;
};

const update = async (id: number, patch: Partial<HostDTO>): Promise<HostDTO> => {
  const { hosts, selectedHost } = requireValue('update');
  const existing = hosts.find((h) => h.id === id);
  if (!existing) {
    throw new Error(`useKnownHosts.update: unknown host id ${id}`);
  }
  Object.assign(existing, patch);
  await existing.save();
  store.setValue({
    hosts: [...hosts],
    selectedHost: selectedHost.id === id ? existing : selectedHost,
  });
  return existing;
};

const remove = async (id: number): Promise<void> => {
  const { hosts, selectedHost } = requireValue('remove');
  await HostDTO.delete(id as unknown as string);
  const next = hosts.filter((h) => h.id !== id);
  let nextSelected = selectedHost;
  if (selectedHost.id === id) {
    nextSelected = next[0];
    if (nextSelected) {
      nextSelected.lastSelected = true;
      await nextSelected.save();
    }
  }
  store.setValue({ hosts: next, selectedHost: nextSelected });
};

export function useKnownHosts(): KnownHostsHook {
  const state = useSharedStore(store);
  return { ...state, select, add, update, remove };
}

export const getKnownHosts = (): Promise<KnownHostsValue> => store.whenReady();
