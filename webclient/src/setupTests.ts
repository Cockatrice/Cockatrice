// Install runtime polyfills (BigInt.prototype.toJSON) before any module
// under test loads — matches the production boot order in src/index.tsx.
import './polyfills';

// Ensure jest-dom matchers are available in every test file.
import '@testing-library/jest-dom/vitest';

// ── Global mock hygiene under `isolate: false` ────────────────────────────────
//
// Vitest is configured with `test.isolate: false` for speed — every spec file
// in a worker shares the same module graph and the same `vi.mock` factories.
// Without aggressive per-test cleanup, state leaks trivially between tests:
//
//   - A test accumulates `.mock.calls` on a shared `vi.fn()`. Later tests
//     either see stale history or accidentally match on prior invocations.
//   - A test installs `vi.spyOn` on a real method. Without restore, the spy
//     persists into every following test and file.
//   - A test swaps to fake timers. Real-time code in later tests hangs.
//
// `vi.clearAllMocks()` clears `.mock.calls` on every tracked mock without
// touching implementations — safe for module factories that produce `vi.fn()`
// instances at the top of a spec file and rely on those instances sticking
// around. `vi.restoreAllMocks()` restores original implementations on
// `vi.spyOn` targets. `vi.useRealTimers()` drops any fake-timer installation.
//
// NOTE: we intentionally do NOT call `vi.resetAllMocks()` — it resets the
// implementations of `vi.fn()` instances created inside `vi.mock(...)`
// factories, which breaks any spec that expects those mocks to persist
// across tests in the same file (e.g. `store.dispatch` mocked once at file
// load).
//
// If a specific test needs to install its own `mockReturnValue` /
// `mockImplementation`, it should set it in that test's body and rely on
// the next test overwriting or the global `clearAllMocks` clearing calls —
// it should NOT assume the mock is reset to its factory default automatically.
//
// Global snapshot/restore guards for non-`vi.spyOn` globals that tests mutate
// directly. `vi.restoreAllMocks()` only restores `vi.spyOn` targets, so bare
// `Object.defineProperty` writes on `window.location` and `globalThis.WebSocket`
// reassignments leak between tests unless we explicitly capture and restore them.
let _locationDescriptor: PropertyDescriptor | undefined;
let _originalWebSocket: typeof globalThis.WebSocket | undefined;

beforeEach(() => {
  _locationDescriptor = Object.getOwnPropertyDescriptor(window, 'location');
  _originalWebSocket = globalThis.WebSocket;
});

afterEach(() => {
  vi.clearAllMocks();
  vi.restoreAllMocks();
  vi.useRealTimers();

  const currentLocationDescriptor = Object.getOwnPropertyDescriptor(window, 'location');
  if (currentLocationDescriptor !== _locationDescriptor && _locationDescriptor) {
    Object.defineProperty(window, 'location', _locationDescriptor);
  }

  if (globalThis.WebSocket !== _originalWebSocket) {
    globalThis.WebSocket = _originalWebSocket as typeof globalThis.WebSocket;
  }
});
