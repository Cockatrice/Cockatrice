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
  ProtobufService: vi.fn().mockImplementation(function ProtobufServiceImpl(transport: SocketTransport) {
    captured.pbOptions = transport;
    return {
      handleMessageEvent: vi.fn(),
      resetCommands: vi.fn(),
    };
  }),
}));

import { WebClient } from './WebClient';
import { WebSocketService } from './services/WebSocketService';
import { ProtobufService } from './services/ProtobufService';
import { StatusEnum } from './types/StatusEnum';
import { Subject } from 'rxjs';
import { Mock } from 'vitest';
import { SocketTransport } from './services/ProtobufService';
import { WebSocketServiceConfig } from './services/WebSocketService';
import type { IWebClientResponse } from './types/WebClientResponse';
import type { IWebClientRequest } from './types/WebClientRequest';
import type { ConnectTarget } from './types/WebClientConfig';
import { installMockWebSocket } from './__mocks__/helpers';
import { create, setExtension, toBinary } from '@bufbuild/protobuf';
import {
  Event_ServerIdentification_ext,
  Event_ServerIdentification_ServerOptions,
  Event_ServerIdentificationSchema,
  ServerMessageSchema,
  ServerMessage_MessageType,
  SessionEventSchema,
} from '@app/generated';
import { PROTOCOL_VERSION } from './config';

function buildServerIdentificationMessage({
  protocolVersion = PROTOCOL_VERSION,
  serverOptions = 0,
}: { protocolVersion?: number; serverOptions?: number } = {}): Uint8Array {
  const ident = create(Event_ServerIdentificationSchema, {
    serverName: 'TestServer',
    serverVersion: '2.8.0',
    protocolVersion,
    serverOptions,
  });
  const sessionEvent = create(SessionEventSchema);
  setExtension(sessionEvent, Event_ServerIdentification_ext, ident);
  const server = create(ServerMessageSchema, {
    messageType: ServerMessage_MessageType.SESSION_EVENT,
    sessionEvent,
  });
  return toBinary(ServerMessageSchema, server);
}

function makeMockResponse(): IWebClientResponse {
  return {
    session: {
      initialized: vi.fn(),
      connectionAttempted: vi.fn(),
      connectionFailed: vi.fn(),
      clearStore: vi.fn(),
      updateStatus: vi.fn(),
      testConnectionSuccessful: vi.fn(),
      testConnectionFailed: vi.fn(),
    },
    room: { clearStore: vi.fn() },
    game: { clearStore: vi.fn() },
    admin: {},
    moderator: {},
  } as unknown as IWebClientResponse;
}

function makeMockRequest(): IWebClientRequest {
  return {} as IWebClientRequest;
}

