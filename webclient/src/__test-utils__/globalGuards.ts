/**
 * Temporarily override fields on `window.location` and return a restore fn.
 *
 * @critical `Object.defineProperty(window, 'location', ...)` isn't a vi.spyOn
 * target, so `vi.restoreAllMocks()` will NOT undo it. Always invoke the
 * returned restore callback.
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
