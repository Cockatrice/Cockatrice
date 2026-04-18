import { createSharedStore, LoadingState } from './useSharedStore';

describe('createSharedStore', () => {
  test('starts in LOADING state before any subscriber connects', () => {
    const store = createSharedStore(async () => 'value');
    expect(store.getSnapshot()).toEqual({ status: LoadingState.LOADING });
    expect(store.peek()).toBeUndefined();
  });

  test('triggers load on first subscribe and resolves to READY', async () => {
    let loadCalls = 0;
    const store = createSharedStore(async () => {
      loadCalls++;
      return 42;
    });

    const cb = vi.fn();
    store.subscribe(cb);

    await vi.waitFor(() => {
      expect(store.getSnapshot()).toEqual({ status: LoadingState.READY, value: 42 });
    });

    expect(loadCalls).toBe(1);
    expect(cb).toHaveBeenCalled();
    expect(store.peek()).toBe(42);
  });

  test('multiple subscribers share a single load', async () => {
    let loadCalls = 0;
    const store = createSharedStore(async () => {
      loadCalls++;
      return 'shared';
    });

    const cb1 = vi.fn();
    const cb2 = vi.fn();
    store.subscribe(cb1);
    store.subscribe(cb2);

    await vi.waitFor(() => {
      expect(store.getSnapshot()).toEqual({ status: LoadingState.READY, value: 'shared' });
    });

    expect(loadCalls).toBe(1);
    expect(cb1).toHaveBeenCalled();
    expect(cb2).toHaveBeenCalled();
  });

  test('setValue notifies subscribers with a new snapshot reference', async () => {
    const store = createSharedStore(async () => ({ n: 1 }));
    const cb = vi.fn();
    store.subscribe(cb);

    await vi.waitFor(() => expect(store.getSnapshot().status).toBe(LoadingState.READY));

    const before = store.getSnapshot();
    cb.mockClear();

    store.setValue({ n: 2 });

    const after = store.getSnapshot();
    expect(after).not.toBe(before);
    expect(after).toEqual({ status: LoadingState.READY, value: { n: 2 } });
    expect(cb).toHaveBeenCalledTimes(1);
  });

  test('transitions to ERROR when loader rejects and notifies subscribers', async () => {
    const store = createSharedStore(async () => {
      throw new Error('boom');
    });

    const cb = vi.fn();
    store.subscribe(cb);

    await vi.waitFor(() => {
      expect(store.getSnapshot().status).toBe(LoadingState.ERROR);
    });

    const snapshot = store.getSnapshot();
    expect(snapshot.status).toBe(LoadingState.ERROR);
    if (snapshot.status === LoadingState.ERROR) {
      expect(snapshot.error.message).toBe('boom');
    }
    expect(cb).toHaveBeenCalled();
    expect(store.peek()).toBeUndefined();
  });

  test('unsubscribe removes the callback', async () => {
    const store = createSharedStore(async () => 'x');
    const cb = vi.fn();
    const unsubscribe = store.subscribe(cb);

    await vi.waitFor(() => expect(store.getSnapshot().status).toBe(LoadingState.READY));
    cb.mockClear();

    unsubscribe();
    store.setValue('y');

    expect(cb).not.toHaveBeenCalled();
  });
});
