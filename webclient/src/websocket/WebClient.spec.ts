jest.mock('./services/WebSocketService', () => ({
  WebSocketService: jest.fn().mockImplementation(() => ({
    message$: { subscribe: jest.fn() },
    connect: jest.fn(),
    testConnect: jest.fn(),
    disconnect: jest.fn(),
  })),
}));

jest.mock('./services/ProtobufService', () => ({
  ProtobufService: jest.fn().mockImplementation(() => ({
    handleMessageEvent: jest.fn(),
    sendKeepAliveCommand: jest.fn(),
    resetCommands: jest.fn(),
  })),
}));

jest.mock('./persistence', () => ({
  RoomPersistence: { clearStore: jest.fn() },
  SessionPersistence: { clearStore: jest.fn() },
}));

import { WebClient } from './WebClient';
import { RoomPersistence, SessionPersistence } from './persistence';
import { StatusEnum } from 'types';
import { Subject } from 'rxjs';

describe('WebClient', () => {
  let client: WebClient;
  let messageSubject: Subject<MessageEvent>;

  beforeEach(() => {
    jest.clearAllMocks();
    const { ProtobufService } = require('./services/ProtobufService');
    ProtobufService.mockImplementation(() => ({
      handleMessageEvent: jest.fn(),
      sendKeepAliveCommand: jest.fn(),
      resetCommands: jest.fn(),
    }));
    messageSubject = new Subject<MessageEvent>();
    const { WebSocketService } = require('./services/WebSocketService');
    WebSocketService.mockImplementation(() => ({
      message$: messageSubject,
      connect: jest.fn(),
      testConnect: jest.fn(),
      disconnect: jest.fn(),
    }));
    // suppress console.log from constructor in non-test-env check
    jest.spyOn(console, 'log').mockImplementation(() => {});
    client = new WebClient();
  });

  afterEach(() => {
    jest.restoreAllMocks();
  });

  describe('constructor', () => {
    it('subscribes socket.message$ to protobuf.handleMessageEvent', () => {
      const event = { data: new ArrayBuffer(0) } as MessageEvent;
      messageSubject.next(event);
      expect(client.protobuf.handleMessageEvent).toHaveBeenCalledWith(event);
    });
  });

  describe('connect', () => {
    it('sets connectionAttemptMade to true', () => {
      const opts: any = { host: 'h', port: 1 };
      client.connect(opts);
      expect(client.connectionAttemptMade).toBe(true);
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

  describe('keepAlive', () => {
    it('delegates to protobuf.sendKeepAliveCommand', () => {
      const pingCb = jest.fn();
      client.keepAlive(pingCb);
      expect(client.protobuf.sendKeepAliveCommand).toHaveBeenCalledWith(pingCb);
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
});
