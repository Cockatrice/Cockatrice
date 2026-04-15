// Shared lifecycle helpers for test files that need to mutate global state.
//
// The root `setupTests.ts` guards catch leaks even when callers forget to
// clean up, but opt-in helpers make intent explicit at the call site and
// avoid piling cleanup logic onto the shared safety net.

/**
 * Temporarily override fields on `window.location` and return a restore fn.
 *
 * `Object.defineProperty(window, 'location', ...)` is not a `vi.spyOn` target,
 * so `vi.restoreAllMocks()` will NOT undo it. Always pair with the returned
 * `restore` callback (ideally in `afterEach`).
 */
export function withMockLocation(overrides: Partial<Location>): () => void {
  const originalDescriptor = Object.getOwnPropertyDescriptor(window, 'location');

  Object.defineProperty(window, 'location', {
    value: { ...window.location, ...overrides },
    writable: true,
    configurable: true,
  });

  return () => {
    if (originalDescriptor) {
      Object.defineProperty(window, 'location', originalDescriptor);
    }
  };
}

/**
 * Push an entry onto a shared event-handler registry array and return a
 * teardown function that removes exactly that entry.
 *
 * Used by ProtobufService specs which install temporary handlers into the
 * (mocked) `GameEvents` / `RoomEvents` / `SessionEvents` arrays. Manual
 * `.push()`/`.pop()` inside a test body corrupts the array if an assertion
 * throws between them — this helper makes the teardown safe to run in
 * `afterEach`.
 */
export function withEventRegistry<T>(registry: T[], entry: T): () => void {
  registry.push(entry);
  return () => {
    const index = registry.lastIndexOf(entry);
    if (index !== -1) {
      registry.splice(index, 1);
    }
  };
}
