import { StatusEnum, WebSocketConnectReason } from 'types';

import {
  AddToListData,
  ConnectionClosedData,
  ListRoomsData,
  RemoveFromListData,
  ServerIdentificationData,
  ServerMessageData,
  SessionEvents,
  UserJoinedData,
  UserLeftData,
} from './SessionEvents';

import { SessionCommands } from '../commands';
import { RoomPersistence, SessionPersistence } from '../persistence';
import webClient from '../WebClient';

describe('SessionEvents', () => {
  const roomId = 1;

  beforeEach(() => {
    jest.spyOn(SessionCommands, 'updateStatus').mockImplementation(() => {});
  });

  describe('.Event_AddToList.ext', () => {
    it('should call SessionPersistence.addToBuddyList if buddy listName', () => {
      jest.spyOn(SessionPersistence, 'addToBuddyList').mockImplementation(() => {});
      const data: AddToListData = { listName: 'buddy', userInfo: {} as any };

      SessionEvents['.Event_AddToList.ext'](data);

      expect(SessionPersistence.addToBuddyList).toHaveBeenCalledWith(
        data.userInfo
      );
    });

    it('should call SessionPersistence.addToIgnoreList if ignore listName', () => {
      jest.spyOn(SessionPersistence, 'addToIgnoreList').mockImplementation(() => {});
      const data: AddToListData = { listName: 'ignore', userInfo: {} as any };

      SessionEvents['.Event_AddToList.ext'](data);

      expect(SessionPersistence.addToIgnoreList).toHaveBeenCalledWith(
        data.userInfo
      );
    });

    it('should call console.log if unknown listName', () => {
      jest.spyOn(console, 'log').mockImplementation(() => {});
      const data: AddToListData = { listName: 'unknown', userInfo: {} as any };

      SessionEvents['.Event_AddToList.ext'](data);

      expect(console.log).toHaveBeenCalledWith(
        `Attempted to add to unknown list: ${data.listName}`
      );
    });
  });

  describe('.Event_ConnectionClosed.ext', () => {
    describe('with reasonStr', () => {
      it('should call SessionCommands.updateStatus', () => {
        const data: ConnectionClosedData = { endTime: 0, reason: 0, reasonStr: 'reasonStr' };

        SessionEvents['.Event_ConnectionClosed.ext'](data);

        expect(SessionCommands.updateStatus).toHaveBeenCalledWith(
          StatusEnum.DISCONNECTED,
          data.reasonStr
        );
      });
    });

    describe('without reasonStr', () => {
      beforeEach(() => {
        webClient.protobuf.controller.Event_ConnectionClosed = { CloseReason: {} };
      });

      describe('USER_LIMIT_REACHED', () => {
        it('should call SessionCommands.updateStatus', () => {
          const USER_LIMIT_REACHED = 1;
          webClient.protobuf.controller.Event_ConnectionClosed.CloseReason.USER_LIMIT_REACHED = USER_LIMIT_REACHED;
          const data: ConnectionClosedData = { endTime: 0, reason: USER_LIMIT_REACHED, reasonStr: null };

          SessionEvents['.Event_ConnectionClosed.ext'](data);

          expect(SessionCommands.updateStatus).toHaveBeenCalledWith(
            StatusEnum.DISCONNECTED,
            'The server has reached its maximum user capacity'
          );
        });
      });

      describe('TOO_MANY_CONNECTIONS', () => {
        it('should call SessionCommands.updateStatus', () => {
          const TOO_MANY_CONNECTIONS = 1;
          webClient.protobuf.controller.Event_ConnectionClosed.CloseReason.TOO_MANY_CONNECTIONS = TOO_MANY_CONNECTIONS;
          const data: ConnectionClosedData = { endTime: 0, reason: TOO_MANY_CONNECTIONS, reasonStr: null };

          SessionEvents['.Event_ConnectionClosed.ext'](data);

          expect(SessionCommands.updateStatus).toHaveBeenCalledWith(
            StatusEnum.DISCONNECTED,
            'There are too many concurrent connections from your address'
          );
        });
      });

      describe('BANNED', () => {
        it('should call SessionCommands.updateStatus', () => {
          const BANNED = 1;
          webClient.protobuf.controller.Event_ConnectionClosed.CloseReason.BANNED = BANNED;
          const data: ConnectionClosedData = { endTime: 0, reason: BANNED, reasonStr: null };

          SessionEvents['.Event_ConnectionClosed.ext'](data);

          expect(SessionCommands.updateStatus).toHaveBeenCalledWith(
            StatusEnum.DISCONNECTED,
            'You are banned'
          );
        });
      });

      describe('DEMOTED', () => {
        it('should call SessionCommands.updateStatus', () => {
          const DEMOTED = 1;
          webClient.protobuf.controller.Event_ConnectionClosed.CloseReason.DEMOTED = DEMOTED;
          const data: ConnectionClosedData = { endTime: 0, reason: DEMOTED, reasonStr: null };

          SessionEvents['.Event_ConnectionClosed.ext'](data);

          expect(SessionCommands.updateStatus).toHaveBeenCalledWith(
            StatusEnum.DISCONNECTED,
            'You were demoted'
          );
        });
      });

      describe('SERVER_SHUTDOWN', () => {
        it('should call SessionCommands.updateStatus', () => {
          const SERVER_SHUTDOWN = 1;
          webClient.protobuf.controller.Event_ConnectionClosed.CloseReason.SERVER_SHUTDOWN = SERVER_SHUTDOWN;
          const data: ConnectionClosedData = { endTime: 0, reason: SERVER_SHUTDOWN, reasonStr: null };

          SessionEvents['.Event_ConnectionClosed.ext'](data);

          expect(SessionCommands.updateStatus).toHaveBeenCalledWith(
            StatusEnum.DISCONNECTED,
            'Scheduled server shutdown'
          );
        });
      });

      describe('USERNAMEINVALID', () => {
        it('should call SessionCommands.updateStatus', () => {
          const USERNAMEINVALID = 1;
          webClient.protobuf.controller.Event_ConnectionClosed.CloseReason.USERNAMEINVALID = USERNAMEINVALID;
          const data: ConnectionClosedData = { endTime: 0, reason: USERNAMEINVALID, reasonStr: null };

          SessionEvents['.Event_ConnectionClosed.ext'](data);

          expect(SessionCommands.updateStatus).toHaveBeenCalledWith(
            StatusEnum.DISCONNECTED,
            'Invalid username'
          );
        });
      });

      describe('LOGGEDINELSEWERE', () => {
        it('should call SessionCommands.updateStatus', () => {
          const LOGGEDINELSEWERE = 1;
          webClient.protobuf.controller.Event_ConnectionClosed.CloseReason.LOGGEDINELSEWERE = LOGGEDINELSEWERE;
          const data: ConnectionClosedData = { endTime: 0, reason: LOGGEDINELSEWERE, reasonStr: null };

          SessionEvents['.Event_ConnectionClosed.ext'](data);

          expect(SessionCommands.updateStatus).toHaveBeenCalledWith(
            StatusEnum.DISCONNECTED,
            'You have been logged out due to logging in at another location'
          );
        });
      });

      describe('OTHER', () => {
        it('should call SessionCommands.updateStatus', () => {
          const OTHER = 1;
          webClient.protobuf.controller.Event_ConnectionClosed.CloseReason.OTHER = OTHER;
          const data: ConnectionClosedData = { endTime: 0, reason: OTHER, reasonStr: null };

          SessionEvents['.Event_ConnectionClosed.ext'](data);

          expect(SessionCommands.updateStatus).toHaveBeenCalledWith(
            StatusEnum.DISCONNECTED,
            'Unknown reason'
          );
        });
      });

      describe('UNKNOWN', () => {
        it('should call SessionCommands.updateStatus', () => {
          const UNKNOWN = 1;
          webClient.protobuf.controller.Event_ConnectionClosed.CloseReason.UNKNOWN = UNKNOWN;
          const data: ConnectionClosedData = { endTime: 0, reason: UNKNOWN, reasonStr: null };

          SessionEvents['.Event_ConnectionClosed.ext'](data);

          expect(SessionCommands.updateStatus).toHaveBeenCalledWith(
            StatusEnum.DISCONNECTED,
            'Unknown reason'
          );
        });
      });
    });
  });

  describe('.Event_ListRooms.ext', () => {
    beforeEach(() => {
      webClient.options.autojoinrooms = false;
      jest.spyOn(RoomPersistence, 'updateRooms').mockImplementation(() => {});
    });

    it('should call RoomPersistence.updateRooms', () => {
      const data: ListRoomsData = { roomList: [{ roomId, autoJoin: false } as any] };

      SessionEvents['.Event_ListRooms.ext'](data);

      expect(RoomPersistence.updateRooms).toHaveBeenCalledWith(data.roomList);
    });

    it('should call SessionCommands.joinRoom if webClient and room is configured for autojoin', () => {
      webClient.options.autojoinrooms = true;
      jest.spyOn(SessionCommands, 'joinRoom').mockImplementation(() => {});
      const data: ListRoomsData = { roomList: [{ roomId, autoJoin: true } as any, { roomId: 2, autoJoin: false } as any] };

      SessionEvents['.Event_ListRooms.ext'](data);

      expect(SessionCommands.joinRoom).toHaveBeenCalledTimes(1);
      expect(SessionCommands.joinRoom).toHaveBeenCalledWith(data.roomList[0].roomId);
    });
  });

  describe('.Event_RemoveFromList.ext', () => {
    it('should call SessionPersistence.removeFromBuddyList if buddy listName', () => {
      jest.spyOn(SessionPersistence, 'removeFromBuddyList').mockImplementation(() => {});
      const data: RemoveFromListData = { listName: 'buddy', userName: '' };

      SessionEvents['.Event_RemoveFromList.ext'](data);

      expect(SessionPersistence.removeFromBuddyList).toHaveBeenCalledWith(
        data.userName
      );
    });

    it('should call SessionPersistence.removeFromIgnoreList if ignore listName', () => {
      jest.spyOn(SessionPersistence, 'removeFromIgnoreList').mockImplementation(() => {});
      const data: RemoveFromListData = { listName: 'ignore', userName: '' };

      SessionEvents['.Event_RemoveFromList.ext'](data);

      expect(SessionPersistence.removeFromIgnoreList).toHaveBeenCalledWith(
        data.userName
      );
    });

    it('should call console.log if unknown listName', () => {
      jest.spyOn(console, 'log').mockImplementation(() => {});
      const data: RemoveFromListData = { listName: 'unknown', userName: '' };

      SessionEvents['.Event_RemoveFromList.ext'](data);

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
      event = SessionEvents['.Event_ServerIdentification.ext'];
      data = {
        serverName: 'serverName',
        serverVersion: 'serverVersion',
        protocolVersion: 0,
        serverOptions: 0
      };

      jest.spyOn(SessionPersistence, 'updateInfo').mockImplementation(() => {});
      webClient.protobuf.controller.Event_ServerIdentification = { ServerOptions: { SupportsPasswordHash: 1 } };
    });

    it('update status/info and login', () => {
      jest.spyOn(SessionCommands, 'login').mockImplementation(() => {});

      webClient.options.reason = WebSocketConnectReason.LOGIN;

      event(data);

      expect(SessionPersistence.updateInfo).toHaveBeenCalledWith(data.serverName, data.serverVersion);
      expect(SessionCommands.updateStatus).toHaveBeenCalledWith(StatusEnum.LOGGING_IN, expect.any(String));
      expect(SessionCommands.login).toHaveBeenCalled();
    });

    it('should update stat/info and register', () => {
      jest.spyOn(SessionCommands, 'register').mockImplementation(() => {});

      webClient.options.reason = WebSocketConnectReason.REGISTER;

      event(data);

      expect(SessionPersistence.updateInfo).toHaveBeenCalledWith(data.serverName, data.serverVersion);
      expect(SessionCommands.register).toHaveBeenCalled();
    });

    it('should update stat/info and activate account', () => {
      jest.spyOn(SessionCommands, 'activateAccount').mockImplementation(() => {});

      webClient.options.reason = WebSocketConnectReason.ACTIVATE_ACCOUNT;

      event(data);

      expect(SessionPersistence.updateInfo).toHaveBeenCalledWith(data.serverName, data.serverVersion);
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
        serverOptions: 0
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
      jest.spyOn(SessionPersistence, 'serverMessage').mockImplementation(() => {});
      const data: ServerMessageData = { message: 'message' };

      SessionEvents['.Event_ServerMessage.ext'](data);

      expect(SessionPersistence.serverMessage).toHaveBeenCalledWith(
        data.message
      );
    });
  });

  describe('.Event_UserJoined.ext', () => {
    it('should call SessionPersistence.userJoined', () => {
      jest.spyOn(SessionPersistence, 'userJoined').mockImplementation(() => {});
      const data: UserJoinedData = { userInfo: {} as any };

      SessionEvents['.Event_UserJoined.ext'](data);

      expect(SessionPersistence.userJoined).toHaveBeenCalledWith(
        data.userInfo
      );
    });
  });

  describe('.Event_UserLeft.ext', () => {
    it('should call SessionPersistence.userLeft', () => {
      jest.spyOn(SessionPersistence, 'userLeft').mockImplementation(() => {});
      const data: UserLeftData = { name: '' };

      SessionEvents['.Event_UserLeft.ext'](data);

      expect(SessionPersistence.userLeft).toHaveBeenCalledWith(
        data.name
      );
    });
  });
});
