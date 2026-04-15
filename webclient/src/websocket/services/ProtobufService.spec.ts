vi.mock('@bufbuild/protobuf', () => ({
  create: vi.fn((_schema: unknown, fields?: Record<string, unknown>) => ({ ...(fields ?? {}) })),
  fromBinary: vi.fn(),
  toBinary: vi.fn().mockReturnValue(new Uint8Array()),
  hasExtension: vi.fn().mockReturnValue(false),
  getExtension: vi.fn(),
  setExtension: vi.fn(),
}));

vi.mock('../../generated/proto/commands_pb', () => ({
  CommandContainerSchema: {},
}));

vi.mock('../../generated/proto/server_message_pb', () => ({
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

import { create, fromBinary, hasExtension, getExtension } from '@bufbuild/protobuf';
import type { GenExtension } from '@bufbuild/protobuf/codegenv2';

import { ProtobufService } from './ProtobufService';
import { GameEvents, RoomEvents, SessionEvents } from '../events';
import type { GameExtensionRegistry } from '../events/game';
import type { RoomExtensionRegistry } from '../events/room';
import type { SessionExtensionRegistry } from '../events/session';

import { Data } from '@app/types';

type ProtobufInternal = ProtobufService & {
  cmdId: number;
  pendingCommands: Map<number, (response: Data.Response) => void>;
  processGameEvent(container: unknown, extra?: unknown): void;
  processRoomEvent(event: unknown): void;
  processSessionEvent(event: unknown): void;
  processServerResponse(response: unknown): void;
};

let mockSocket: { isOpen: ReturnType<typeof vi.fn>; send: ReturnType<typeof vi.fn> };

beforeEach(() => {
  mockSocket = {
    isOpen: vi.fn().mockReturnValue(true),
    send: vi.fn(),
  };
});

describe('ProtobufService', () => {
  // Mock extensions for send*Command tests — @bufbuild/protobuf is fully mocked so these are never invoked
  const sessionExt = {} as GenExtension<Data.SessionCommand, Record<string, never>>;
  const roomExt = {} as GenExtension<Data.RoomCommand, Record<string, never>>;
  const gameExt = {} as GenExtension<Data.GameCommand, Record<string, never>>;
  const moderatorExt = {} as GenExtension<Data.ModeratorCommand, Record<string, never>>;
  const adminExt = {} as GenExtension<Data.AdminCommand, Record<string, never>>;

  describe('resetCommands', () => {
    it('resets cmdId and pendingCommands', () => {
      const service = new ProtobufService(mockSocket);
      service.sendSessionCommand(sessionExt, vi.fn());
      expect((service as ProtobufInternal).cmdId).toBe(1);
      service.resetCommands();
      expect((service as ProtobufInternal).cmdId).toBe(0);
      expect((service as ProtobufInternal).pendingCommands).toEqual(new Map());
    });
  });

  describe('sendCommand', () => {
    it('increments cmdId and stores callback', () => {
      const service = new ProtobufService(mockSocket);
      const cb = vi.fn();
      service.sendCommand(create(Data.CommandContainerSchema), cb);
      expect((service as ProtobufInternal).cmdId).toBe(1);
      expect((service as ProtobufInternal).pendingCommands.get(1)).toBe(cb);
    });

    it('sends encoded data when socket is OPEN', () => {
      const service = new ProtobufService(mockSocket);
      mockSocket.isOpen.mockReturnValue(true);
      service.sendCommand(create(Data.CommandContainerSchema), vi.fn());
      expect(mockSocket.send).toHaveBeenCalled();
    });

    it('does not send when socket is not OPEN', () => {
      const service = new ProtobufService(mockSocket);
      mockSocket.isOpen.mockReturnValue(false);
      service.sendCommand(create(Data.CommandContainerSchema), vi.fn());
      expect(mockSocket.send).not.toHaveBeenCalled();
    });
  });

  describe('sendSessionCommand', () => {
    it('stores callback and increments cmdId', () => {
      const service = new ProtobufService(mockSocket);
      service.sendSessionCommand(sessionExt, {});
      expect((service as ProtobufInternal).cmdId).toBe(1);
      expect((service as ProtobufInternal).pendingCommands.get(1)).toBeTypeOf('function');
    });

    it('invokes onResponse with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      const cb = vi.fn();
      service.sendSessionCommand(sessionExt, {}, { onResponse: cb });

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      storedCb(create(Data.ResponseSchema));

      expect(cb).toHaveBeenCalledWith(create(Data.ResponseSchema));
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      service.sendSessionCommand(sessionExt, {});

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      expect(() => storedCb(create(Data.ResponseSchema))).not.toThrow();
    });
  });

  describe('sendRoomCommand', () => {
    it('stores callback and increments cmdId', () => {
      const service = new ProtobufService(mockSocket);
      service.sendRoomCommand(42, roomExt, {});
      expect((service as ProtobufInternal).cmdId).toBe(1);
    });

    it('invokes onResponse with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      const cb = vi.fn();
      service.sendRoomCommand(42, roomExt, {}, { onResponse: cb });

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      storedCb(create(Data.ResponseSchema));

      expect(cb).toHaveBeenCalledWith(create(Data.ResponseSchema));
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      service.sendRoomCommand(42, roomExt, {});

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      expect(() => storedCb(create(Data.ResponseSchema))).not.toThrow();
    });
  });

  describe('sendGameCommand', () => {
    it('stores callback and increments cmdId', () => {
      const service = new ProtobufService(mockSocket);
      service.sendGameCommand(7, gameExt, {});
      expect((service as ProtobufInternal).cmdId).toBe(1);
    });

    it('invokes onResponse with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      const cb = vi.fn();
      service.sendGameCommand(7, gameExt, {}, { onResponse: cb });

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      storedCb(create(Data.ResponseSchema));

      expect(cb).toHaveBeenCalledWith(create(Data.ResponseSchema));
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      service.sendGameCommand(7, gameExt, {});

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      expect(() => storedCb(create(Data.ResponseSchema))).not.toThrow();
    });
  });

  describe('sendModeratorCommand', () => {
    it('stores callback and increments cmdId', () => {
      const service = new ProtobufService(mockSocket);
      service.sendModeratorCommand(moderatorExt, {});
      expect((service as ProtobufInternal).cmdId).toBe(1);
    });

    it('invokes onResponse with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      const cb = vi.fn();
      service.sendModeratorCommand(moderatorExt, {}, { onResponse: cb });

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      storedCb(create(Data.ResponseSchema));

      expect(cb).toHaveBeenCalledWith(create(Data.ResponseSchema));
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      service.sendModeratorCommand(moderatorExt, {});

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      expect(() => storedCb(create(Data.ResponseSchema))).not.toThrow();
    });
  });

  describe('sendAdminCommand', () => {
    it('stores callback and increments cmdId', () => {
      const service = new ProtobufService(mockSocket);
      service.sendAdminCommand(adminExt, {});
      expect((service as ProtobufInternal).cmdId).toBe(1);
    });

    it('invokes onResponse with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      const cb = vi.fn();
      service.sendAdminCommand(adminExt, {}, { onResponse: cb });

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      storedCb(create(Data.ResponseSchema));

      expect(cb).toHaveBeenCalledWith(create(Data.ResponseSchema));
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockSocket);
      service.sendAdminCommand(adminExt, {});

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      expect(() => storedCb(create(Data.ResponseSchema))).not.toThrow();
    });
  });

  describe('handleMessageEvent', () => {
    it('routes RESPONSE message to processServerResponse', () => {
      const service = new ProtobufService(mockSocket);
      const cb = vi.fn();
      (service as ProtobufInternal).cmdId = 1;
      (service as ProtobufInternal).pendingCommands.set(1, cb);

      vi.mocked(fromBinary).mockReturnValue(
        create(Data.ServerMessageSchema, {
          messageType: Data.ServerMessage_MessageType.RESPONSE,
          response: create(Data.ResponseSchema, { cmdId: BigInt(1) }),
        })
      );

      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(cb).toHaveBeenCalledWith(expect.objectContaining({ cmdId: BigInt(1) }));
      expect((service as ProtobufInternal).pendingCommands.get(1)).toBeUndefined();
    });

    it('routes ROOM_EVENT message', () => {
      const service = new ProtobufService(mockSocket);
      const processRoomEvent = vi.spyOn(service as ProtobufInternal, 'processRoomEvent');

      vi.mocked(fromBinary).mockReturnValue(
        create(Data.ServerMessageSchema, {
          messageType: Data.ServerMessage_MessageType.ROOM_EVENT,
        })
      );

      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(processRoomEvent).toHaveBeenCalled();
    });

    it('routes SESSION_EVENT message', () => {
      const service = new ProtobufService(mockSocket);
      const processSessionEvent = vi.spyOn(service as ProtobufInternal, 'processSessionEvent');

      vi.mocked(fromBinary).mockReturnValue(
        create(Data.ServerMessageSchema, {
          messageType: Data.ServerMessage_MessageType.SESSION_EVENT,
        })
      );

      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(processSessionEvent).toHaveBeenCalled();
    });

    it('routes GAME_EVENT_CONTAINER message', () => {
      const service = new ProtobufService(mockSocket);
      const processGameEvent = vi.spyOn(service as ProtobufInternal, 'processGameEvent');

      vi.mocked(fromBinary).mockReturnValue(
        create(Data.ServerMessageSchema, {
          messageType: Data.ServerMessage_MessageType.GAME_EVENT_CONTAINER,
        })
      );

      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(processGameEvent).toHaveBeenCalled();
    });

    it('logs unknown message types (default case)', () => {
      const service = new ProtobufService(mockSocket);
      const consoleSpy = vi.spyOn(console, 'log').mockImplementation(() => {});

      vi.mocked(fromBinary).mockReturnValue(
        create(Data.ServerMessageSchema, {
          messageType: 999,
        })
      );

      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(consoleSpy).toHaveBeenCalled();
      consoleSpy.mockRestore();
    });

    it('does nothing when decoded message is null', () => {
      const service = new ProtobufService(mockSocket);
      vi.mocked(fromBinary).mockReturnValue(null!);
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
      (service as ProtobufInternal).processGameEvent(null, {});
      expect(hasExtension).not.toHaveBeenCalled();
    });

    it('dispatches to a GameEvents handler when hasExtension returns true', () => {
      const service = new ProtobufService(mockSocket);
      const handler = vi.fn();
      const mockExt = {} as GenExtension<Data.GameEvent, unknown>;
      const payload = { someData: 1 };

      // Temporarily override GameEvents for this test
      (GameEvents as GameExtensionRegistry).push([mockExt, handler]);
      vi.mocked(hasExtension).mockReturnValue(true);
      vi.mocked(getExtension).mockReturnValue(payload);

      (service as ProtobufInternal).processGameEvent({
        gameId: 42,
        eventList: [{ playerId: 5 }],
      }, {});

      expect(handler).toHaveBeenCalledWith(payload, expect.objectContaining({ gameId: 42, playerId: 5 }));
      (GameEvents as GameExtensionRegistry).pop();
    });

    it('defaults gameId and playerId to -1 when undefined', () => {
      const service = new ProtobufService(mockSocket);
      const handler = vi.fn();
      const mockExt = {} as GenExtension<Data.GameEvent, unknown>;
      const payload = { someData: 1 };

      (GameEvents as GameExtensionRegistry).push([mockExt, handler]);
      vi.mocked(hasExtension).mockReturnValue(true);
      vi.mocked(getExtension).mockReturnValue(payload);

      (service as ProtobufInternal).processGameEvent({
        gameId: undefined,
        eventList: [{ playerId: undefined }],
      });

      expect(handler).toHaveBeenCalledWith(payload, expect.objectContaining({ gameId: -1, playerId: -1 }));
      (GameEvents as GameExtensionRegistry).pop();
    });
  });

  describe('processServerResponse', () => {
    it('returns early when response is undefined', () => {
      const service = new ProtobufService(mockSocket);
      (service as ProtobufInternal).pendingCommands.set(1, vi.fn());
      (service as ProtobufInternal).processServerResponse(undefined);
      expect((service as ProtobufInternal).pendingCommands.size).toBe(1);
    });
  });

  describe('processRoomEvent', () => {
    it('returns early when event is undefined', () => {
      const service = new ProtobufService(mockSocket);
      vi.mocked(hasExtension).mockReturnValue(false);
      (service as ProtobufInternal).processRoomEvent(undefined);
      expect(hasExtension).not.toHaveBeenCalled();
    });

    it('dispatches to a RoomEvents handler when hasExtension returns true', () => {
      const service = new ProtobufService(mockSocket);
      const handler = vi.fn();
      const mockExt = {} as GenExtension<Data.RoomEvent, unknown>;
      const payload = { roomData: 1 };

      (RoomEvents as RoomExtensionRegistry).push([mockExt, handler]);
      vi.mocked(hasExtension).mockReturnValue(true);
      vi.mocked(getExtension).mockReturnValue(payload);

      const event = { roomId: 10 };
      (service as ProtobufInternal).processRoomEvent(event);

      expect(handler).toHaveBeenCalledWith(payload, event);
      (RoomEvents as RoomExtensionRegistry).pop();
    });
  });

  describe('processSessionEvent', () => {
    it('returns early when event is undefined', () => {
      const service = new ProtobufService(mockSocket);
      vi.mocked(hasExtension).mockReturnValue(false);
      (service as ProtobufInternal).processSessionEvent(undefined);
      expect(hasExtension).not.toHaveBeenCalled();
    });

    it('dispatches to a SessionEvents handler when hasExtension returns true', () => {
      const service = new ProtobufService(mockSocket);
      const handler = vi.fn();
      const mockExt = {} as GenExtension<Data.SessionEvent, unknown>;
      const payload = { sessionData: 1 };

      (SessionEvents as SessionExtensionRegistry).push([mockExt, handler]);
      vi.mocked(hasExtension).mockReturnValue(true);
      vi.mocked(getExtension).mockReturnValue(payload);

      (service as ProtobufInternal).processSessionEvent({ sessionId: 7 });

      expect(handler).toHaveBeenCalledWith(payload);
      (SessionEvents as SessionExtensionRegistry).pop();
    });
  });

});
