vi.mock('websocket', () => ({
  SessionCommands: {
    joinRoom: vi.fn(),
  },
  RoomCommands: {
    leaveRoom: vi.fn(),
    roomSay: vi.fn(),
  },
}));

import { RoomsService } from './RoomsService';
import { RoomCommands, SessionCommands } from 'websocket';

beforeEach(() => vi.clearAllMocks());

describe('RoomsService', () => {
  describe('joinRoom', () => {
    it('delegates to SessionCommands.joinRoom', () => {
      RoomsService.joinRoom(42);
      expect(SessionCommands.joinRoom).toHaveBeenCalledWith(42);
    });
  });

  describe('leaveRoom', () => {
    it('delegates to RoomCommands.leaveRoom', () => {
      RoomsService.leaveRoom(42);
      expect(RoomCommands.leaveRoom).toHaveBeenCalledWith(42);
    });
  });

  describe('roomSay', () => {
    it('delegates to RoomCommands.roomSay', () => {
      RoomsService.roomSay(42, 'hello room');
      expect(RoomCommands.roomSay).toHaveBeenCalledWith(42, 'hello room');
    });
  });
});
