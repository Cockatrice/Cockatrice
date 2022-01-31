import { Room, StatusEnum, User, WebSocketConnectReason } from 'types';

import { SessionCommands } from '../commands';
import { RoomPersistence, SessionPersistence } from '../persistence';
import { ProtobufEvents } from '../services/ProtobufService';
import { passwordSaltSupported } from '../utils';
import webClient from '../WebClient';

export const SessionEvents: ProtobufEvents = {
  '.Event_AddToList.ext': addToList,
  '.Event_ConnectionClosed.ext': connectionClosed,
  '.Event_ListRooms.ext': listRooms,
  '.Event_NotifyUser.ext': notifyUser,
  '.Event_PlayerPropertiesChanges.ext': playerPropertiesChanges,
  '.Event_RemoveFromList.ext': removeFromList,
  '.Event_ServerIdentification.ext': serverIdentification,
  '.Event_ServerMessage.ext': serverMessage,
  '.Event_ServerShutdown.ext': serverShutdown,
  '.Event_UserJoined.ext': userJoined,
  '.Event_UserLeft.ext': userLeft,
  '.Event_UserMessage.ext': userMessage,
}

function addToList({ listName, userInfo }: AddToListData) {
  switch (listName) {
    case 'buddy': {
      SessionPersistence.addToBuddyList(userInfo);
      break;
    }
    case 'ignore': {
      SessionPersistence.addToIgnoreList(userInfo);
      break;
    }
    default: {
      console.log(`Attempted to add to unknown list: ${listName}`);
    }
  }
}

function connectionClosed({ reason, reasonStr }: ConnectionClosedData) {
  let message;

  // @TODO (5)
  if (reasonStr) {
    message = reasonStr;
  } else {
    switch (reason) {
      case webClient.protobuf.controller.Event_ConnectionClosed.CloseReason.USER_LIMIT_REACHED:
        message = 'The server has reached its maximum user capacity';
        break;
      case webClient.protobuf.controller.Event_ConnectionClosed.CloseReason.TOO_MANY_CONNECTIONS:
        message = 'There are too many concurrent connections from your address';
        break;
      case webClient.protobuf.controller.Event_ConnectionClosed.CloseReason.BANNED:
        message = 'You are banned';
        break;
      case webClient.protobuf.controller.Event_ConnectionClosed.CloseReason.DEMOTED:
        message = 'You were demoted';
        break;
      case webClient.protobuf.controller.Event_ConnectionClosed.CloseReason.SERVER_SHUTDOWN:
        message = 'Scheduled server shutdown';
        break;
      case webClient.protobuf.controller.Event_ConnectionClosed.CloseReason.USERNAMEINVALID:
        message = 'Invalid username';
        break;
      case webClient.protobuf.controller.Event_ConnectionClosed.CloseReason.LOGGEDINELSEWERE:
        message = 'You have been logged out due to logging in at another location';
        break;
      case webClient.protobuf.controller.Event_ConnectionClosed.CloseReason.OTHER:
      default:
        message = 'Unknown reason';
        break;
    }
  }

  SessionCommands.updateStatus(StatusEnum.DISCONNECTED, message);
}

function listRooms({ roomList }: ListRoomsData) {
  RoomPersistence.updateRooms(roomList);

  if (webClient.options.autojoinrooms) {
    roomList.forEach(({ autoJoin, roomId }) => {
      if (autoJoin) {
        SessionCommands.joinRoom(roomId);
      }
    });
  }
}

function notifyUser(payload) {
  // console.info('Event_NotifyUser', payload);
}

function playerPropertiesChanges(payload) {
  // console.info('Event_PlayerPropertiesChanges', payload);
}

function removeFromList({ listName, userName }: RemoveFromListData) {
  switch (listName) {
    case 'buddy': {
      SessionPersistence.removeFromBuddyList(userName);
      break;
    }
    case 'ignore': {
      SessionPersistence.removeFromIgnoreList(userName);
      break;
    }
    default: {
      console.log(`Attempted to remove from unknown list: ${listName}`);
    }
  }
}

function serverIdentification(info: ServerIdentificationData) {
  const { serverName, serverVersion, protocolVersion, serverOptions } = info;
  if (protocolVersion !== webClient.protocolVersion) {
    SessionCommands.updateStatus(StatusEnum.DISCONNECTED, `Protocol version mismatch: ${protocolVersion}`);
    SessionCommands.disconnect();
    return;
  }

  switch (webClient.options.reason) {
    case WebSocketConnectReason.LOGIN:
      SessionCommands.updateStatus(StatusEnum.LOGGING_IN, 'Logging In...');
      if (passwordSaltSupported(serverOptions, webClient)) {
        SessionCommands.requestPasswordSalt();
      } else {
        SessionCommands.login();
      }
      break;
    case WebSocketConnectReason.REGISTER:
      if (passwordSaltSupported(serverOptions, webClient)) {
        SessionCommands.requestPasswordSalt();
      } else {
        SessionCommands.register();
      }
      break;
    case WebSocketConnectReason.ACTIVATE_ACCOUNT:
      if (passwordSaltSupported(serverOptions, webClient)) {
        SessionCommands.requestPasswordSalt();
      } else {
        SessionCommands.activateAccount();
      }
      break;
    case WebSocketConnectReason.PASSWORD_RESET_REQUEST:
      SessionCommands.resetPasswordRequest();
      break;
    case WebSocketConnectReason.PASSWORD_RESET_CHALLENGE:
      SessionCommands.resetPasswordChallenge();
      break;
    case WebSocketConnectReason.PASSWORD_RESET:
      if (passwordSaltSupported(serverOptions, webClient)) {
        SessionCommands.requestPasswordSalt();
      } else {
        SessionCommands.resetPassword();
      }
      break;
    default:
      SessionCommands.updateStatus(StatusEnum.DISCONNECTED, 'Unknown Connection Reason: ' + webClient.options.reason);
      SessionCommands.disconnect();
      break;
  }

  SessionPersistence.updateInfo(serverName, serverVersion);
}

function serverMessage({ message }: ServerMessageData) {
  SessionPersistence.serverMessage(message);
}

function serverShutdown(payload) {
  // console.info('Event_ServerShutdown', payload);
}

function userJoined({ userInfo }: UserJoinedData) {
  SessionPersistence.userJoined(userInfo);
}

function userLeft({ name }: UserLeftData) {
  SessionPersistence.userLeft(name);
}

function userMessage(payload) {
  // console.info('Event_UserMessage', payload);
}

export interface SessionEvent {
  sessionEvent: {}
}

export interface AddToListData {
  listName: string;
  userInfo: User;
}

export interface ConnectionClosedData {
  endTime: number;
  reason: number;
  reasonStr: string;
}

export interface ListRoomsData {
  roomList: Room[];
}

export interface RemoveFromListData {
  listName: string;
  userName: string;
}

export interface ServerIdentificationData {
  protocolVersion: number;
  serverName: string;
  serverVersion: string;
  serverOptions: number;
}

export interface ServerMessageData {
  message: string;
}

export interface UserJoinedData {
  userInfo: User;
}

export interface UserLeftData {
  name: string;
}
