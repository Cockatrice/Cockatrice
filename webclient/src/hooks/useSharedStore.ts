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

// @critical Two surfaces: subscribe (reactive) vs whenReady (one-shot).
// See .github/instructions/webclient.instructions.md#shared-store-pattern.
export interface SharedStore<T> {
  subscribe: (cb: () => void) => () => void;
  getSnapshot: () => Loadable<T>;
  whenReady: () => Promise<T>;
  setValue: (value: T) => void;
  peek: () => T | undefined;
  reset: () => void;
}

export function createSharedStore<T>(load: () => Promise<T>): SharedStore<T> {
  let state: Loadable<T> = { status: LoadingState.LOADING };
  const subscribers = new Set<() => void>();
  let loadStarted = false;

  // Lazy to avoid unhandled-rejection bookkeeping when no caller awaits it.
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
