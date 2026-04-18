import { useSyncExternalStore } from 'react';

export enum LoadingState {
  LOADING = 'loading',
  READY = 'ready',
  ERROR = 'error',
}

export interface Loadable<T> {
  status: LoadingState;
  value?: T;
  error?: Error;
}

export interface SharedStore<T> {
  // Reactive surface: subscribe + snapshot back useSyncExternalStore so
  // consuming components re-render on every store update.
  subscribe: (cb: () => void) => () => void;
  getSnapshot: () => Loadable<T>;

  // One-shot surface: whenReady resolves with the initial loaded value and
  // never fires again. Callers that only need "read once after init" (e.g.
  // the auto-login orchestrator) use this to avoid subscribing to updates
  // they don't care about — which would otherwise turn a user preference
  // toggle into a re-evaluation of startup logic.
  whenReady: () => Promise<T>;

  // Mutator-side helpers, not for consumption inside render.
  setValue: (value: T) => void;
  peek: () => T | undefined;

  // Clear cached state and the resolved readyPromise; the next subscribe /
  // whenReady call triggers a fresh load. In production nobody calls this;
  // integration tests use it to discard per-test Dexie state without
  // paying the cost of vi.resetModules across the whole dep graph.
  reset: () => void;
}

export function createSharedStore<T>(load: () => Promise<T>): SharedStore<T> {
  let state: Loadable<T> = { status: LoadingState.LOADING };
  const subscribers = new Set<() => void>();
  let loadStarted = false;

  // whenReady is lazy: we only attach a promise once someone asks for one.
  // This avoids Node's unhandled-rejection bookkeeping for stores whose
  // loader fails but never had a whenReady caller.
  let readyPromise: Promise<T> | null = null;

  const notify = () => {
    for (const cb of subscribers) {
      cb();
    }
  };

  const ensureLoaded = () => {
    if (loadStarted) {
      return;
    }
    loadStarted = true;
    load().then(
      (value) => {
        state = { status: LoadingState.READY, value };
        notify();
      },
      (error: unknown) => {
        const err = error instanceof Error ? error : new Error(String(error));
        state = { status: LoadingState.ERROR, error: err };
        notify();
      }
    );
  };

  const subscribe = (cb: () => void) => {
    subscribers.add(cb);
    ensureLoaded();
    return () => {
      subscribers.delete(cb);
    };
  };

  return {
    subscribe,
    getSnapshot: () => state,
    whenReady: () => {
      ensureLoaded();
      if (!readyPromise) {
        readyPromise = new Promise<T>((resolve, reject) => {
          const settle = (): boolean => {
            if (state.status === LoadingState.READY) {
              resolve(state.value as T);
              return true;
            }
            if (state.status === LoadingState.ERROR && state.error) {
              reject(state.error);
              return true;
            }
            return false;
          };
          if (settle()) {
            return;
          }
          const unsub = subscribe(() => {
            if (settle()) {
              unsub();
            }
          });
        });
      }
      return readyPromise;
    },
    setValue: (value) => {
      state = { status: LoadingState.READY, value };
      notify();
    },
    peek: () => (state.status === LoadingState.READY ? (state.value as T) : undefined),
    reset: () => {
      state = { status: LoadingState.LOADING };
      loadStarted = false;
      readyPromise = null;
      notify();
    },
  };
}

export function useSharedStore<T>(store: SharedStore<T>): Loadable<T> {
  return useSyncExternalStore(store.subscribe, store.getSnapshot);
}
