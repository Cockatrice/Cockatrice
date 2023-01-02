import {
  Event_ConnectionClosed,
  Event_Leave,
  Event_LeaveRoom,
  Event_UserLeft,
  IEvent_AddToList,
  IEvent_ConnectionClosed,
  SessionEvent,
} from 'protoFiles';
import { StatusEnum, WebSocketConnectReason } from 'types';

import { SessionCommands } from '../commands';
import { RoomPersistence, SessionPersistence } from '../persistence';
import webClient from '../WebClient';
import { connectionClosed, messageMap, SessionHandlers } from './SessionEvents';

describe('SessionHandlers', () => {
  const roomId = 1;

  beforeEach(() => {
    jest.spyOn(SessionCommands, 'updateStatus').mockImplementation(() => {});
  });

  describe('.Event_AddToList.ext', () => {
    it('should call SessionPersistence.addToBuddyList if buddy listName', () => {
      jest
        .spyOn(SessionPersistence, 'addToBuddyList')
        .mockImplementation(() => {});
      const data: IEvent_AddToList = { listName: 'buddy', userInfo: {} as any };
      SessionHandlers['.Event_AddToList.ext']?.(data);

      expect(SessionPersistence.addToBuddyList).toHaveBeenCalledWith(
        data.userInfo
      );
    });

    it('should call SessionPersistence.addToIgnoreList if ignore listName', () => {
      jest
        .spyOn(SessionPersistence, 'addToIgnoreList')
        .mockImplementation(() => {});
      const data: IEvent_AddToList = {
        listName: 'ignore',
        userInfo: {} as any,
      };

      SessionHandlers['.Event_AddToList.ext']?.(data);

      expect(SessionPersistence.addToIgnoreList).toHaveBeenCalledWith(
        data.userInfo
      );
    });

    it('should call console.log if unknown listName', () => {
      jest.spyOn(console, 'log').mockImplementation(() => {});
      const data: IEvent_AddToList = {
        listName: 'unknown',
        userInfo: {} as any,
      };

      SessionHandlers['.Event_AddToList.ext']?.(data);

      expect(console.log).toHaveBeenCalledWith(
        `Attempted to add to unknown list: ${data.listName}`
      );
    });
  });

  describe('.Event_ConnectionClosed.ext', () => {
    describe('with reasonStr', () => {
      it('should call SessionCommands.updateStatus', () => {
        const data: IEvent_ConnectionClosed = {
          endTime: 0,
          reason: 0,
          reasonStr: 'reasonStr',
        };

        SessionHandlers['.Event_ConnectionClosed.ext']?.(data);

        expect(SessionCommands.updateStatus).toHaveBeenCalledWith(
          StatusEnum.DISCONNECTED,
          data.reasonStr
        );
      });
    });

    describe('without reasonStr', () => {
      const spy = jest.spyOn(SessionCommands, 'updateStatus');
      spy.mockImplementation();
      it('should call a message for every reason', () => {
        Object.entries(Event_ConnectionClosed.CloseReason).forEach(
          ([name, val]) => {
            const data: IEvent_ConnectionClosed = {
              reason: val,
              reasonStr: null,
            };
            connectionClosed(data);
            expect(spy).toBeCalledWith(
              StatusEnum.DISCONNECTED,
              expect.stringContaining('')
            );
          }
        );
      });

      beforeEach(() => {
        webClient.clientOptions.autojoinrooms = false;
        jest.spyOn(RoomPersistence, 'updateRooms').mockImplementation(() => {});
      });

      it('should call RoomPersistence.updateRooms', () => {
        const data: ListRoomsData = {
          roomList: [{ roomId, autoJoin: false } as any],
        };

        SessionHandlers['.Event_ListRooms.ext'](data);

        expect(RoomPersistence.updateRooms).toHaveBeenCalledWith(data.roomList);
      });

      it('should call SessionCommands.joinRoom if webClient and room is configured for autojoin', () => {
        webClient.clientOptions.autojoinrooms = true;
        jest.spyOn(SessionCommands, 'joinRoom').mockImplementation(() => {});
        const data: ListRoomsData = {
          roomList: [
            { roomId, autoJoin: true } as any,
            { roomId: 2, autoJoin: false } as any,
          ],
        };

        SessionHandlers['.Event_ListRooms.ext'](data);

        expect(SessionCommands.joinRoom).toHaveBeenCalledTimes(1);
        expect(SessionCommands.joinRoom).toHaveBeenCalledWith(
          data.roomList[0].roomId
        );
      });
    });

    describe('.Event_RemoveFromList.ext', () => {
      it('should call SessionPersistence.removeFromBuddyList if buddy listName', () => {
        jest
          .spyOn(SessionPersistence, 'removeFromBuddyList')
          .mockImplementation(() => {});
        const data: RemoveFromListData = { listName: 'buddy', userName: '' };

        SessionHandlers['.Event_RemoveFromList.ext'](data);

        expect(SessionPersistence.removeFromBuddyList).toHaveBeenCalledWith(
          data.userName
        );
      });

      it('should call SessionPersistence.removeFromIgnoreList if ignore listName', () => {
        jest
          .spyOn(SessionPersistence, 'removeFromIgnoreList')
          .mockImplementation(() => {});
        const data: RemoveFromListData = { listName: 'ignore', userName: '' };

        SessionHandlers['.Event_RemoveFromList.ext'](data);

        expect(SessionPersistence.removeFromIgnoreList).toHaveBeenCalledWith(
          data.userName
        );
      });

      it('should call console.log if unknown listName', () => {
        jest.spyOn(console, 'log').mockImplementation(() => {});
        const data: RemoveFromListData = { listName: 'unknown', userName: '' };

        SessionHandlers['.Event_RemoveFromList.ext'](data);

        expect(console.log).toHaveBeenCalledWith(
          `Attempted to remove from unknown list: ${data.listName}`
        );
      });
    });

    describe('.Event_ServerIdentification.ext', () => {
      let data: ServerIdentificationData;
      let event;

      beforeEach(() => {
        webClient.protocolVersion = 0;
        event = SessionHandlers['.Event_ServerIdentification.ext'];
        data = {
          serverName: 'serverName',
          serverVersion: 'serverVersion',
          protocolVersion: 0,
          serverOptions: 0,
        };

        jest
          .spyOn(SessionPersistence, 'updateInfo')
          .mockImplementation(() => {});
        webClient.protobuf.controller.Event_ServerIdentification = {
          ServerOptions: { SupportsPasswordHash: 1 },
        };
        webClient.options = {};
      });

      it('update status/info and login', () => {
        jest.spyOn(SessionCommands, 'login').mockImplementation(() => {});

        webClient.options.reason = WebSocketConnectReason.LOGIN;

        event(data);

        expect(SessionPersistence.updateInfo).toHaveBeenCalledWith(
          data.serverName,
          data.serverVersion
        );
        expect(SessionCommands.updateStatus).toHaveBeenCalledWith(
          StatusEnum.LOGGING_IN,
          expect.any(String)
        );
        expect(SessionCommands.login).toHaveBeenCalled();
      });

      it('should update stat/info and register', () => {
        jest.spyOn(SessionCommands, 'register').mockImplementation(() => {});

        webClient.options.reason = WebSocketConnectReason.REGISTER;

        event(data);

        expect(SessionPersistence.updateInfo).toHaveBeenCalledWith(
          data.serverName,
          data.serverVersion
        );
        expect(SessionCommands.register).toHaveBeenCalled();
      });

      it('should update stat/info and activate account', () => {
        jest
          .spyOn(SessionCommands, 'activateAccount')
          .mockImplementation(() => {});

        webClient.options.reason = WebSocketConnectReason.ACTIVATE_ACCOUNT;

        event(data);

        expect(SessionPersistence.updateInfo).toHaveBeenCalledWith(
          data.serverName,
          data.serverVersion
        );
        expect(SessionCommands.activateAccount).toHaveBeenCalled();
      });

      it('should disconnect if protocolVersion mismatched', () => {
        jest.spyOn(SessionCommands, 'login').mockImplementation(() => {});
        jest.spyOn(SessionCommands, 'disconnect').mockImplementation(() => {});

        webClient.protocolVersion = 0;
        const data: ServerIdentificationData = {
          serverName: '',
          serverVersion: '',
          protocolVersion: 1,
          serverOptions: 0,
        };

        event(data);

        expect(SessionCommands.disconnect).toHaveBeenCalled();
        expect(SessionCommands.updateStatus).toHaveBeenCalledWith(
          StatusEnum.DISCONNECTED,
          `Protocol version mismatch: ${data.protocolVersion}`
        );
        expect(SessionCommands.login).not.toHaveBeenCalled();
      });
    });

    describe('.Event_ServerMessage.ext', () => {
      it('should call SessionPersistence.serverMessage', () => {
        jest
          .spyOn(SessionPersistence, 'serverMessage')
          .mockImplementation(() => {});
        const data: ServerMessageData = { message: 'message' };

        SessionHandlers['.Event_ServerMessage.ext'](data);

        expect(SessionPersistence.serverMessage).toHaveBeenCalledWith(
          data.message
        );
      });
    });

    describe('.Event_UserJoined.ext', () => {
      it('should call SessionPersistence.userJoined', () => {
        jest
          .spyOn(SessionPersistence, 'userJoined')
          .mockImplementation(() => {});
        const data: UserJoinedData = { userInfo: {} as any };

        SessionHandlers['.Event_UserJoined.ext'](data);

        expect(SessionPersistence.userJoined).toHaveBeenCalledWith();
      });
    });

    describe('.Event_UserLeft.ext', () => {
      it('should call SessionPersistence.userLeft', () => {
        jest.spyOn(SessionPersistence, 'userLeft').mockImplementation(() => {});
        const data: UserLeftData = { name: '' };

        SessionHandlers['.Event_UserLeft.ext'](data);

        expect(SessionPersistence.userLeft).toHaveBeenCalledWith(data.name);
      });
    });
  });
});
