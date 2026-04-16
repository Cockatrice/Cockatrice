vi.mock('@bufbuild/protobuf', async (importOriginal) => ({
  ...(await importOriginal<typeof import('@bufbuild/protobuf')>()),
  fromBinary: vi.fn(),
  toBinary: vi.fn().mockReturnValue(new Uint8Array()),
  hasExtension: vi.fn().mockReturnValue(false),
  getExtension: vi.fn(),
  setExtension: vi.fn(),
}));

vi.mock('../WebClient', () => ({
  __esModule: true,
  default: {},
  WebClient: { _instance: null },
}));

import { useWebClientCleanup } from '../__mocks__/helpers';
import { create, fromBinary, hasExtension, getExtension } from '@bufbuild/protobuf';
import type { GenExtension } from '@bufbuild/protobuf/codegenv2';

import { ProtobufService } from './ProtobufService';
import type { EventRegistries } from './ProtobufService';

import type {
  AdminCommand,
  GameCommand,
  GameEvent,
  ModeratorCommand,
  Response,
  RoomCommand,
  RoomEvent,
  SessionCommand,
  SessionEvent,
} from '@app/generated';
import {
  CommandContainerSchema,
  ResponseSchema,
  ServerMessageSchema,
  ServerMessage_MessageType,
} from '@app/generated';

type ProtobufInternal = ProtobufService & {
  cmdId: number;
  pendingCommands: Map<number, (response: Response) => void>;
  processGameEvent(container: unknown, extra?: unknown): void;
  processRoomEvent(event: unknown): void;
  processSessionEvent(event: unknown): void;
  processServerResponse(response: unknown): void;
};

useWebClientCleanup();

let mockSocket: { isOpen: ReturnType<typeof vi.fn>; send: ReturnType<typeof vi.fn> };
let mockEvents: EventRegistries;

beforeEach(() => {
  mockSocket = {
    isOpen: vi.fn().mockReturnValue(true),
    send: vi.fn(),
  };
  mockEvents = {
    sessionEvents: [],
    roomEvents: [],
    gameEvents: [],
  };
});

