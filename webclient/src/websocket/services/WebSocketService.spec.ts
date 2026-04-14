import { installMockWebSocket } from '../__mocks__/helpers';
import { Mock } from 'vitest';

vi.mock('../WebClient', () => ({
  WebClient: vi.fn(),
}));

vi.mock('../commands/session', () => ({
  updateStatus: vi.fn(),
}));

vi.mock('../persistence', () => ({
  SessionPersistence: {
    connectionFailed: vi.fn(),
    testConnectionSuccessful: vi.fn(),
    testConnectionFailed: vi.fn(),
  },
}));

import { WebSocketService } from './WebSocketService';
import { SessionPersistence } from '../persistence';
import { updateStatus } from '../commands/session';
import { StatusEnum } from 'types';

let MockWS: Mock;
let mockInstance: ReturnType<typeof installMockWebSocket>['mockInstance'];
let restoreWebSocket: ReturnType<typeof installMockWebSocket>['restore'];
let mockWebClient: any;

beforeEach(() => {
  vi.useFakeTimers();
  vi.clearAllMocks();

  const installed = installMockWebSocket();
  MockWS = installed.MockWS;
  mockInstance = installed.mockInstance;
  restoreWebSocket = installed.restore;

  mockWebClient = {
    status: StatusEnum.CONNECTED,
    clientOptions: { keepalive: 1000 },
    keepAlive: vi.fn(),
  };
});

afterEach(() => {
  restoreWebSocket();
  vi.useRealTimers();
});

