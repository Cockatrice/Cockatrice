const captured = vi.hoisted(() => ({
  wsOptions: null as WebSocketServiceConfig | null,
  pbOptions: null as SocketTransport | null,
}));

vi.mock('./services/WebSocketService', () => ({
  WebSocketService: vi.fn().mockImplementation(function WebSocketServiceImpl(options: WebSocketServiceConfig) {
    captured.wsOptions = options;
    return {
      message$: { subscribe: vi.fn() },
      connect: vi.fn(),
      disconnect: vi.fn(),
      send: vi.fn(),
      checkReadyState: vi.fn().mockReturnValue(true),
    };
  }),
}));

vi.mock('./services/ProtobufService', () => ({
  ProtobufService: vi.fn().mockImplementation(function ProtobufServiceImpl(options: SocketTransport) {
    captured.pbOptions = options;
    return {
      handleMessageEvent: vi.fn(),
      resetCommands: vi.fn(),
    };
  }),
}));

vi.mock('./commands/session', () => ({
  ping: vi.fn(),
}));

import { WebClient } from './WebClient';
import { WebSocketService } from './services/WebSocketService';
import { ProtobufService } from './services/ProtobufService';
import { ping } from './commands/session';
import { App, Enriched } from '@app/types';
import { Subject } from 'rxjs';
import { Mock } from 'vitest';
import { SocketTransport } from './services/ProtobufService';
import { WebSocketServiceConfig } from './services/WebSocketService';
import type { IWebClientResponse, IWebClientRequest } from './interfaces';
import { installMockWebSocket } from './__mocks__/helpers';

function makeMockResponse(): IWebClientResponse {
  return {
    session: {
      initialized: vi.fn(),
      connectionAttempted: vi.fn(),
      clearStore: vi.fn(),
      updateStatus: vi.fn(),
    },
    room: { clearStore: vi.fn() },
    game: { clearStore: vi.fn() },
    admin: {},
    moderator: {},
  } as unknown as IWebClientResponse;
}

function makeMockRequest(): IWebClientRequest {
  return {
    authentication: {},
    session: {},
    rooms: {},
    admin: {},
    moderator: {},
  } as unknown as IWebClientRequest;
}