describe('ProtobufService', () => {
  // Mock extensions for send*Command tests — @bufbuild/protobuf is fully mocked so these are never invoked
  const sessionExt = {} as GenExtension<SessionCommand, Record<string, never>>;
  const roomExt = {} as GenExtension<RoomCommand, Record<string, never>>;
  const gameExt = {} as GenExtension<GameCommand, Record<string, never>>;
  const moderatorExt = {} as GenExtension<ModeratorCommand, Record<string, never>>;
  const adminExt = {} as GenExtension<AdminCommand, Record<string, never>>;

  describe('resetCommands', () => {
    it('resets cmdId and pendingCommands', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      service.sendSessionCommand(sessionExt, vi.fn());
      expect((service as ProtobufInternal).cmdId).toBe(1);
      service.resetCommands();
      expect((service as ProtobufInternal).cmdId).toBe(0);
      expect((service as ProtobufInternal).pendingCommands).toEqual(new Map());
    });
  });

  describe('sendCommand', () => {
    it('increments cmdId and stores callback', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      const cb = vi.fn();
      service.sendCommand(create(CommandContainerSchema), cb);
      expect((service as ProtobufInternal).cmdId).toBe(1);
      expect((service as ProtobufInternal).pendingCommands.get(1)).toBe(cb);
    });

    it('sends encoded data when socket is OPEN', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      mockSocket.isOpen.mockReturnValue(true);
      service.sendCommand(create(CommandContainerSchema), vi.fn());
      expect(mockSocket.send).toHaveBeenCalled();
    });

    it('does not register callback or increment cmdId when transport is closed', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      mockSocket.isOpen.mockReturnValue(false);
      const cb = vi.fn();
      service.sendCommand(create(CommandContainerSchema), cb);
      expect(mockSocket.send).not.toHaveBeenCalled();
      expect((service as ProtobufInternal).cmdId).toBe(0);
      expect((service as ProtobufInternal).pendingCommands.size).toBe(0);
    });
  });

  describe('sendSessionCommand', () => {
    it('stores callback and increments cmdId', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      service.sendSessionCommand(sessionExt, {});
      expect((service as ProtobufInternal).cmdId).toBe(1);
      expect((service as ProtobufInternal).pendingCommands.get(1)).toBeTypeOf('function');
    });

    it('invokes onResponse with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      const cb = vi.fn();
      service.sendSessionCommand(sessionExt, {}, { onResponse: cb });

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      storedCb(create(ResponseSchema));

      expect(cb).toHaveBeenCalledWith(create(ResponseSchema));
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      service.sendSessionCommand(sessionExt, {});

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      expect(() => storedCb(create(ResponseSchema))).not.toThrow();
    });
  });

  describe('sendRoomCommand', () => {
    it('stores callback and increments cmdId', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      service.sendRoomCommand(42, roomExt, {});
      expect((service as ProtobufInternal).cmdId).toBe(1);
    });

    it('invokes onResponse with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      const cb = vi.fn();
      service.sendRoomCommand(42, roomExt, {}, { onResponse: cb });

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      storedCb(create(ResponseSchema));

      expect(cb).toHaveBeenCalledWith(create(ResponseSchema));
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      service.sendRoomCommand(42, roomExt, {});

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      expect(() => storedCb(create(ResponseSchema))).not.toThrow();
    });
  });

  describe('sendGameCommand', () => {
    it('stores callback and increments cmdId', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      service.sendGameCommand(7, gameExt, {});
      expect((service as ProtobufInternal).cmdId).toBe(1);
    });

    it('invokes onResponse with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      const cb = vi.fn();
      service.sendGameCommand(7, gameExt, {}, { onResponse: cb });

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      storedCb(create(ResponseSchema));

      expect(cb).toHaveBeenCalledWith(create(ResponseSchema));
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      service.sendGameCommand(7, gameExt, {});

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      expect(() => storedCb(create(ResponseSchema))).not.toThrow();
    });
  });

  describe('sendModeratorCommand', () => {
    it('stores callback and increments cmdId', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      service.sendModeratorCommand(moderatorExt, {});
      expect((service as ProtobufInternal).cmdId).toBe(1);
    });

    it('invokes onResponse with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      const cb = vi.fn();
      service.sendModeratorCommand(moderatorExt, {}, { onResponse: cb });

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      storedCb(create(ResponseSchema));

      expect(cb).toHaveBeenCalledWith(create(ResponseSchema));
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      service.sendModeratorCommand(moderatorExt, {});

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      expect(() => storedCb(create(ResponseSchema))).not.toThrow();
    });
  });

  describe('sendAdminCommand', () => {
    it('stores callback and increments cmdId', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      service.sendAdminCommand(adminExt, {});
      expect((service as ProtobufInternal).cmdId).toBe(1);
    });

    it('invokes onResponse with raw response when the pending command is triggered', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      const cb = vi.fn();
      service.sendAdminCommand(adminExt, {}, { onResponse: cb });

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      storedCb(create(ResponseSchema));

      expect(cb).toHaveBeenCalledWith(create(ResponseSchema));
    });

    it('does not throw when no callback is provided and pending command is triggered', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      service.sendAdminCommand(adminExt, {});

      const storedCb = (service as ProtobufInternal).pendingCommands.get(1)!;
      expect(() => storedCb(create(ResponseSchema))).not.toThrow();
    });
  });

  describe('handleMessageEvent', () => {
    it('routes RESPONSE message to processServerResponse', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      const cb = vi.fn();
      (service as ProtobufInternal).cmdId = 1;
      (service as ProtobufInternal).pendingCommands.set(1, cb);

      vi.mocked(fromBinary).mockReturnValue(
        create(ServerMessageSchema, {
          messageType: ServerMessage_MessageType.RESPONSE,
          response: create(ResponseSchema, { cmdId: BigInt(1) }),
        })
      );

      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(cb).toHaveBeenCalledWith(expect.objectContaining({ cmdId: BigInt(1) }));
      expect((service as ProtobufInternal).pendingCommands.get(1)).toBeUndefined();
    });

    it('routes ROOM_EVENT message', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      const processRoomEvent = vi.spyOn(service as ProtobufInternal, 'processRoomEvent');

      vi.mocked(fromBinary).mockReturnValue(
        create(ServerMessageSchema, {
          messageType: ServerMessage_MessageType.ROOM_EVENT,
        })
      );

      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(processRoomEvent).toHaveBeenCalled();
    });

    it('routes SESSION_EVENT message', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      const processSessionEvent = vi.spyOn(service as ProtobufInternal, 'processSessionEvent');

      vi.mocked(fromBinary).mockReturnValue(
        create(ServerMessageSchema, {
          messageType: ServerMessage_MessageType.SESSION_EVENT,
        })
      );

      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(processSessionEvent).toHaveBeenCalled();
    });

    it('routes GAME_EVENT_CONTAINER message', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      const processGameEvent = vi.spyOn(service as ProtobufInternal, 'processGameEvent');

      vi.mocked(fromBinary).mockReturnValue(
        create(ServerMessageSchema, {
          messageType: ServerMessage_MessageType.GAME_EVENT_CONTAINER,
        })
      );

      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(processGameEvent).toHaveBeenCalled();
    });

    it('logs unknown message types (default case)', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      const consoleSpy = vi.spyOn(console, 'log').mockImplementation(() => {});

      vi.mocked(fromBinary).mockReturnValue(
        create(ServerMessageSchema, {
          messageType: 999,
        })
      );

      service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent);
      expect(consoleSpy).toHaveBeenCalled();
      consoleSpy.mockRestore();
    });

    it('does nothing when decoded message is null', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      vi.mocked(fromBinary).mockReturnValue(null!);
      expect(() => service.handleMessageEvent({ data: new ArrayBuffer(0) } as MessageEvent)).not.toThrow();
    });

    it('catches and logs decode errors', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
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
      const service = new ProtobufService(mockSocket, mockEvents);
      vi.mocked(hasExtension).mockReturnValue(false);
      (service as ProtobufInternal).processGameEvent(null, {});
      expect(hasExtension).not.toHaveBeenCalled();
    });

    it('dispatches to a GameEvents handler when hasExtension returns true', () => {
      const handler = vi.fn();
      const mockExt = {} as GenExtension<GameEvent, unknown>;
      const payload = { someData: 1 };

      mockEvents.gameEvents.push([mockExt, handler] as any);
      const service = new ProtobufService(mockSocket, mockEvents);
      vi.mocked(hasExtension).mockReturnValue(true);
      vi.mocked(getExtension).mockReturnValue(payload);

      (service as ProtobufInternal).processGameEvent({
        gameId: 42,
        eventList: [{ playerId: 5 }],
      }, {});

      expect(handler).toHaveBeenCalledWith(payload, expect.objectContaining({ gameId: 42, playerId: 5 }));
    });

    it('defaults gameId and playerId to -1 when undefined', () => {
      const handler = vi.fn();
      const mockExt = {} as GenExtension<GameEvent, unknown>;
      const payload = { someData: 1 };

      mockEvents.gameEvents.push([mockExt, handler] as any);
      const service = new ProtobufService(mockSocket, mockEvents);
      vi.mocked(hasExtension).mockReturnValue(true);
      vi.mocked(getExtension).mockReturnValue(payload);

      (service as ProtobufInternal).processGameEvent({
        gameId: undefined,
        eventList: [{ playerId: undefined }],
      });

      expect(handler).toHaveBeenCalledWith(payload, expect.objectContaining({ gameId: -1, playerId: -1 }));
    });
  });

  describe('processServerResponse', () => {
    it('returns early when response is undefined', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      (service as ProtobufInternal).pendingCommands.set(1, vi.fn());
      (service as ProtobufInternal).processServerResponse(undefined);
      expect((service as ProtobufInternal).pendingCommands.size).toBe(1);
    });
  });

  describe('processRoomEvent', () => {
    it('returns early when event is undefined', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      vi.mocked(hasExtension).mockReturnValue(false);
      (service as ProtobufInternal).processRoomEvent(undefined);
      expect(hasExtension).not.toHaveBeenCalled();
    });

    it('dispatches to a RoomEvents handler when hasExtension returns true', () => {
      const handler = vi.fn();
      const mockExt = {} as GenExtension<RoomEvent, unknown>;
      const payload = { roomData: 1 };

      mockEvents.roomEvents.push([mockExt, handler] as any);
      const service = new ProtobufService(mockSocket, mockEvents);
      vi.mocked(hasExtension).mockReturnValue(true);
      vi.mocked(getExtension).mockReturnValue(payload);

      const event = { roomId: 10 };
      (service as ProtobufInternal).processRoomEvent(event);

      expect(handler).toHaveBeenCalledWith(payload, event);
    });
  });

  describe('processSessionEvent', () => {
    it('returns early when event is undefined', () => {
      const service = new ProtobufService(mockSocket, mockEvents);
      vi.mocked(hasExtension).mockReturnValue(false);
      (service as ProtobufInternal).processSessionEvent(undefined);
      expect(hasExtension).not.toHaveBeenCalled();
    });

    it('dispatches to a SessionEvents handler when hasExtension returns true', () => {
      const handler = vi.fn();
      const mockExt = {} as GenExtension<SessionEvent, unknown>;
      const payload = { sessionData: 1 };

      mockEvents.sessionEvents.push([mockExt, handler] as any);
      const service = new ProtobufService(mockSocket, mockEvents);
      vi.mocked(hasExtension).mockReturnValue(true);
      vi.mocked(getExtension).mockReturnValue(payload);

      (service as ProtobufInternal).processSessionEvent({ sessionId: 7 });

      expect(handler).toHaveBeenCalledWith(payload, undefined);
    });
  });

});
