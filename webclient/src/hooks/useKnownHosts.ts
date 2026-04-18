import { DefaultHosts, HostDTO } from '@app/services';
import { App } from '@app/types';

import { createSharedStore, Loadable, useSharedStore } from './useSharedStore';

// Shared-store scope justification: multiple components on the login screen
// read the same host list and selected host simultaneously (KnownHosts
// dropdown, LoginForm's host-sync effect, useAutoLogin, and the Login
// container's post-login write). Collapsing to useState inside one component
// would duplicate Dexie reads and race on lastSelected updates — exactly the
// bug we set out to fix.
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

  // No row marked lastSelected yet (first-ever load after seeding, or legacy
  // data). Pin hosts[0] and persist so subsequent boots are deterministic.
  const selected = hosts[0];
  selected.lastSelected = true;
  await selected.save();
  return { hosts, selectedHost: selected };
};

// Exported for integration-test reset; see settingsStore for the rationale.
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

// Guard for mutators. Mutators run outside React render, so we can't gate
// them through the hook's status; peek + throw is the fail-fast alternative.
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

// Non-reactive one-shot accessor, mirroring getSettings. See the comment on
// that export in useSettings.ts for the rationale.
export const getKnownHosts = (): Promise<KnownHostsValue> => store.whenReady();
