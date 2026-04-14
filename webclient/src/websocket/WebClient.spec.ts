const captured = vi.hoisted(() => ({
  wsOptions: null as any,
  pbOptions: null as any,
}));

vi.mock('./services/WebSocketService', () => ({
  WebSocketService: vi.fn().mockImplementation(function WebSocketServiceImpl(options: any) {
    captured.wsOptions = options;
    return {
      message$: { subscribe: vi.fn() },
      connect: vi.fn(),
      testConnect: vi.fn(),
      disconnect: vi.fn(),
      send: vi.fn(),
      checkReadyState: vi.fn().mockReturnValue(true),
    };
  }),
}));

vi.mock('./services/ProtobufService', () => ({
  ProtobufService: vi.fn().mockImplementation(function ProtobufServiceImpl(options: any) {
    captured.pbOptions = options;
    return {
      handleMessageEvent: vi.fn(),
      resetCommands: vi.fn(),
    };
  }),
}));

vi.mock('./persistence', () => ({
  RoomPersistence: { clearStore: vi.fn() },
  SessionPersistence: { clearStore: vi.fn(), initialized: vi.fn(), connectionAttempted: vi.fn() },
}));

vi.mock('store', () => ({
  GameDispatch: { clearStore: vi.fn() },
}));

vi.mock('./commands/session', () => ({
  ping: vi.fn(),
}));

import { WebClient } from './WebClient';
import { WebSocketService } from './services/WebSocketService';
import { ProtobufService } from './services/ProtobufService';
import { RoomPersistence, SessionPersistence } from './persistence';
import { ping } from './commands/session';
import { StatusEnum } from 'types';
import { Subject } from 'rxjs';
import { Mock } from 'vitest';

describe('WebClient', () => {
  let client: WebClient;
  let messageSubject: Subject<MessageEvent>;

  beforeEach(() => {
    vi.clearAllMocks();
    (ProtobufService as Mock).mockImplementation(function ProtobufServiceImpl(options: any) {
      captured.pbOptions = options;
      return {
        handleMessageEvent: vi.fn(),
        resetCommands: vi.fn(),
      };
    });
    messageSubject = new Subject<MessageEvent>();
    (WebSocketService as Mock).mockImplementation(function WebSocketServiceImpl(options: any) {
      captured.wsOptions = options;
      return {
        message$: messageSubject,
        connect: vi.fn(),
        testConnect: vi.fn(),
        disconnect: vi.fn(),
        send: vi.fn(),
        checkReadyState: vi.fn().mockReturnValue(true),
      };
    });
    // suppress console.log from constructor in non-test-env check
    vi.spyOn(console, 'log').mockImplementation(() => {});
    client = new WebClient();
  });

  afterEach(() => {
    vi.restoreAllMocks();
  });

  describe('constructor', () => {
    it('subscribes socket.message$ to protobuf.handleMessageEvent', () => {
      const event = { data: new ArrayBuffer(0) } as MessageEvent;
      messageSubject.next(event);
      expect(client.protobuf.handleMessageEvent).toHaveBeenCalledWith(event);
    });

    it('calls SessionPersistence.initialized', () => {
      expect(SessionPersistence.initialized).toHaveBeenCalled();
    });
  });

  describe('connect', () => {
    it('calls SessionPersistence.connectionAttempted', () => {
      const opts: any = { host: 'h', port: 1 };
      client.connect(opts);
      expect(SessionPersistence.connectionAttempted).toHaveBeenCalled();
    });

    it('stores options and calls socket.connect', () => {
      const opts: any = { host: 'h', port: 1 };
      client.connect(opts);
      expect(client.options).toBe(opts);
      expect(client.socket.connect).toHaveBeenCalledWith(opts);
    });
  });

  describe('testConnect', () => {
    it('delegates to socket.testConnect', () => {
      const opts: any = { host: 'h', port: 1 };
      client.testConnect(opts);
      expect(client.socket.testConnect).toHaveBeenCalledWith(opts);
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

    it('calls protobuf.resetCommands and clears stores on DISCONNECTED', () => {
      client.updateStatus(StatusEnum.DISCONNECTED);
      expect(client.protobuf.resetCommands).toHaveBeenCalled();
      expect(RoomPersistence.clearStore).toHaveBeenCalled();
      expect(SessionPersistence.clearStore).toHaveBeenCalled();
    });

    it('does not clear stores when status is not DISCONNECTED', () => {
      client.updateStatus(StatusEnum.CONNECTED);
      expect(client.protobuf.resetCommands).not.toHaveBeenCalled();
      expect(RoomPersistence.clearStore).not.toHaveBeenCalled();
    });
  });

  describe('constructor closures', () => {
    it('keepAliveFn calls ping with the callback', () => {
      const cb = vi.fn();
      captured.wsOptions.keepAliveFn(cb);
      expect(ping).toHaveBeenCalledWith(cb);
    });

    it('send closure delegates to socket.send', () => {
      const data = new Uint8Array([1, 2, 3]);
      captured.pbOptions.send(data);
      expect(client.socket.send).toHaveBeenCalledWith(data);
    });

    it('isOpen closure delegates to socket.checkReadyState', () => {
      const result = captured.pbOptions.isOpen();
      expect(client.socket.checkReadyState).toHaveBeenCalledWith(WebSocket.OPEN);
      expect(result).toBe(true);
    });
  });
});
