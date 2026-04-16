/**
 * Shared mock factories for websocket layer unit tests.
 * Import the helpers you need in each spec file via:
 *   import { makeMockWebSocket, useWebClientCleanup } from '../__mocks__/helpers';
 */
import { WebClient } from '../WebClient';

/**
 * Resets the WebClient singleton to null. Call directly, or use
 * `useWebClientCleanup()` to register automatic beforeEach/afterEach hooks.
 */
export function resetWebClientSingleton() {
  (WebClient as unknown as { _instance: WebClient | null })._instance = null;
}

/**
 * Registers beforeEach/afterEach hooks that reset the WebClient singleton.
 * Call at describe-level or file-level in any spec that mocks WebClient.
 * Prevents isolate:false singleton leakage between spec files.
 */
export function useWebClientCleanup() {
  beforeEach(() => resetWebClientSingleton());
  afterEach(() => resetWebClientSingleton());
}

/** Builds a mock WebSocket instance */
export function makeMockWebSocketInstance() {
  return {
    send: vi.fn(),
    close: vi.fn(),
    readyState: WebSocket.OPEN as number,
    binaryType: '' as BinaryType,
    onopen: null as any,
    onclose: null as any,
    onerror: null as any,
    onmessage: null as any,
  };
}

/** Installs a mock WebSocket constructor on global. Returns the mock instance and a cleanup function. */
export function installMockWebSocket() {
  const originalWebSocket = (globalThis as any).WebSocket;
  const mockInstance = makeMockWebSocketInstance();
  const MockWS = vi.fn(function MockWebSocket() {
    return mockInstance;
  }) as any;
  MockWS.OPEN = 1;
  MockWS.CLOSED = 3;
  (globalThis as any).WebSocket = MockWS;
  return { MockWS, mockInstance, restore: () => {
    (globalThis as any).WebSocket = originalWebSocket;
  } };
}
