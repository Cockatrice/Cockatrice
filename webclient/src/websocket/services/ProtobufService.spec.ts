vi.mock('@bufbuild/protobuf', () => ({
  create: vi.fn((_schema: any, fields?: any) => ({ ...(fields ?? {}) })),
  fromBinary: vi.fn(),
  toBinary: vi.fn().mockReturnValue(new Uint8Array()),
  hasExtension: vi.fn().mockReturnValue(false),
  getExtension: vi.fn(),
  setExtension: vi.fn(),
}));

vi.mock('generated/proto/commands_pb', () => ({
  CommandContainerSchema: {},
}));

vi.mock('generated/proto/server_message_pb', () => ({
  ServerMessageSchema: {},
  ServerMessage_MessageType: {
    RESPONSE: 1,
    ROOM_EVENT: 2,
    SESSION_EVENT: 3,
    GAME_EVENT_CONTAINER: 4,
  },
}));

vi.mock('../events', () => ({
  GameEvents: [],
  RoomEvents: [],
  SessionEvents: [],
}));

vi.mock('../WebClient', () => ({
  __esModule: true,
  default: {},
}));

import { fromBinary, hasExtension, getExtension } from '@bufbuild/protobuf';
import { ServerMessage_MessageType } from 'generated/proto/server_message_pb';
import { ProtobufService } from './ProtobufService';
import { GameEvents, RoomEvents, SessionEvents } from '../events';

let mockSocket: any;

beforeEach(() => {
  vi.clearAllMocks();

  mockSocket = {
    isOpen: vi.fn().mockReturnValue(true),
    send: vi.fn(),
  };
});

