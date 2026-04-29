import { installMockWebSocket } from '../__mocks__/helpers';
import { withMockLocation } from '../../__test-utils__';
import { Mock } from 'vitest';

vi.mock('../config', () => ({
  CLIENT_OPTIONS: { keepalive: 1000 },
}));

import { WebSocketService } from './WebSocketService';
import type { WebSocketServiceConfig, ReconnectConfig } from './WebSocketService';
import { KeepAliveService } from './KeepAliveService';
import { StatusEnum } from '../types/StatusEnum';

type WebSocketInternal = WebSocketService & {
  keepAliveService: KeepAliveService;
};

let MockWS: Mock;
let mockInstance: ReturnType<typeof installMockWebSocket>['mockInstance'];
let restoreWebSocket: ReturnType<typeof installMockWebSocket>['restore'];
let mockConfig: WebSocketServiceConfig;
let mockOnConnectionFailed: Mock;
let mockOnStatusChange: Mock;
let locationRestores: Array<() => void>;

beforeEach(() => {
  vi.useFakeTimers();

  const installed = installMockWebSocket();
  MockWS = installed.MockWS;
  mockInstance = installed.mockInstance;
  restoreWebSocket = installed.restore;

  mockOnConnectionFailed = vi.fn();
  mockOnStatusChange = vi.fn();

  mockConfig = {
    keepAliveFn: vi.fn(),
    onConnectionFailed: mockOnConnectionFailed,
    onStatusChange: mockOnStatusChange,
  };

  locationRestores = [];
});

afterEach(() => {
  while (locationRestores.length > 0) {
    locationRestores.pop()!();
  }
  restoreWebSocket();
  vi.useRealTimers();
});

