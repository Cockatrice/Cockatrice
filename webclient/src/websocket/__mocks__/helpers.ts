/**
 * Shared mock factories for websocket layer unit tests.
 */

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
