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
  const instances: ReturnType<typeof makeMockWebSocketInstance>[] = [mockInstance];
  let firstCall = true;
  const MockWS = vi.fn(function MockWebSocket() {
    if (firstCall) {
      firstCall = false;
      return mockInstance;
    }
    const next = makeMockWebSocketInstance();
    instances.push(next);
    return next;
  }) as any;
  MockWS.OPEN = 1;
  MockWS.CLOSED = 3;
  (globalThis as any).WebSocket = MockWS;
  return { MockWS, mockInstance, instances, restore: () => {
    (globalThis as any).WebSocket = originalWebSocket;
  } };
}
