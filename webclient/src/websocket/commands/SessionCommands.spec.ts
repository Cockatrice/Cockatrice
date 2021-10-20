import {StatusEnum} from 'types';

import {SessionCommands} from './SessionCommands';

import {RoomPersistence, SessionPersistence} from '../persistence';
import webClient from '../WebClient';
import {WebSocketConnectReason} from "../services/WebSocketService";

describe('SessionCommands', () => {
  const roomId = 1;
  let sendModeratorCommandSpy;
  let sendSessionCommandSpy;

  beforeEach(() => {
    spyOn(SessionCommands, 'updateStatus').and.callThrough();
    spyOn(webClient, 'updateStatus');
    spyOn(console, 'error');

    sendModeratorCommandSpy = spyOn(webClient.protobuf, 'sendModeratorCommand');
    sendSessionCommandSpy = spyOn(webClient.protobuf, 'sendSessionCommand');
    webClient.protobuf.controller.ModeratorCommand = { create: args => args };
    webClient.protobuf.controller.SessionCommand = { create: args => args };
  });

  describe('connect', () => {
    it('should call SessionCommands.updateStatus and webClient.connect', () => {
      spyOn(webClient, 'connect');
      const options: any = {
        host: 'host',
        port: 'port',
        user: 'user',
        pass: 'pass',
      };

      SessionCommands.connect(options, WebSocketConnectReason.CONNECT);

      expect(SessionCommands.updateStatus).toHaveBeenCalled();
      expect(SessionCommands.updateStatus).toHaveBeenCalledWith(StatusEnum.CONNECTING, 'Connecting...');
      
      expect(webClient.connect).toHaveBeenCalled();
      expect(webClient.connect).toHaveBeenCalledWith({ ...options, reason: WebSocketConnectReason.CONNECT });
    });
  });

  describe('disconnect', () => {
    it('should call SessionCommands.updateStatus and webClient.disconnect', () => {
      spyOn(webClient, 'disconnect');

      SessionCommands.disconnect();

      expect(SessionCommands.updateStatus).toHaveBeenCalled();
      expect(SessionCommands.updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTING, 'Disconnecting...');
      
      expect(webClient.disconnect).toHaveBeenCalled();
    });
  });

  describe('login', () => {
    beforeEach(() => {
      webClient.protobuf.controller.Command_Login = { create: args => args };
      webClient.options.user = 'user';
      webClient.options.pass = 'pass';
    });

    it('should call protobuf controller methods and sendCommand', () => {
      SessionCommands.login();

      expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalled();
      expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith({
        '.Command_Login.ext': {
          ...webClient.clientConfig,
          userName: webClient.options.user,
          password: webClient.options.pass,
          clientid: jasmine.any(String)
        }
      }, jasmine.any(Function));
    });

    describe('response', () => {
      const RespOk = 'RespOk';
      const respKey = '.Response_Login.ext';
      let response;

      beforeEach(() => {
        response = {
          responseCode: RespOk,
          [respKey]: {
            buddyList: [],
            ignoreList: [],
            userInfo: {}
          }
        };

        webClient.protobuf.controller.Response = { ResponseCode: { RespOk } };

        sendSessionCommandSpy.and.callFake((_, callback) => callback(response));
      });

      it('RespOk should update user/state and list users/games', () => {
        spyOn(SessionPersistence, 'updateBuddyList');
        spyOn(SessionPersistence, 'updateIgnoreList');
        spyOn(SessionPersistence, 'updateUser');
        spyOn(SessionCommands, 'listUsers');
        spyOn(SessionCommands, 'listRooms');

        SessionCommands.login();

        expect(SessionPersistence.updateBuddyList).toHaveBeenCalledWith(response[respKey].buddyList);
        expect(SessionPersistence.updateIgnoreList).toHaveBeenCalledWith(response[respKey].ignoreList);
        expect(SessionPersistence.updateUser).toHaveBeenCalledWith(response[respKey].userInfo);

        expect(SessionCommands.listUsers).toHaveBeenCalled();
        expect(SessionCommands.listRooms).toHaveBeenCalled();
        expect(SessionCommands.updateStatus).toHaveBeenCalledWith(StatusEnum.LOGGEDIN, 'Logged in.');
      });

      it('RespClientUpdateRequired should update status', () => {
        const RespClientUpdateRequired = 'RespClientUpdateRequired';
        webClient.protobuf.controller.Response.ResponseCode.RespClientUpdateRequired = RespClientUpdateRequired;
        response.responseCode = RespClientUpdateRequired;

        SessionCommands.login();

        expect(SessionCommands.updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, 'Login failed: missing features');
      });

      it('RespWrongPassword should update status', () => {
        const RespWrongPassword = 'RespWrongPassword';
        webClient.protobuf.controller.Response.ResponseCode.RespWrongPassword = RespWrongPassword;
        response.responseCode = RespWrongPassword;

        SessionCommands.login();

        expect(SessionCommands.updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, 'Login failed: incorrect username or password');
      });

      it('RespUsernameInvalid should update status', () => {
        const RespUsernameInvalid = 'RespUsernameInvalid';
        webClient.protobuf.controller.Response.ResponseCode.RespUsernameInvalid = RespUsernameInvalid;
        response.responseCode = RespUsernameInvalid;

        SessionCommands.login();

        expect(SessionCommands.updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, 'Login failed: incorrect username or password');
      });

      it('RespWouldOverwriteOldSession should update status', () => {
        const RespWouldOverwriteOldSession = 'RespWouldOverwriteOldSession';
        webClient.protobuf.controller.Response.ResponseCode.RespWouldOverwriteOldSession = RespWouldOverwriteOldSession;
        response.responseCode = RespWouldOverwriteOldSession;

        SessionCommands.login();

        expect(SessionCommands.updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, 'Login failed: duplicated user session');
      });

      it('RespUserIsBanned should update status', () => {
        const RespUserIsBanned = 'RespUserIsBanned';
        webClient.protobuf.controller.Response.ResponseCode.RespUserIsBanned = RespUserIsBanned;
        response.responseCode = RespUserIsBanned;

        SessionCommands.login();

        expect(SessionCommands.updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, 'Login failed: banned user');
      });

      it('RespRegistrationRequired should update status', () => {
        const RespRegistrationRequired = 'RespRegistrationRequired';
        webClient.protobuf.controller.Response.ResponseCode.RespRegistrationRequired = RespRegistrationRequired;
        response.responseCode = RespRegistrationRequired;

        SessionCommands.login();

        expect(SessionCommands.updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, 'Login failed: registration required');
      });

      it('RespClientIdRequired should update status', () => {
        const RespClientIdRequired = 'RespClientIdRequired';
        webClient.protobuf.controller.Response.ResponseCode.RespClientIdRequired = RespClientIdRequired;
        response.responseCode = RespClientIdRequired;

        SessionCommands.login();

        expect(SessionCommands.updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, 'Login failed: missing client ID');
      });

      it('RespContextError should update status', () => {
        const RespContextError = 'RespContextError';
        webClient.protobuf.controller.Response.ResponseCode.RespContextError = RespContextError;
        response.responseCode = RespContextError;

        SessionCommands.login();

        expect(SessionCommands.updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, 'Login failed: server error');
      });

      it('RespAccountNotActivated should update status', () => {
        const RespAccountNotActivated = 'RespAccountNotActivated';
        webClient.protobuf.controller.Response.ResponseCode.RespAccountNotActivated = RespAccountNotActivated;
        response.responseCode = RespAccountNotActivated;

        SessionCommands.login();

        expect(SessionCommands.updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, 'Login failed: account not activated');
      });

      it('all other responseCodes should update status', () => {
        const UnknownCode = 'UnknownCode';
        webClient.protobuf.controller.Response.ResponseCode.UnknownCode = UnknownCode;
        response.responseCode = UnknownCode;

        SessionCommands.login();

        expect(SessionCommands.updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, `Login failed: unknown error: ${response.responseCode}`);
      });
    });
  });

  describe('listUsers', () => {
    beforeEach(() => {
      webClient.protobuf.controller.Command_ListUsers = { create: () => ({}) };
    });

    it('should call protobuf controller methods and sendCommand', () => {
      SessionCommands.listUsers();

      expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalled();
      expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith({
        '.Command_ListUsers.ext': {}
      }, jasmine.any(Function));
    });

    it('should call SessionPersistence.updateUsers if RespOk', () => {
      const RespOk = 'ok';
      const respKey = '.Response_ListUsers.ext';
      const response = {
        responseCode: RespOk,
        [respKey]: { userList: [] }
      };

      webClient.protobuf.controller.Response = { ResponseCode: { RespOk } };
      sendSessionCommandSpy.and.callFake((_, callback) => callback(response));
      spyOn(SessionPersistence, 'updateUsers');

      SessionCommands.listUsers();

      expect(SessionPersistence.updateUsers).toHaveBeenCalledWith(response[respKey].userList);
    });
  });

  describe('listRooms', () => {
    beforeEach(() => {
      webClient.protobuf.controller.Command_ListRooms = { create: () => ({}) };
    });

    it('should call protobuf controller methods and sendCommand', () => {
      SessionCommands.listRooms();

      expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalled();
      expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith({
        '.Command_ListRooms.ext': {}
      });
    });
  });

  describe('joinRoom', () => {
    beforeEach(() => {
      webClient.protobuf.controller.Command_JoinRoom = { create: args => args };
    });

    it('should call protobuf controller methods and sendCommand', () => {
      SessionCommands.joinRoom(roomId);

      expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalled();
      expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith({
        '.Command_JoinRoom.ext': { roomId }
      }, jasmine.any(Function));
    });

    describe('response', () => {
      const RespOk = 'RespOk';
      const respKey = '.Response_JoinRoom.ext';
      let response;

      beforeEach(() => {
        response = {
          responseCode: RespOk,
          [respKey]: { roomInfo: {} }
        };

        webClient.protobuf.controller.Response = { ResponseCode: { RespOk } };

        sendSessionCommandSpy.and.callFake((_, callback) => callback(response));
      });

      it('RespOk should call RoomPersistence.joinRoom', () => {
        spyOn(RoomPersistence, 'joinRoom');

        SessionCommands.joinRoom(roomId);

        expect(RoomPersistence.joinRoom).toHaveBeenCalledWith(response[respKey].roomInfo);
      });

      it('RespNameNotFound should console error', () => {
        const RespNameNotFound = 'RespNameNotFound';
        webClient.protobuf.controller.Response.ResponseCode.RespNameNotFound = RespNameNotFound;
        response.responseCode = RespNameNotFound;

        SessionCommands.joinRoom(roomId);

        expect(console.error).toHaveBeenCalledWith(RespNameNotFound, 'Failed to join the room: it doesn\'t exist on the server.');
      });

      it('RespContextError should console error', () => {
        const RespContextError = 'RespContextError';
        webClient.protobuf.controller.Response.ResponseCode.RespContextError = RespContextError;
        response.responseCode = RespContextError;

        SessionCommands.joinRoom(roomId);

        expect(console.error).toHaveBeenCalledWith(RespContextError, 'The server thinks you are in the room but Cockatrice is unable to display it. Try restarting Cockatrice.');
      });

      it('RespUserLevelTooLow should console error', () => {
        const RespUserLevelTooLow = 'RespUserLevelTooLow';
        webClient.protobuf.controller.Response.ResponseCode.RespUserLevelTooLow = RespUserLevelTooLow;
        response.responseCode = RespUserLevelTooLow;

        SessionCommands.joinRoom(roomId);

        expect(console.error).toHaveBeenCalledWith(RespUserLevelTooLow, 'You do not have the required permission to join this room.');
      });

      it('all other responseCodes should update status', () => {
        const UnknownCode = 'UnknownCode';
        webClient.protobuf.controller.Response.ResponseCode.UnknownCode = UnknownCode;
        response.responseCode = UnknownCode;

        SessionCommands.joinRoom(roomId);

        expect(console.error).toHaveBeenCalledWith(UnknownCode, 'Failed to join the room due to an unknown error.');
      });
    });
  });

  describe('addToBuddyList', () => {
    it('should call SessionCommands.addToList', () => {
      spyOn(SessionCommands, 'addToList');
      const userName = 'userName';

      SessionCommands.addToBuddyList(userName);

      expect(SessionCommands.addToList).toHaveBeenCalledWith('buddy', userName);
    });
  });

  describe('removeFromBuddyList', () => {
    it('should call SessionCommands.removeFromList', () => {
      spyOn(SessionCommands, 'removeFromList');
      const userName = 'userName';

      SessionCommands.removeFromBuddyList(userName);

      expect(SessionCommands.removeFromList).toHaveBeenCalledWith('buddy', userName);
    });
  });

  describe('addToIgnoreList', () => {
    it('should call SessionCommands.addToList', () => {
      spyOn(SessionCommands, 'addToList');
      const userName = 'userName';

      SessionCommands.addToIgnoreList(userName);

      expect(SessionCommands.addToList).toHaveBeenCalledWith('ignore', userName);
    });
  });

  describe('removeFromIgnoreList', () => {
    it('should call SessionCommands.removeFromList', () => {
      spyOn(SessionCommands, 'removeFromList');
      const userName = 'userName';

      SessionCommands.removeFromIgnoreList(userName);

      expect(SessionCommands.removeFromList).toHaveBeenCalledWith('ignore', userName);
    });
  });

  describe('addToList', () => {
    beforeEach(() => {
      webClient.protobuf.controller.Command_AddToList = { create: args => args };
    });

    it('should call protobuf controller methods and sendCommand', () => {
      const addToList = { list: 'list', userName: 'userName'};
      SessionCommands.addToList(addToList.list, addToList.userName);

      expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalled();
      expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith({
        '.Command_AddToList.ext': addToList
      }, jasmine.any(Function));
    });
  });

  describe('removeFromList', () => {
    beforeEach(() => {
      webClient.protobuf.controller.Command_RemoveFromList = { create: args => args };
    });

    it('should call protobuf controller methods and sendCommand', () => {
      const removeFromList = { list: 'list', userName: 'userName'};
      SessionCommands.removeFromList(removeFromList.list, removeFromList.userName);

      expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalled();
      expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith({
        '.Command_RemoveFromList.ext': removeFromList
      }, jasmine.any(Function));
    });
  });

  describe('viewLogHistory', () => {
    const filters = {};

    beforeEach(() => {
      webClient.protobuf.controller.Command_ViewLogHistory = { create: args => args };
    });

    it('should call protobuf controller methods and sendCommand', () => {
      SessionCommands.viewLogHistory(filters);

      expect(webClient.protobuf.sendModeratorCommand).toHaveBeenCalled();
      expect(webClient.protobuf.sendModeratorCommand).toHaveBeenCalledWith({
        '.Command_ViewLogHistory.ext': filters
      }, jasmine.any(Function));
    });

    describe('response', () => {
      const RespOk = 'RespOk';
      const respKey = '.Response_ViewLogHistory.ext';
      let response;

      beforeEach(() => {
        response = {
          responseCode: RespOk,
          [respKey]: { logMessage: {} }
        };

        webClient.protobuf.controller.Response = { ResponseCode: { RespOk } };

        sendModeratorCommandSpy.and.callFake((_, callback) => callback(response));
      });

      it('RespOk should call SessionPersistence.viewLogs', () => {
        spyOn(SessionPersistence, 'viewLogs');

        SessionCommands.viewLogHistory(filters);

        expect(SessionPersistence.viewLogs).toHaveBeenCalledWith(response[respKey].logMessage);
      });

      it('all other responseCodes should console error', () => {
        const UnknownCode = 'UnknownCode';
        webClient.protobuf.controller.Response.ResponseCode.UnknownCode = UnknownCode;
        response.responseCode = UnknownCode;

        SessionCommands.viewLogHistory(filters);

        expect(console.error).toHaveBeenCalledWith(UnknownCode, 'Failed to retrieve log history.');
      });
    });
  });

  describe('updateStatus', () => {
    it('should call webClient.updateStatus', () => {
      SessionCommands.updateStatus(StatusEnum.CONNECTING, 'description');
      expect(webClient.updateStatus).toHaveBeenCalledWith(StatusEnum.CONNECTING, 'description');
    });
  });
});
