import { makeMockProtoRoot } from '../__mocks__/helpers';

jest.mock('./ProtoController', () => ({
  ProtoController: { root: null, load: jest.fn() },
}));

jest.mock('../commands/session', () => ({
  SessionCommands: { ping: jest.fn() },
  ping: jest.fn(),
}));

jest.mock('../events', () => ({
  GameEvents: { '.Event_Game.ext': jest.fn() },
  RoomEvents: { '.Event_Room.ext': jest.fn() },
  SessionEvents: { '.Event_Session.ext': jest.fn() },
}));

jest.mock('../WebClient');

import { ProtobufService } from './ProtobufService';
import { ProtoController } from './ProtoController';
import { ping as sessionPing } from '../commands/session';
import { GameEvents } from '../events';

let mockSocket: any;
let mockWebClient: any;

beforeEach(() => {
  jest.clearAllMocks();

  ProtoController.root = makeMockProtoRoot();
  const encodeResult = { finish: jest.fn().mockReturnValue(new Uint8Array([1, 2])) };
  ProtoController.root.CommandContainer.encode = jest.fn().mockReturnValue(encodeResult);

  mockSocket = {
    checkReadyState: jest.fn().mockReturnValue(true),
    send: jest.fn(),
  };

  mockWebClient = {
    socket: mockSocket,
  };
});