describe('WebSocketService', () => {
  function createConnectedService() {
    const service = new WebSocketService(mockWebClient);
    service.connect({ host: 'h', port: 1 } as any, 'ws');
    return service;
  }

  function createTestConnectedService() {
    const service = new WebSocketService(mockWebClient);
    service.testConnect({ host: 'h', port: 1 } as any, 'ws');
    return service;
  }

  describe('constructor', () => {
    it('subscribes disconnected$ from KeepAliveService', () => {
      const service = new WebSocketService(mockWebClient);
      expect(service).toBeDefined();
    });

    it('calls disconnect and updateStatus when keepAlive disconnected$ fires', () => {
      const service = new WebSocketService(mockWebClient);
      service.connect({ host: 'localhost', port: 8080 } as any, 'ws');
      // trigger keepAliveService.disconnected$
      (service as any).keepAliveService.disconnected$.next();
      expect(mockInstance.close).toHaveBeenCalled();
      expect(updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, 'Connection timeout');
    });
  });

  describe('connect', () => {
    it('creates a WebSocket with wss protocol by default', () => {
      const service = new WebSocketService(mockWebClient);
      Object.defineProperty(window, 'location', {
        value: { hostname: 'example.com' },
        writable: true,
        configurable: true,
      });
      service.connect({ host: 'example.com', port: 8080 } as any);
      expect(MockWS).toHaveBeenCalledWith('wss://example.com:8080');
    });

    it('switches to ws protocol when hostname is localhost', () => {
      const service = new WebSocketService(mockWebClient);
      Object.defineProperty(window, 'location', {
        value: { hostname: 'localhost' },
        writable: true,
        configurable: true,
      });
      service.connect({ host: 'somehost', port: 1234 } as any);
      expect(MockWS).toHaveBeenCalledWith('ws://somehost:1234');
    });

    it('sets binaryType to arraybuffer', () => {
      createConnectedService();
      expect(mockInstance.binaryType).toBe('arraybuffer');
    });

    it('fires socket.close after keepalive timeout', () => {
      createConnectedService();
      vi.advanceTimersByTime(1000);
      expect(mockInstance.close).toHaveBeenCalled();
    });
  });

  describe('socket event handlers (onopen)', () => {
    it('clears the connection timeout when socket opens', () => {
      const clearSpy = vi.spyOn(globalThis, 'clearTimeout');
      createConnectedService();
      mockInstance.onopen();
      expect(clearSpy).toHaveBeenCalled();
    });

    it('calls updateStatus CONNECTED on open', () => {
      createConnectedService();
      mockInstance.onopen();
      expect(updateStatus).toHaveBeenCalledWith(StatusEnum.CONNECTED, 'Connected');
    });

    it('starts the ping loop with the keepalive interval', () => {
      const service = new WebSocketService(mockWebClient);
      const startSpy = vi.spyOn((service as any).keepAliveService, 'startPingLoop');
      service.connect({ host: 'h', port: 1 } as any, 'ws');
      mockInstance.onopen();
      expect(startSpy).toHaveBeenCalledWith(1000, expect.any(Function));
    });

    it('ping loop callback calls webClient.keepAlive', () => {
      const service = new WebSocketService(mockWebClient);
      const startSpy = vi.spyOn((service as any).keepAliveService, 'startPingLoop');
      service.connect({ host: 'h', port: 1 } as any, 'ws');
      mockInstance.onopen();
      const pingCb = startSpy.mock.calls[0][1] as (done: Function) => void;
      const done = vi.fn();
      pingCb(done);
      expect(mockWebClient.keepAlive).toHaveBeenCalledWith(done);
    });
  });

  describe('socket event handlers (onclose)', () => {
    it('calls updateStatus DISCONNECTED on close when not already DISCONNECTED', () => {
      createConnectedService();
      mockInstance.onclose();
      expect(updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, 'Connection Closed');
    });

    it('does not overwrite status if already DISCONNECTED', () => {
      createConnectedService();
      mockWebClient.status = StatusEnum.DISCONNECTED;
      mockInstance.onclose();
      expect(updateStatus).not.toHaveBeenCalledWith(StatusEnum.DISCONNECTED, 'Connection Closed');
    });

    it('ends the ping loop on close', () => {
      const service = new WebSocketService(mockWebClient);
      const endSpy = vi.spyOn((service as any).keepAliveService, 'endPingLoop');
      service.connect({ host: 'h', port: 1 } as any, 'ws');
      mockInstance.onclose();
      expect(endSpy).toHaveBeenCalled();
    });
  });

  describe('socket event handlers (onerror)', () => {
    it('calls updateStatus DISCONNECTED on error', () => {
      createConnectedService();
      mockInstance.onerror();
      expect(updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, 'Connection Failed');
    });

    it('calls SessionPersistence.connectionFailed on error', () => {
      createConnectedService();
      mockInstance.onerror();
      expect(SessionPersistence.connectionFailed).toHaveBeenCalled();
    });
  });

  describe('socket event handlers (onmessage)', () => {
    it('emits on message$ subject', () => {
      const service = createConnectedService();
      const handler = vi.fn();
      service.message$.subscribe(handler);
      const event = { data: new ArrayBuffer(4) } as MessageEvent;
      mockInstance.onmessage(event);
      expect(handler).toHaveBeenCalledWith(event);
    });
  });

  describe('disconnect', () => {
    it('closes the socket', () => {
      const service = createConnectedService();
      service.disconnect();
      expect(mockInstance.close).toHaveBeenCalled();
    });
  });

  describe('send', () => {
    it('delegates to socket.send', () => {
      const service = createConnectedService();
      const data = new Uint8Array([1, 2, 3]);
      service.send(data);
      expect(mockInstance.send).toHaveBeenCalledWith(data);
    });
  });

  describe('checkReadyState', () => {
    it('returns true when readyState matches', () => {
      const service = createConnectedService();
      mockInstance.readyState = WebSocket.OPEN;
      expect(service.checkReadyState(WebSocket.OPEN)).toBe(true);
    });

    it('returns false when readyState does not match', () => {
      const service = createConnectedService();
      // CLOSED
      mockInstance.readyState = 3;
      expect(service.checkReadyState(WebSocket.OPEN)).toBe(false);
    });

    it('returns false when socket is null', () => {
      const service = new WebSocketService(mockWebClient);
      // no connect called, socket is undefined
      expect(service.checkReadyState(WebSocket.OPEN)).toBe(false);
    });
  });

  describe('testConnect', () => {
    it('creates a test WebSocket with correct URL', () => {
      const service = new WebSocketService(mockWebClient);
      Object.defineProperty(window, 'location', {
        value: { hostname: 'example.com' },
        writable: true,
        configurable: true,
      });
      service.testConnect({ host: 'example.com', port: 9000 } as any);
      expect(MockWS).toHaveBeenCalledWith('wss://example.com:9000');
    });

    it('uses ws protocol on localhost', () => {
      const service = new WebSocketService(mockWebClient);
      Object.defineProperty(window, 'location', {
        value: { hostname: 'localhost' },
        writable: true,
        configurable: true,
      });
      service.testConnect({ host: 'h', port: 1 } as any);
      expect(MockWS).toHaveBeenCalledWith('ws://h:1');
    });

    it('closes previous testSocket when connecting again', () => {
      const service = new WebSocketService(mockWebClient);
      service.testConnect({ host: 'h', port: 1 } as any, 'ws');
      const firstInstance = mockInstance;
      // install second mock instance and restore after test
      const installed2 = installMockWebSocket();
      service.testConnect({ host: 'h', port: 2 } as any, 'ws');
      expect(firstInstance.close).toHaveBeenCalled();
      // restore original mock so subsequent tests see a clean global
      mockInstance = installed2.mockInstance;
      MockWS = installed2.MockWS;
    });

    it('calls SessionPersistence.testConnectionSuccessful on open', () => {
      createTestConnectedService();
      vi.spyOn(globalThis, 'clearTimeout');
      mockInstance.onopen();
      expect(SessionPersistence.testConnectionSuccessful).toHaveBeenCalled();
      expect(mockInstance.close).toHaveBeenCalled();
    });

    it('fires socket.close after keepalive timeout for testConnect', () => {
      createTestConnectedService();
      vi.advanceTimersByTime(1000);
      expect(mockInstance.close).toHaveBeenCalled();
    });

    it('calls SessionPersistence.testConnectionFailed on error', () => {
      createTestConnectedService();
      mockInstance.onerror();
      expect(SessionPersistence.testConnectionFailed).toHaveBeenCalled();
    });

    it('nulls out testSocket on close', () => {
      const service = createTestConnectedService();
      mockInstance.onclose();
      expect((service as any).testSocket).toBeNull();
    });
  });
});
