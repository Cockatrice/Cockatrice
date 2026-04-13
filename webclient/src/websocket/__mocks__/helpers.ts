/**
 * Shared mock factories for websocket layer unit tests.
 * Import the helpers you need in each spec file via:
 *   import { makeMockWebSocket } from '../__mocks__/helpers';
 */

/** Builds a mock WebSocket instance */
export function makeMockWebSocketInstance() {
  return {
    send: vi.fn(),
    close: vi.fn(),
    readyState: WebSocket.OPEN,
    binaryType: '' as BinaryType,
    onopen: null as any,
    onclose: null as any,
    onerror: null as any,
    onmessage: null as any,
  };
}

/** Installs a mock WebSocket constructor on global. Returns the mock instance. */
export function installMockWebSocket() {
  const mockInstance = makeMockWebSocketInstance();
  const MockWS = vi.fn(() => mockInstance) as any;
  MockWS.OPEN = 1;
  MockWS.CLOSED = 3;
  (global as any).WebSocket = MockWS;
  return { MockWS, mockInstance };
}
