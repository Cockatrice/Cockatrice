// ensure jest-dom is always available during testing to cut down on boilerplate
import '@testing-library/jest-dom/vitest';

// With isolate: false, all test files share the same module context.
// Restore all mocks/spies after each test to prevent leakage between tests.
afterEach(() => {
  vi.restoreAllMocks();
  vi.useRealTimers();
});
