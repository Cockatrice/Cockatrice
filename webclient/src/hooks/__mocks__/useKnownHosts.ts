import type { HostDTO } from '@app/services';
import { LoadingState } from '../useSharedStore';
import type { KnownHostsHook, KnownHostsValue } from '../useKnownHosts';

export const makeHost = (overrides: Partial<HostDTO> = {}): HostDTO =>
  ({
    id: 1,
    name: 'Test Host',
    host: 'test.example',
    port: '4747',
    editable: false,
    lastSelected: true,
    userName: undefined,
    hashedPassword: undefined,
    remember: false,
    save: vi.fn(),
    ...overrides,
  }) as unknown as HostDTO;

export const makeKnownHostsValue = (overrides: Partial<KnownHostsValue> = {}): KnownHostsValue => {
  const host = makeHost();
  return { hosts: [host], selectedHost: host, ...overrides };
};

export const makeKnownHostsHook = (overrides: Partial<KnownHostsHook> = {}): KnownHostsHook =>
  ({
    status: LoadingState.READY,
    value: makeKnownHostsValue(),
    select: vi.fn().mockResolvedValue(undefined),
    add: vi.fn(),
    update: vi.fn(),
    remove: vi.fn().mockResolvedValue(undefined),
    ...overrides,
  }) as KnownHostsHook;

export const useKnownHosts = vi.fn<() => KnownHostsHook>(() => makeKnownHostsHook());

export const getKnownHosts = vi.fn<() => Promise<KnownHostsValue>>(() =>
  Promise.resolve(makeKnownHostsValue())
);