describe('WebClient', () => {
  let client: WebClient;
  let mockResponse: IWebClientResponse;
  let mockRequest: IWebClientRequest;
  let messageSubject: Subject<MessageEvent>;

  beforeEach(() => {
    // Reset the singleton so each test starts fresh.
    (WebClient as unknown as { _instance: WebClient | null })._instance = null;

    (ProtobufService as Mock).mockImplementation(function ProtobufServiceImpl(options: SocketTransport) {
      captured.pbOptions = options;
      return {
        handleMessageEvent: vi.fn(),
        resetCommands: vi.fn(),
      };
    });
    messageSubject = new Subject<MessageEvent>();
    (WebSocketService as Mock).mockImplementation(function WebSocketServiceImpl(options: WebSocketServiceConfig) {
      captured.wsOptions = options;
      return {
        message$: messageSubject,
        connect: vi.fn(),
        disconnect: vi.fn(),
        send: vi.fn(),
        checkReadyState: vi.fn().mockReturnValue(true),
      };
    });
    vi.spyOn(console, 'log').mockImplementation(() => {});

    mockResponse = makeMockResponse();
    mockRequest = makeMockRequest();
    client = new WebClient(mockResponse, mockRequest);
  });

  afterEach(() => {
    vi.restoreAllMocks();
    (WebClient as unknown as { _instance: WebClient | null })._instance = null;
  });

  describe('constructor', () => {
    it('stores the response and request on the instance', () => {
      expect(client.response).toBe(mockResponse);
      expect(client.request).toBe(mockRequest);
    });

    it('subscribes socket.message$ to protobuf.handleMessageEvent', () => {
      const event = { data: new ArrayBuffer(0) } as MessageEvent;
      messageSubject.next(event);
      expect(client.protobuf.handleMessageEvent).toHaveBeenCalledWith(event);
    });

    it('calls response.session.initialized', () => {
      expect(mockResponse.session.initialized).toHaveBeenCalled();
    });

    it('sets WebClient.instance to the constructed instance', () => {
      expect(WebClient.instance).toBe(client);
    });

    it('throws when instantiated more than once', () => {
      expect(() => new WebClient(makeMockResponse(), makeMockRequest())).toThrow(/singleton/);
    });
  });

  describe('static instance accessor', () => {
    it('throws when accessed before construction', () => {
      (WebClient as unknown as { _instance: WebClient | null })._instance = null;
      expect(() => WebClient.instance).toThrow(/not been initialized/);
    });
  });

  describe('connect', () => {
    it('calls response.session.connectionAttempted', () => {
      const opts: Enriched.WebSocketConnectOptions = { host: 'h', port: '1', reason: App.WebSocketConnectReason.LOGIN, userName: 'u' };
      client.connect(opts);
      expect(mockResponse.session.connectionAttempted).toHaveBeenCalled();
    });

    it('stores options and calls socket.connect', () => {
      const opts: Enriched.WebSocketConnectOptions = { host: 'h', port: '1', reason: App.WebSocketConnectReason.LOGIN, userName: 'u' };
      client.connect(opts);
      expect(client.options).toBe(opts);
      expect(client.socket.connect).toHaveBeenCalledWith(opts);
    });
  });

  describe('testConnect', () => {
    let MockWS: ReturnType<typeof installMockWebSocket>['MockWS'];
    let wsMockInstance: ReturnType<typeof installMockWebSocket>['mockInstance'];
    let restoreWS: ReturnType<typeof installMockWebSocket>['restore'];

    beforeEach(() => {
      vi.useFakeTimers();
      const installed = installMockWebSocket();
      MockWS = installed.MockWS;
      wsMockInstance = installed.mockInstance;
      restoreWS = installed.restore;
    });

    afterEach(() => {
      restoreWS();
      vi.useRealTimers();
    });

    const opts: Enriched.WebSocketConnectOptions = { host: 'h', port: '1', reason: App.WebSocketConnectReason.LOGIN, userName: 'u' };

    it('creates a WebSocket with the correct URL', () => {
      client.testConnect(opts);
      expect(MockWS).toHaveBeenCalledWith(expect.stringContaining('://h:1'));
    });

    it('calls testConnectionSuccessful and closes on open', () => {
      (mockResponse.session as any).testConnectionSuccessful = vi.fn();
      client.testConnect(opts);
      wsMockInstance.onopen();
      expect((mockResponse.session as any).testConnectionSuccessful).toHaveBeenCalled();
      expect(wsMockInstance.close).toHaveBeenCalled();
    });

    it('calls testConnectionFailed on error', () => {
      (mockResponse.session as any).testConnectionFailed = vi.fn();
      client.testConnect(opts);
      wsMockInstance.onerror();
      expect((mockResponse.session as any).testConnectionFailed).toHaveBeenCalled();
    });

    it('closes socket after keepalive timeout', () => {
      client.testConnect(opts);
      vi.advanceTimersByTime(5000);
      expect(wsMockInstance.close).toHaveBeenCalled();
    });
  });

  describe('disconnect', () => {
    it('delegates to socket.disconnect', () => {
      client.disconnect();
      expect(client.socket.disconnect).toHaveBeenCalled();
    });
  });

  describe('updateStatus', () => {
    it('sets the status', () => {
      client.updateStatus(App.StatusEnum.CONNECTED);
      expect(client.status).toBe(App.StatusEnum.CONNECTED);
    });

    it('calls protobuf.resetCommands on DISCONNECTED', () => {
      client.updateStatus(App.StatusEnum.DISCONNECTED);
      expect(client.protobuf.resetCommands).toHaveBeenCalled();
    });

    it('does not reset protobuf when status is not DISCONNECTED', () => {
      client.updateStatus(App.StatusEnum.CONNECTED);
      expect(client.protobuf.resetCommands).not.toHaveBeenCalled();
    });
  });

  describe('constructor closures', () => {
    it('keepAliveFn calls ping with the callback', () => {
      const cb = vi.fn();
      captured.wsOptions!.keepAliveFn(cb);
      expect(ping).toHaveBeenCalledWith(cb);
    });

    it('onStatusChange routes to response.session.updateStatus and updates own status', () => {
      captured.wsOptions!.onStatusChange(App.StatusEnum.CONNECTED, 'Connected');
      expect(mockResponse.session.updateStatus).toHaveBeenCalledWith(App.StatusEnum.CONNECTED, 'Connected');
      expect(client.status).toBe(App.StatusEnum.CONNECTED);
    });

    it('send closure delegates to socket.send', () => {
      const data = new Uint8Array([1, 2, 3]);
      captured.pbOptions!.send(data);
      expect(client.socket.send).toHaveBeenCalledWith(data);
    });

    it('isOpen closure delegates to socket.checkReadyState', () => {
      const result = captured.pbOptions!.isOpen();
      expect(client.socket.checkReadyState).toHaveBeenCalledWith(WebSocket.OPEN);
      expect(result).toBe(true);
    });
  });
});