describe('ProtobufService', () => {
  it('calls ProtoController.load on construction', () => {
    new ProtobufService(mockWebClient);
    expect(ProtoController.load).toHaveBeenCalled();
  });

  describe('resetCommands', () => {
    it('resets cmdId and pendingCommands', () => {
      const service = new ProtobufService(mockWebClient);
      // add a pending command
      service.sendSessionCommand({}, jest.fn());
      expect((service as any).cmdId).toBe(1);
      service.resetCommands();
      expect((service as any).cmdId).toBe(0);
      expect((service as any).pendingCommands).toEqual({});
    });
  });

  describe('sendCommand', () => {
    it('increments cmdId and stores callback', () => {
      const service = new ProtobufService(mockWebClient);
      const cb = jest.fn();
      service.sendCommand({}, cb);
      expect((service as any).cmdId).toBe(1);
      expect((service as any).pendingCommands[1]).toBe(cb);
    });

    it('sends encoded data when socket is OPEN', () => {
      const service = new ProtobufService(mockWebClient);
      mockSocket.checkReadyState.mockReturnValue(true);
      service.sendCommand({}, jest.fn());
      expect(mockSocket.send).toHaveBeenCalled();
    });

    it('does not send when socket is not OPEN', () => {
      const service = new ProtobufService(mockWebClient);
      mockSocket.checkReadyState.mockReturnValue(false);
      service.sendCommand({}, jest.fn());
      expect(mockSocket.send).not.toHaveBeenCalled();
    });
  });

  describe('sendSessionCommand', () => {
    it('creates a CommandContainer and calls sendCommand', () => {
      const service = new ProtobufService(mockWebClient);
      const cb = jest.fn();
      service.sendSessionCommand({ cmdType: 'test' }, cb);
      expect(ProtoController.root.CommandContainer.create).toHaveBeenCalledWith(
        expect.objectContaining({ sessionCommand: expect.anything() })
      );
    });

    it('invokes callback with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockWebClient);
      const cb = jest.fn();
      service.sendSessionCommand({ cmdType: 'test' }, cb);

      const storedCb = (service as any).pendingCommands[1];
      storedCb({ responseData: true });

      expect(cb).toHaveBeenCalledWith({ responseData: true });
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockWebClient);
      service.sendSessionCommand({ cmdType: 'test' });

      const storedCb = (service as any).pendingCommands[1];
      expect(() => storedCb({ responseData: true })).not.toThrow();
    });
  });

  describe('sendRoomCommand', () => {
    it('creates a CommandContainer with roomId and calls sendCommand', () => {
      const service = new ProtobufService(mockWebClient);
      service.sendRoomCommand(42, { roomCmdType: 'test' }, jest.fn());
      expect(ProtoController.root.CommandContainer.create).toHaveBeenCalledWith(
        expect.objectContaining({ roomId: 42 })
      );
    });

    it('invokes callback with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockWebClient);
      const cb = jest.fn();
      service.sendRoomCommand(42, { roomCmdType: 'test' }, cb);

      const storedCb = (service as any).pendingCommands[1];
      storedCb({ responseData: true });

      expect(cb).toHaveBeenCalledWith({ responseData: true });
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockWebClient);
      service.sendRoomCommand(42, { roomCmdType: 'test' });

      const storedCb = (service as any).pendingCommands[1];
      expect(() => storedCb({ responseData: true })).not.toThrow();
    });
  });

  describe('sendGameCommand', () => {
    it('creates a CommandContainer with gameId and gameCommand', () => {
      const service = new ProtobufService(mockWebClient);
      service.sendGameCommand(7, { gameCmdType: 'test' }, jest.fn());
      expect(ProtoController.root.CommandContainer.create).toHaveBeenCalledWith(
        expect.objectContaining({ gameId: 7, gameCommand: expect.anything() })
      );
    });

    it('invokes callback with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockWebClient);
      const cb = jest.fn();
      service.sendGameCommand(7, { gameCmdType: 'test' }, cb);

      const storedCb = (service as any).pendingCommands[1];
      storedCb({ responseData: true });

      expect(cb).toHaveBeenCalledWith({ responseData: true });
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockWebClient);
      service.sendGameCommand(7, { gameCmdType: 'test' });

      const storedCb = (service as any).pendingCommands[1];
      expect(() => storedCb({ responseData: true })).not.toThrow();
    });
  });

  describe('sendModeratorCommand', () => {
    it('creates a CommandContainer with moderatorCommand', () => {
      const service = new ProtobufService(mockWebClient);
      service.sendModeratorCommand({ modCmdType: 'test' }, jest.fn());
      expect(ProtoController.root.CommandContainer.create).toHaveBeenCalledWith(
        expect.objectContaining({ moderatorCommand: expect.anything() })
      );
    });

    it('invokes callback with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockWebClient);
      const cb = jest.fn();
      service.sendModeratorCommand({ modCmdType: 'test' }, cb);

      const storedCb = (service as any).pendingCommands[1];
      storedCb({ responseData: true });

      expect(cb).toHaveBeenCalledWith({ responseData: true });
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockWebClient);
      service.sendModeratorCommand({ modCmdType: 'test' });

      const storedCb = (service as any).pendingCommands[1];
      expect(() => storedCb({ responseData: true })).not.toThrow();
    });
  });

  describe('sendAdminCommand', () => {
    it('creates a CommandContainer with adminCommand', () => {
      const service = new ProtobufService(mockWebClient);
      service.sendAdminCommand({ adminCmdType: 'test' }, jest.fn());
      expect(ProtoController.root.CommandContainer.create).toHaveBeenCalledWith(
        expect.objectContaining({ adminCommand: expect.anything() })
      );
    });

    it('invokes callback with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockWebClient);
      const cb = jest.fn();
      service.sendAdminCommand({ adminCmdType: 'test' }, cb);

      const storedCb = (service as any).pendingCommands[1];
      storedCb({ responseData: true });

      expect(cb).toHaveBeenCalledWith({ responseData: true });
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockWebClient);
      service.sendAdminCommand({ adminCmdType: 'test' });

      const storedCb = (service as any).pendingCommands[1];
      expect(() => storedCb({ responseData: true })).not.toThrow();
    });
  });

  describe('sendKeepAliveCommand', () => {
    it('delegates to SessionCommands.ping', () => {
      const service = new ProtobufService(mockWebClient);
      const pingReceived = jest.fn();
      service.sendKeepAliveCommand(pingReceived);
      expect(sessionPing).toHaveBeenCalledWith(pingReceived);
    });
  });

  describe('handleMessageEvent', () => {
    it('routes RESPONSE message to processServerResponse', () => {
      const service = new ProtobufService(mockWebClient);
      const cb = jest.fn();
      // store a callback for cmdId 1
      (service as any).cmdId = 1;
      (service as any).pendingCommands[1] = cb;

      const response = { cmdId: 1 };
      ProtoController.root.ServerMessage.decode = jest.fn().mockReturnValue({
        messageType: ProtoController.root.ServerMessage.MessageType.RESPONSE,
        response,
      });

      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(cb).toHaveBeenCalledWith(response);
      expect((service as any).pendingCommands[1]).toBeUndefined();
    });

    it('resolves pending command when response cmdId is a protobufjs Long object', () => {
      const service = new ProtobufService(mockWebClient);
      const cb = jest.fn();
      (service as any).cmdId = 1;
      (service as any).pendingCommands[1] = cb;

      // Simulate protobufjs decoding cmdId as a Long object (low=1, high=0)
      const longCmdId = { low: 1, high: 0, unsigned: false, toString: () => '1' };
      const response = { cmdId: longCmdId };
      ProtoController.root.ServerMessage.decode = jest.fn().mockReturnValue({
        messageType: ProtoController.root.ServerMessage.MessageType.RESPONSE,
        response,
      });

      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(cb).toHaveBeenCalledWith(response);
      expect((service as any).pendingCommands[1]).toBeUndefined();
    });

    it('routes ROOM_EVENT message', () => {
      const service = new ProtobufService(mockWebClient);
      const processRoomEvent = jest.spyOn(service as any, 'processRoomEvent');
      ProtoController.root.ServerMessage.decode = jest.fn().mockReturnValue({
        messageType: ProtoController.root.ServerMessage.MessageType.ROOM_EVENT,
        roomEvent: { '.Event_Room.ext': {} },
      });
      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(processRoomEvent).toHaveBeenCalled();
    });

    it('routes SESSION_EVENT message', () => {
      const service = new ProtobufService(mockWebClient);
      const processSessionEvent = jest.spyOn(service as any, 'processSessionEvent');
      ProtoController.root.ServerMessage.decode = jest.fn().mockReturnValue({
        messageType: ProtoController.root.ServerMessage.MessageType.SESSION_EVENT,
        sessionEvent: { '.Event_Session.ext': {} },
      });
      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(processSessionEvent).toHaveBeenCalled();
    });

    it('routes GAME_EVENT_CONTAINER message', () => {
      const service = new ProtobufService(mockWebClient);
      const processGameEvent = jest.spyOn(service as any, 'processGameEvent');
      ProtoController.root.ServerMessage.decode = jest.fn().mockReturnValue({
        messageType: ProtoController.root.ServerMessage.MessageType.GAME_EVENT_CONTAINER,
        gameEvent: { '.Event_Game.ext': {} },
      });
      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(processGameEvent).toHaveBeenCalled();
    });

    it('logs unknown message types (default case)', () => {
      const service = new ProtobufService(mockWebClient);
      const consoleSpy = jest.spyOn(console, 'log').mockImplementation(() => {});
      ProtoController.root.ServerMessage.decode = jest.fn().mockReturnValue({
        messageType: 'UNKNOWN_TYPE',
      });
      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(consoleSpy).toHaveBeenCalled();
      consoleSpy.mockRestore();
    });

    it('does nothing when decoded message is null', () => {
      const service = new ProtobufService(mockWebClient);
      ProtoController.root.ServerMessage.decode = jest.fn().mockReturnValue(null);
      expect(() => service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent)).not.toThrow();
    });

    it('catches and logs decode errors', () => {
      const service = new ProtobufService(mockWebClient);
      const consoleSpy = jest.spyOn(console, 'error').mockImplementation(() => {});
      ProtoController.root.ServerMessage.decode = jest.fn().mockImplementation(() => {
        throw new Error('decode error');
      });
      expect(() => service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent)).not.toThrow();
      expect(consoleSpy).toHaveBeenCalled();
      consoleSpy.mockRestore();
    });
  });

  describe('processGameEvent', () => {
    it('returns early when container has no eventList', () => {
      const service = new ProtobufService(mockWebClient);
      const gameEventHandler = (GameEvents as any)['.Event_Game.ext'] as jest.Mock;
      (service as any).processGameEvent(null, {});
      expect(gameEventHandler).not.toHaveBeenCalled();
    });

    it('dispatches to a GameEvents handler when event key matches', () => {
      const service = new ProtobufService(mockWebClient);
      const gameEventHandler = (GameEvents as any)['.Event_Game.ext'] as jest.Mock;
      const payload = { someData: 1 };
      (service as any).processGameEvent({
        gameId: 42,
        context: null,
        secondsElapsed: 10,
        forcedByJudge: 0,
        eventList: [{ '.Event_Game.ext': payload, playerId: 5 }],
      }, {});
      expect(gameEventHandler).toHaveBeenCalledWith(payload, expect.objectContaining({ gameId: 42, playerId: 5 }));
    });

  });

  describe('processEvent', () => {
    it('calls matching event handler with payload and raw', () => {
      const service = new ProtobufService(mockWebClient);
      const handler = jest.fn();
      const events = { '.Event_Test.ext': handler };
      const payload = { someData: 1 };
      const response = { '.Event_Test.ext': payload };
      const raw = { extra: true };

      (service as any).processEvent(response, events, raw);

      expect(handler).toHaveBeenCalledWith(payload, raw);
    });

    it('stops after first matching event', () => {
      const service = new ProtobufService(mockWebClient);
      const handler1 = jest.fn();
      const handler2 = jest.fn();
      const events = { '.Event_A.ext': handler1, '.Event_B.ext': handler2 };
      const response = { '.Event_A.ext': { x: 1 } };

      (service as any).processEvent(response, events, {});

      expect(handler1).toHaveBeenCalled();
      expect(handler2).not.toHaveBeenCalled();
    });
  });
});