describe('WebSocketService', () => {
  function createConnectedService() {
    const service = new WebSocketService(mockConfig);
    service.connect({ host: 'h', port: '1' }, 'ws');
    return service;
  }

  describe('constructor', () => {
    it('subscribes disconnected$ from KeepAliveService', () => {
      const service = new WebSocketService(mockConfig);
      expect(service).toBeDefined();
    });

    it('calls disconnect and updateStatus when keepAlive disconnected$ fires', () => {
      const service = new WebSocketService(mockConfig);
      service.connect({ host: 'localhost', port: '8080' }, 'ws');
      // trigger keepAliveService.disconnected$
      (service as WebSocketInternal).keepAliveService.disconnected$.next();
      expect(mockInstance.close).toHaveBeenCalled();
      expect(mockOnStatusChange).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, 'Connection timeout');
    });
  });

  describe('connect', () => {
    it('creates a WebSocket with wss protocol by default', () => {
      const service = new WebSocketService(mockConfig);
      locationRestores.push(withMockLocation({ hostname: 'example.com' }));
      service.connect({ host: 'example.com', port: '8080' });
      expect(MockWS).toHaveBeenCalledWith('wss://example.com:8080');
    });

    it('switches to ws protocol when hostname is localhost', () => {
      const service = new WebSocketService(mockConfig);
      locationRestores.push(withMockLocation({ hostname: 'localhost' }));
      service.connect({ host: 'somehost', port: '1234' });
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

    it('calls onStatusChange CONNECTED on open', () => {
      createConnectedService();
      mockInstance.onopen();
      expect(mockOnStatusChange).toHaveBeenCalledWith(StatusEnum.CONNECTED, 'Connected');
    });

    it('starts the ping loop with the keepalive interval', () => {
      const service = new WebSocketService(mockConfig);
      const startSpy = vi.spyOn((service as WebSocketInternal).keepAliveService, 'startPingLoop');
      service.connect({ host: 'h', port: '1' }, 'ws');
      mockInstance.onopen();
      expect(startSpy).toHaveBeenCalledWith(1000, expect.any(Function));
    });

    it('ping loop callback calls keepAliveFn', () => {
      const service = new WebSocketService(mockConfig);
      const startSpy = vi.spyOn((service as WebSocketInternal).keepAliveService, 'startPingLoop');
      service.connect({ host: 'h', port: '1' }, 'ws');
      mockInstance.onopen();
      const pingCb = startSpy.mock.calls[0][1] as (done: Function) => void;
      const done = vi.fn();
      pingCb(done);
      expect(mockConfig.keepAliveFn).toHaveBeenCalledWith(done);
    });
  });

  describe('socket event handlers (onclose)', () => {
    it('calls onStatusChange DISCONNECTED on close when not already DISCONNECTED', () => {
      createConnectedService();
      mockInstance.onclose();
      expect(mockOnStatusChange).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, 'Connection Closed');
    });

    it('does not overwrite status if already DISCONNECTED', () => {
      createConnectedService();
      mockInstance.onerror();
      mockInstance.onclose();
      expect(mockOnStatusChange).not.toHaveBeenCalledWith(StatusEnum.DISCONNECTED, 'Connection Closed');
    });

    it('ends the ping loop on close', () => {
      const service = new WebSocketService(mockConfig);
      const endSpy = vi.spyOn((service as WebSocketInternal).keepAliveService, 'endPingLoop');
      service.connect({ host: 'h', port: '1' }, 'ws');
      mockInstance.onclose();
      expect(endSpy).toHaveBeenCalled();
    });
  });

  describe('socket event handlers (onerror)', () => {
    it('calls onStatusChange DISCONNECTED on error', () => {
      createConnectedService();
      mockInstance.onerror();
      expect(mockOnStatusChange).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, 'Connection Failed');
    });

    it('calls onConnectionFailed on error', () => {
      createConnectedService();
      mockInstance.onerror();
      expect(mockOnConnectionFailed).toHaveBeenCalled();
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

    it('does not throw when socket is undefined (before connect)', () => {
      const service = new WebSocketService(mockConfig);
      const data = new Uint8Array([1, 2, 3]);
      expect(() => service.send(data)).not.toThrow();
    });

    it('skips send when readyState is not OPEN', () => {
      const warnSpy = vi.spyOn(console, 'warn').mockImplementation(() => {});
      const service = createConnectedService();
      // CONNECTING
      mockInstance.readyState = 0;
      const data = new Uint8Array([1, 2, 3]);
      service.send(data);
      expect(mockInstance.send).not.toHaveBeenCalled();
      expect(warnSpy).toHaveBeenCalledWith(
        '[WebSocketService] send() skipped: socket not OPEN',
        0,
      );
      warnSpy.mockRestore();
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
      const service = new WebSocketService(mockConfig);
      // no connect called, socket is undefined
      expect(service.checkReadyState(WebSocket.OPEN)).toBe(false);
    });
  });

  describe('connect (re-entry)', () => {
    it('closes the prior socket when connect is called twice', () => {
      const service = new WebSocketService(mockConfig);
      service.connect({ host: 'h', port: '1' }, 'ws');
      const firstInstance = mockInstance;
      service.connect({ host: 'h', port: '2' }, 'ws');
      expect(firstInstance.close).toHaveBeenCalled();
    });
  });

  describe('reconnect', () => {
    const reconnect: ReconnectConfig = {
      maxAttempts: 3,
      baseDelayMs: 100,
      maxDelayMs: 1000,
    };

    function createReconnectService() {
      const service = new WebSocketService({ ...mockConfig, reconnect });
      service.connect({ host: 'h', port: '1' }, 'ws');
      mockInstance.onopen();
      return service;
    }

    it('emits RECONNECTING (not DISCONNECTED) on unexpected close when configured', () => {
      createReconnectService();
      mockOnStatusChange.mockClear();
      mockInstance.onclose();
      const statuses = mockOnStatusChange.mock.calls.map(c => c[0]);
      expect(statuses).toContain(StatusEnum.RECONNECTING);
      expect(statuses).not.toContain(StatusEnum.DISCONNECTED);
    });

    it('does not reconnect after explicit disconnect()', () => {
      const service = createReconnectService();
      mockOnStatusChange.mockClear();
      service.disconnect();
      // onclose fires with `intentionalDisconnect` already set; reconnect is
      // suppressed and the status settles on DISCONNECTED.
      mockInstance.onclose();
      const statuses = mockOnStatusChange.mock.calls.map(c => c[0]);
      expect(statuses).not.toContain(StatusEnum.RECONNECTING);
      expect(statuses).toContain(StatusEnum.DISCONNECTED);
    });

    it('gives up after maxAttempts and emits DISCONNECTED', () => {
      const { instances } = installMockWebSocket();
      const service = new WebSocketService({ ...mockConfig, reconnect });
      service.connect({ host: 'h', port: '1' }, 'ws');
      // Flip hasEverOpened so reconnect is eligible, then drop.
      instances[0].onopen();
      mockOnStatusChange.mockClear();

      // Walk through maxAttempts failed reconnects (each socket never opens,
      // so the attempt counter keeps climbing). One final close after the
      // counter hits max emits DISCONNECTED.
      for (let i = 0; i <= reconnect.maxAttempts; i += 1) {
        instances[i].onclose();
        vi.advanceTimersByTime(reconnect.maxDelayMs);
      }

      const statuses = mockOnStatusChange.mock.calls.map(c => c[0]);
      expect(statuses).toContain(StatusEnum.DISCONNECTED);
      void service;
    });

    it('resets attempt counter on successful open', () => {
      createReconnectService();
      mockOnStatusChange.mockClear();
      // simulate a drop, advance timer to trigger reconnect attempt
      mockInstance.onclose();
      vi.advanceTimersByTime(reconnect.maxDelayMs);
      // manually open the new socket (simulated)
      mockInstance.onopen();
      mockOnStatusChange.mockClear();
      // another drop — the description should start the attempt counter from 1 again
      mockInstance.onclose();
      const firstReconnect = mockOnStatusChange.mock.calls.find(c => c[0] === StatusEnum.RECONNECTING);
      expect(firstReconnect?.[1]).toMatch(/attempt 1\//);
    });
  });

});
