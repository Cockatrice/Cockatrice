import { RoomCommands } from './RoomCommands';

import { RoomPersistence } from '../persistence';
import webClient from '../WebClient';

describe('RoomCommands', () => {
  const roomId = 1;
  let sendRoomCommandSpy;

  beforeEach(() => {
    sendRoomCommandSpy = spyOn(webClient.protobuf, 'sendRoomCommand');

    webClient.protobuf.controller.RoomCommand = { create: args => args };
  });

  describe('roomSay', () => {
    beforeEach(() => {
      webClient.protobuf.controller.Command_RoomSay = { create: args => args };
    });

    it('should call protobuf controller methods and sendCommand', () => {
      const message = ' message ';

      RoomCommands.roomSay(roomId, message);

      expect(webClient.protobuf.sendRoomCommand).toHaveBeenCalled();
      expect(webClient.protobuf.sendRoomCommand).toHaveBeenCalledWith(roomId, {
        '.Command_RoomSay.ext': { message: message.trim() }
      });
    });

    it('should not call sendRoomCommand if trimmed message is empty', () => {
      const message = ' ';

      RoomCommands.roomSay(roomId, message);

      expect(webClient.protobuf.sendRoomCommand).not.toHaveBeenCalled();
    });
  });

  describe('leaveRoom', () => {
    beforeEach(() => {
      webClient.protobuf.controller.Command_LeaveRoom = { create: () => ({}) };
    });

    it('should call protobuf controller methods and sendCommand', () => {
      RoomCommands.leaveRoom(roomId);

      expect(webClient.protobuf.sendRoomCommand).toHaveBeenCalled();
      expect(webClient.protobuf.sendRoomCommand).toHaveBeenCalledWith(
        roomId,
        { '.Command_LeaveRoom.ext': {} },
        jasmine.any(Function)
      );
    });

    it('should call RoomPersistence.leaveRoom if RespOk', () => {
      const RespOk = 'ok';
      webClient.protobuf.controller.Response = { ResponseCode: { RespOk } };
      sendRoomCommandSpy.and.callFake((_, __, callback) => {
        callback({ responseCode: RespOk })
      });

      spyOn(RoomPersistence, 'leaveRoom');

      RoomCommands.leaveRoom(roomId);

      expect(RoomPersistence.leaveRoom).toHaveBeenCalledWith(roomId);
    });
  });
});
