import { renderHook, act, waitFor } from '@testing-library/react';

type StoredHost = {
  id?: number;
  name: string;
  host: string;
  port: string;
  editable: boolean;
  lastSelected?: boolean;
  userName?: string;
  hashedPassword?: string;
  remember?: boolean;
  save?: ReturnType<typeof vi.fn>;
};

let stored: StoredHost[] = [];
let nextId = 1;

async function upsertStoredHost(this: StoredHost) {
  const idx = stored.findIndex((h) => h.id === this.id);
  if (idx >= 0) {
    stored[idx] = this;
  } else {
    this.id = this.id ?? nextId++;
    stored.push(this);
  }
}

const mockSave = vi.fn<(self: StoredHost) => Promise<void>>(upsertStoredHost);

vi.mock('@app/services', () => ({
  HostDTO: class MockHostDTO {
    id?: number;
    name!: string;
    host!: string;
    port!: string;
    editable!: boolean;
    lastSelected?: boolean;
    userName?: string;
    hashedPassword?: string;
    remember?: boolean;

    save = function save(this: StoredHost) {
      return mockSave.call(this);
    };

    static getAll = vi.fn(async () => {
      return stored.map((h) => {
        const inst = new MockHostDTO() as unknown as StoredHost;
        Object.assign(inst, h);
        (inst as unknown as MockHostDTO).save = function save() {
          return mockSave.call(this as unknown as StoredHost);
        };
        return inst;
      });
    });

    static get = vi.fn(async (id: number) => {
      const match = stored.find((h) => h.id === id);
      if (!match) {
        return undefined;
      }
      const inst = new MockHostDTO() as unknown as StoredHost;
      Object.assign(inst, match);
      (inst as unknown as MockHostDTO).save = function save() {
        return mockSave.call(this as unknown as StoredHost);
      };
      return inst;
    });

    static add = vi.fn(async (host: StoredHost) => {
      const id = nextId++;
      stored.push({ ...host, id });
      return id;
    });

    static bulkAdd = vi.fn(async (hosts: StoredHost[]) => {
      for (const h of hosts) {
        stored.push({ ...h, id: nextId++ });
      }
    });

    static delete = vi.fn(async (id: number | string) => {
      const numericId = typeof id === 'string' ? Number(id) : id;
      stored = stored.filter((h) => h.id !== numericId);
    });
  },
  DefaultHosts: [
    { name: 'A', host: 'a.x', port: '1', editable: false },
    { name: 'B', host: 'b.x', port: '2', editable: false },
  ],
}));

vi.mock('@app/types', () => ({ App: {} }));

let useKnownHostsModule: typeof import('./useKnownHosts');
let LoadingState: typeof import('./useSharedStore').LoadingState;

beforeEach(async () => {
  vi.resetModules();
  stored = [];
  nextId = 1;
  mockSave.mockClear();
  useKnownHostsModule = await import('./useKnownHosts');
  ({ LoadingState } = await import('./useSharedStore'));
});

describe('useKnownHosts', () => {
  test('seeds DefaultHosts when the DB is empty and pins hosts[0] as lastSelected', async () => {
    const { result } = renderHook(() => useKnownHostsModule.useKnownHosts());

    await waitFor(() => {
      expect(result.current.status).toBe(LoadingState.READY);
    });

    if (result.current.status !== LoadingState.READY) {
      throw new Error('not ready');
    }
    expect(result.current.value.hosts).toHaveLength(2);
    expect(result.current.value.selectedHost.name).toBe('A');
    expect(result.current.value.selectedHost.lastSelected).toBe(true);
  });

  test('select(id) flips lastSelected atomically — exactly one row true', async () => {
    stored = [
      { id: 1, name: 'A', host: 'a', port: '1', editable: false, lastSelected: true },
      { id: 2, name: 'B', host: 'b', port: '2', editable: false, lastSelected: false },
    ];
    nextId = 3;

    const { result } = renderHook(() => useKnownHostsModule.useKnownHosts());

    await waitFor(() => {
      expect(result.current.status).toBe(LoadingState.READY);
    });

    await act(async () => {
      await result.current.select(2);
    });

    if (result.current.status !== LoadingState.READY) {
      throw new Error('not ready');
    }
    expect(result.current.value.selectedHost.id).toBe(2);
    const lastSelectedCount = result.current.value.hosts.filter((h) => h.lastSelected).length;
    expect(lastSelectedCount).toBe(1);
  });

  test('add() persists to Dexie and mirrors the new host into in-memory state', async () => {
    stored = [{ id: 1, name: 'A', host: 'a', port: '1', editable: false, lastSelected: true }];
    nextId = 2;

    const { result } = renderHook(() => useKnownHostsModule.useKnownHosts());
    await waitFor(() => expect(result.current.status).toBe(LoadingState.READY));

    await act(async () => {
      await result.current.add({ name: 'C', host: 'c', port: '3', editable: true });
    });

    if (result.current.status !== LoadingState.READY) {
      throw new Error('not ready');
    }
    expect(result.current.value.hosts).toHaveLength(2);
    expect(result.current.value.hosts.some((h) => h.name === 'C')).toBe(true);
  });

  test('update() patches the host and replaces the snapshot reference', async () => {
    stored = [
      { id: 1, name: 'A', host: 'a', port: '1', editable: false, lastSelected: true, remember: false },
    ];
    nextId = 2;

    const { result } = renderHook(() => useKnownHostsModule.useKnownHosts());
    await waitFor(() => expect(result.current.status).toBe(LoadingState.READY));

    const before = result.current;

    await act(async () => {
      await result.current.update(1, { remember: true, userName: 'joe' });
    });

    expect(result.current).not.toBe(before);
    if (result.current.status !== LoadingState.READY) {
      throw new Error('not ready');
    }
    expect(result.current.value.hosts[0].remember).toBe(true);
    expect(result.current.value.hosts[0].userName).toBe('joe');
  });

  test('remove() deletes and picks a new selectedHost when the removed row was selected', async () => {
    stored = [
      { id: 1, name: 'A', host: 'a', port: '1', editable: false, lastSelected: true },
      { id: 2, name: 'B', host: 'b', port: '2', editable: false, lastSelected: false },
    ];
    nextId = 3;

    const { result } = renderHook(() => useKnownHostsModule.useKnownHosts());
    await waitFor(() => expect(result.current.status).toBe(LoadingState.READY));

    await act(async () => {
      await result.current.remove(1);
    });

    if (result.current.status !== LoadingState.READY) {
      throw new Error('not ready');
    }
    expect(result.current.value.hosts).toHaveLength(1);
    expect(result.current.value.selectedHost.id).toBe(2);
    expect(result.current.value.selectedHost.lastSelected).toBe(true);
  });
});