describe('ProtobufService', () => {
  describe('resetCommands', () => {
    it('resets cmdId and pendingCommands', () => {
      const service = new ProtobufService(mockSocket);
      service.sendSessionCommand({} as any, vi.fn());
      expect((service as any).cmdId).toBe(1);
      service.resetCommands();
      expect((service as any).cmdId).toBe(0);
      expect((service as any).pendingCommands).toEqual(new Map());
    });
  });

  describe('sendCommand', () => {
    it('increments cmdId and stores callback', () => {
      const service = new ProtobufService(mockSocket);
      const cb = vi.fn();
      service.sendCommand({} as any, cb);
      expect((service as any).cmdId).toBe(1);
      expect((service as any).pendingCommands.get(1)).toBe(cb);
    });

    it('sends encoded data when socket is OPEN', () => {
      const service = new ProtobufService(mockSocket);
      mockSocket.isOpen.mockReturnValue(true);
      service.sendCommand({} as any, vi.fn());
      expect(mockSocket.send).toHaveBeenCalled();
    });

    it('does not send when socket is not OPEN', () => {
      const service = new ProtobufService(mockSocket);
      mockSocket.isOpen.mockReturnValue(false);
      service.sendCommand({} as any, vi.fn());
      expect(mockSocket.send).not.toHaveBeenCalled();
    });
  });

  describe('sendSessionCommand', () => {
    it('stores callback and increments cmdId', () => {
      const service = new ProtobufService(mockSocket);
      service.sendSessionCommand({} as any, {});
      expect((service as any).cmdId).toBe(1);
      expect((service as any).pendingCommands.get(1)).toBeTypeOf('function');
    });

    it('invokes onResponse with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      const cb = vi.fn();
      service.sendSessionCommand({} as any, {}, { onResponse: cb });

      const storedCb = (service as any).pendingCommands.get(1);
      storedCb({ responseData: true });

      expect(cb).toHaveBeenCalledWith({ responseData: true });
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      service.sendSessionCommand({} as any, {});

      const storedCb = (service as any).pendingCommands.get(1);
      expect(() => storedCb({ responseData: true })).not.toThrow();
    });
  });

  describe('sendRoomCommand', () => {
    it('stores callback and increments cmdId', () => {
      const service = new ProtobufService(mockSocket);
      service.sendRoomCommand(42, {} as any, {});
      expect((service as any).cmdId).toBe(1);
    });

    it('invokes onResponse with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      const cb = vi.fn();
      service.sendRoomCommand(42, {} as any, {}, { onResponse: cb });

      const storedCb = (service as any).pendingCommands.get(1);
      storedCb({ responseData: true });

      expect(cb).toHaveBeenCalledWith({ responseData: true });
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      service.sendRoomCommand(42, {} as any, {});

      const storedCb = (service as any).pendingCommands.get(1);
      expect(() => storedCb({ responseData: true })).not.toThrow();
    });
  });

  describe('sendGameCommand', () => {
    it('stores callback and increments cmdId', () => {
      const service = new ProtobufService(mockSocket);
      service.sendGameCommand(7, {} as any, {});
      expect((service as any).cmdId).toBe(1);
    });

    it('invokes onResponse with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      const cb = vi.fn();
      service.sendGameCommand(7, {} as any, {}, { onResponse: cb });

      const storedCb = (service as any).pendingCommands.get(1);
      storedCb({ responseData: true });

      expect(cb).toHaveBeenCalledWith({ responseData: true });
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      service.sendGameCommand(7, {} as any, {});

      const storedCb = (service as any).pendingCommands.get(1);
      expect(() => storedCb({ responseData: true })).not.toThrow();
    });
  });

  describe('sendModeratorCommand', () => {
    it('stores callback and increments cmdId', () => {
      const service = new ProtobufService(mockSocket);
      service.sendModeratorCommand({} as any, {});
      expect((service as any).cmdId).toBe(1);
    });

    it('invokes onResponse with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      const cb = vi.fn();
      service.sendModeratorCommand({} as any, {}, { onResponse: cb });

      const storedCb = (service as any).pendingCommands.get(1);
      storedCb({ responseData: true });

      expect(cb).toHaveBeenCalledWith({ responseData: true });
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      service.sendModeratorCommand({} as any, {});

      const storedCb = (service as any).pendingCommands.get(1);
      expect(() => storedCb({ responseData: true })).not.toThrow();
    });
  });

  describe('sendAdminCommand', () => {
    it('stores callback and increments cmdId', () => {
      const service = new ProtobufService(mockSocket);
      service.sendAdminCommand({} as any, {});
      expect((service as any).cmdId).toBe(1);
    });

    it('invokes onResponse with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      const cb = vi.fn();
      service.sendAdminCommand({} as any, {}, { onResponse: cb });

      const storedCb = (service as any).pendingCommands.get(1);
      storedCb({ responseData: true });

      expect(cb).toHaveBeenCalledWith({ responseData: true });
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      service.sendAdminCommand({} as any, {});

      const storedCb = (service as any).pendingCommands.get(1);
      expect(() => storedCb({ responseData: true })).not.toThrow();
    });
  });

  describe('handleMessageEvent', () => {
    it('routes RESPONSE message to processServerResponse', () => {
      const service = new ProtobufService(mockSocket);
      const cb = vi.fn();
      (service as any).cmdId = 1;
      (service as any).pendingCommands.set(1, cb);

      vi.mocked(fromBinary).mockReturnValue({
        messageType: ServerMessage_MessageType.RESPONSE,
        response: { cmdId: BigInt(1) },
      } as any);

      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(cb).toHaveBeenCalledWith({ cmdId: BigInt(1) });
      expect((service as any).pendingCommands.get(1)).toBeUndefined();
    });

    it('routes ROOM_EVENT message', () => {
      const service = new ProtobufService(mockSocket);
      const processRoomEvent = vi.spyOn(service as any, 'processRoomEvent');

      vi.mocked(fromBinary).mockReturnValue({
        messageType: ServerMessage_MessageType.ROOM_EVENT,
        roomEvent: {},
      } as any);

      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(processRoomEvent).toHaveBeenCalled();
    });

    it('routes SESSION_EVENT message', () => {
      const service = new ProtobufService(mockSocket);
      const processSessionEvent = vi.spyOn(service as any, 'processSessionEvent');

      vi.mocked(fromBinary).mockReturnValue({
        messageType: ServerMessage_MessageType.SESSION_EVENT,
        sessionEvent: {},
      } as any);

      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(processSessionEvent).toHaveBeenCalled();
    });

    it('routes GAME_EVENT_CONTAINER message', () => {
      const service = new ProtobufService(mockSocket);
      const processGameEvent = vi.spyOn(service as any, 'processGameEvent');

      vi.mocked(fromBinary).mockReturnValue({
        messageType: ServerMessage_MessageType.GAME_EVENT_CONTAINER,
        gameEventContainer: {},
      } as any);

      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(processGameEvent).toHaveBeenCalled();
    });

    it('logs unknown message types (default case)', () => {
      const service = new ProtobufService(mockSocket);
      const consoleSpy = vi.spyOn(console, 'log').mockImplementation(() => {});

      vi.mocked(fromBinary).mockReturnValue({
        messageType: 999,
      } as any);

      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(consoleSpy).toHaveBeenCalled();
      consoleSpy.mockRestore();
    });

    it('does nothing when decoded message is null', () => {
      const service = new ProtobufService(mockSocket);
      vi.mocked(fromBinary).mockReturnValue(null as any);
      expect(() => service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent)).not.toThrow();
    });

    it('catches and logs decode errors', () => {
      const service = new ProtobufService(mockSocket);
      const consoleSpy = vi.spyOn(console, 'error').mockImplementation(() => {});
      vi.mocked(fromBinary).mockImplementation(() => {
        throw new Error('decode error');
      });
      expect(() => service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent)).not.toThrow();
      expect(consoleSpy).toHaveBeenCalled();
      consoleSpy.mockRestore();
    });
  });

  describe('processGameEvent', () => {
    it('returns early when container has no eventList', () => {
      const service = new ProtobufService(mockSocket);
      vi.mocked(hasExtension).mockReturnValue(false);
      (service as any).processGameEvent(null, {});
      expect(hasExtension).not.toHaveBeenCalled();
    });

    it('dispatches to a GameEvents handler when hasExtension returns true', () => {
      const service = new ProtobufService(mockSocket);
      const handler = vi.fn();
      const mockExt = {};
      const payload = { someData: 1 };

      // Temporarily override GameEvents for this test
      (GameEvents as any).push([mockExt, handler]);
      vi.mocked(hasExtension).mockReturnValue(true);
      vi.mocked(getExtension).mockReturnValue(payload);

      (service as any).processGameEvent({
        gameId: 42,
        eventList: [{ playerId: 5 }],
      }, {});

      expect(handler).toHaveBeenCalledWith(payload, expect.objectContaining({ gameId: 42, playerId: 5 }));
      (GameEvents as any).pop();
    });

    it('defaults gameId and playerId to -1 when undefined', () => {
      const service = new ProtobufService(mockSocket);
      const handler = vi.fn();
      const mockExt = {};
      const payload = { someData: 1 };

      (GameEvents as any).push([mockExt, handler]);
      vi.mocked(hasExtension).mockReturnValue(true);
      vi.mocked(getExtension).mockReturnValue(payload);

      (service as any).processGameEvent({
        gameId: undefined,
        eventList: [{ playerId: undefined }],
      });

      expect(handler).toHaveBeenCalledWith(payload, expect.objectContaining({ gameId: -1, playerId: -1 }));
      (GameEvents as any).pop();
    });
  });

  describe('processServerResponse', () => {
    it('returns early when response is undefined', () => {
      const service = new ProtobufService(mockSocket);
      (service as any).pendingCommands.set(1, vi.fn());
      (service as any).processServerResponse(undefined);
      expect((service as any).pendingCommands.size).toBe(1);
    });
  });

  describe('processRoomEvent', () => {
    it('returns early when event is undefined', () => {
      const service = new ProtobufService(mockSocket);
      vi.mocked(hasExtension).mockReturnValue(false);
      (service as any).processRoomEvent(undefined);
      expect(hasExtension).not.toHaveBeenCalled();
    });

    it('dispatches to a RoomEvents handler when hasExtension returns true', () => {
      const service = new ProtobufService(mockSocket);
      const handler = vi.fn();
      const mockExt = {};
      const payload = { roomData: 1 };

      (RoomEvents as any).push([mockExt, handler]);
      vi.mocked(hasExtension).mockReturnValue(true);
      vi.mocked(getExtension).mockReturnValue(payload);

      const event = { roomId: 10 };
      (service as any).processRoomEvent(event);

      expect(handler).toHaveBeenCalledWith(payload, event);
      (RoomEvents as any).pop();
    });
  });

  describe('processSessionEvent', () => {
    it('returns early when event is undefined', () => {
      const service = new ProtobufService(mockSocket);
      vi.mocked(hasExtension).mockReturnValue(false);
      (service as any).processSessionEvent(undefined);
      expect(hasExtension).not.toHaveBeenCalled();
    });

    it('dispatches to a SessionEvents handler when hasExtension returns true', () => {
      const service = new ProtobufService(mockSocket);
      const handler = vi.fn();
      const mockExt = {};
      const payload = { sessionData: 1 };

      (SessionEvents as any).push([mockExt, handler]);
      vi.mocked(hasExtension).mockReturnValue(true);
      vi.mocked(getExtension).mockReturnValue(payload);

      (service as any).processSessionEvent({ sessionId: 7 });

      expect(handler).toHaveBeenCalledWith(payload);
      (SessionEvents as any).pop();
    });
  });

});