describe('WebClient', () => {
  let client: WebClient;
  let mockResponse: IWebClientResponse;
  let mockRequest: IWebClientRequest;
  let messageSubject: Subject<MessageEvent>;

  beforeEach(() => {
    (WebClient as unknown as { _instance: WebClient | null })._instance = null;

    (ProtobufService as Mock).mockImplementation(function ProtobufServiceImpl(transport: SocketTransport) {
      captured.pbOptions = transport;
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
    client = new WebClient(mockRequest, mockResponse);
  });

  afterEach(() => {
    vi.restoreAllMocks();
    (WebClient as unknown as { _instance: WebClient | null })._instance = null;
  });

  describe('constructor', () => {
    it('stores the request and response on the instance', () => {
      expect(client.request).toBe(mockRequest);
      expect(client.response).toBe(mockResponse);
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
      expect(() => new WebClient(makeMockRequest(), makeMockResponse())).toThrow(/singleton/);
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
      const target: ConnectTarget = { host: 'h', port: '1' };
      client.connect(target);
      expect(mockResponse.session.connectionAttempted).toHaveBeenCalled();
    });

    it('calls socket.connect with target', () => {
      const target: ConnectTarget = { host: 'h', port: '1' };
      client.connect(target);
      expect(client.socket.connect).toHaveBeenCalledWith(target);
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

    const target: ConnectTarget = { host: 'h', port: '1' };

    it('creates a WebSocket with the correct URL', () => {
      client.testConnect(target);
      expect(MockWS).toHaveBeenCalledWith(expect.stringContaining('://h:1'));
    });

    it('routes path-bearing hosts through the default TLS port (nginx proxy)', () => {
      client.testConnect({ host: 'server.example.com/servatrice', port: '4748' });
      expect(MockWS).toHaveBeenCalledWith(expect.stringMatching(/:\/\/server\.example\.com\/servatrice$/));
    });

    it('dispatches testConnectionSuccessful with supportsHashedPassword=true when the bit is set', () => {
      client.testConnect(target);
      const data = buildServerIdentificationMessage({
        serverOptions: Event_ServerIdentification_ServerOptions.SupportsPasswordHash,
      });
      wsMockInstance.onmessage({ data: data.buffer });
      expect(mockResponse.session.testConnectionSuccessful).toHaveBeenCalledWith(true);
      expect(wsMockInstance.close).toHaveBeenCalled();
    });

    it('dispatches testConnectionSuccessful with supportsHashedPassword=false for naked-password servers', () => {
      client.testConnect(target);
      const data = buildServerIdentificationMessage({ serverOptions: 0 });
      wsMockInstance.onmessage({ data: data.buffer });
      expect(mockResponse.session.testConnectionSuccessful).toHaveBeenCalledWith(false);
    });

    it('fails on protocol-version mismatch instead of reporting success', () => {
      client.testConnect(target);
      const data = buildServerIdentificationMessage({ protocolVersion: PROTOCOL_VERSION + 1 });
      wsMockInstance.onmessage({ data: data.buffer });
      expect(mockResponse.session.testConnectionFailed).toHaveBeenCalled();
      expect(mockResponse.session.testConnectionSuccessful).not.toHaveBeenCalled();
    });

    it('calls testConnectionFailed on error', () => {
      client.testConnect(target);
      wsMockInstance.onerror();
      expect(mockResponse.session.testConnectionFailed).toHaveBeenCalled();
    });

    it('fires testConnectionFailed when ServerIdentification never arrives before the keepalive timeout', () => {
      client.testConnect(target);
      vi.advanceTimersByTime(5000);
      expect(wsMockInstance.close).toHaveBeenCalled();
      expect(mockResponse.session.testConnectionFailed).toHaveBeenCalled();
    });

    it('closes the prior in-flight socket on rapid re-click', () => {
      const { instances } = installMockWebSocket();
      // The fresh installMockWebSocket replaces the stub from beforeEach so
      // we observe the next two constructions in isolation.
      client.testConnect(target);
      const first = instances[instances.length - 1];
      expect(first.close).not.toHaveBeenCalled();

      client.testConnect(target);
      expect(first.close).toHaveBeenCalled();
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
      client.updateStatus(StatusEnum.CONNECTED);
      expect(client.status).toBe(StatusEnum.CONNECTED);
    });

    it('calls protobuf.resetCommands on DISCONNECTED', () => {
      client.updateStatus(StatusEnum.DISCONNECTED);
      expect(client.protobuf.resetCommands).toHaveBeenCalled();
    });

    it('does not reset protobuf when status is not DISCONNECTED', () => {
      client.updateStatus(StatusEnum.CONNECTED);
      expect(client.protobuf.resetCommands).not.toHaveBeenCalled();
    });
  });

  describe('constructor closures', () => {
    it('keepAliveFn is set to ping function in WebSocketService', () => {
      expect(captured.wsOptions!.keepAliveFn).toBeDefined();
      expect(typeof captured.wsOptions!.keepAliveFn).toBe('function');
    });

    it('onStatusChange routes to response.session.updateStatus and updates own status', () => {
      captured.wsOptions!.onStatusChange(StatusEnum.CONNECTED, 'Connected');
      expect(mockResponse.session.updateStatus).toHaveBeenCalledWith(StatusEnum.CONNECTED, 'Connected');
      expect(client.status).toBe(StatusEnum.CONNECTED);
    });

    it('onConnectionFailed routes to response.session.connectionFailed', () => {
      captured.wsOptions!.onConnectionFailed();
      expect(mockResponse.session.connectionFailed).toHaveBeenCalled();
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
