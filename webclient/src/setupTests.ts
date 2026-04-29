// @critical Must match the production boot order in src/index.tsx. See .github/instructions/webclient.instructions.md#initialization-order.
import './polyfills';

import '@testing-library/jest-dom/vitest';

// jsdom doesn't provide ResizeObserver; react-window needs it.
if (typeof globalThis.ResizeObserver === 'undefined') {
  globalThis.ResizeObserver = class {
    observe() {}
    unobserve() {}
    disconnect() {}
  } as any;
}

// Dexie eagerly opens IndexedDB on import; jsdom's fake-indexeddb is memory-intensive.
vi.mock('dexie', () => {
  const fakeTable = {
    mapToClass: () => {},
    get: () => Promise.resolve(null),
    put: () => Promise.resolve(),
    add: () => Promise.resolve(1),
    bulkAdd: () => Promise.resolve(),
    delete: () => Promise.resolve(),
    toArray: () => Promise.resolve([]),
    where: () => ({ equals: () => ({ first: () => Promise.resolve(null) }) }),
  };
  class FakeDexie {
    version() {
      return { stores: () => this };
    }
    open() {
      return Promise.resolve(this);
    }
    table() {
      return fakeTable;
    }
  }
  return { default: FakeDexie, __esModule: true };
});

// Tests within a file share the module graph (vite.config.ts sets isolate: true
// between files, not within them). Never add vi.resetAllMocks() — it resets
// vi.fn() instances created inside vi.mock(...) factories at file load.
afterEach(() => {
  vi.clearAllMocks();
  vi.restoreAllMocks();
  vi.useRealTimers();
});
