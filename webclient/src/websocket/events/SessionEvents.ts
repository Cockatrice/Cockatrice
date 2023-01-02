import {
  Event_ConnectionClosed,
  IEvent_AddToList,
  IEvent_ConnectionClosed,
  IEvent_ListRooms,
  IEvent_RemoveFromList,
  IEvent_ServerIdentification,
  IEvent_ServerMessage,
  IEvent_UserJoined,
  IEvent_UserLeft,
} from 'protoFiles';
import { StatusEnum, WebSocketConnectReason } from 'types';
import type { Handlers } from 'websocket/services/ProtobufService';

import { SessionCommands } from '../commands';
import { RoomPersistence, SessionPersistence } from '../persistence';
import { generateSalt, passwordSaltSupported } from '../utils';
import webClient from '../WebClient';

export const SessionHandlers: Handlers = {
  '.Event_AddToList.ext': addToList,
  '.Event_ConnectionClosed.ext': connectionClosed,
  '.Event_ListRooms.ext': listRooms,
  '.Event_RemoveFromList.ext': removeFromList,
  '.Event_ServerIdentification.ext': serverIdentification,
  '.Event_ServerMessage.ext': serverMessage,
  '.Event_UserJoined.ext': userJoined,
  '.Event_UserLeft.ext': userLeft,
};

function addToList({ listName, userInfo }: IEvent_AddToList) {
  switch (listName) {
    case 'buddy':
      SessionPersistence.addToBuddyList(userInfo!);
      break;
    case 'ignore':
      SessionPersistence.addToIgnoreList(userInfo!);
      break;
    default:
      console.log(`Attempted to add to unknown list: ${listName}`);
  }
}

let r = Event_ConnectionClosed.CloseReason;
export const messageMap = {
  [r.USER_LIMIT_REACHED]: 'The server has reached its maximum user capacity',
  [r.TOO_MANY_CONNECTIONS]:
    'There are too many concurrent connections from your address',
  [r.BANNED]: 'You are banned',
  [r.DEMOTED]: 'You were demoted',
  [r.SERVER_SHUTDOWN]: 'Scheduled server shutdown',
  [r.USERNAMEINVALID]: 'Invalid username',
  [r.LOGGEDINELSEWERE]:
    'You have been logged out due to logging in at another location',
  [r.OTHER]: 'Unkown reason',
};
export function connectionClosed({ reason, reasonStr }: IEvent_ConnectionClosed) {
  let message: string
  // @TODO (5)
  if (reasonStr) {
    message = reasonStr;
  } else if (reason && reason in messageMap) {
    message = messageMap[reason];
  } else {
    message = 'Unknown reason';
  }
  SessionCommands.updateStatus(StatusEnum.DISCONNECTED, message);
}

function listRooms({ roomList }: IEvent_ListRooms) {
  if (!roomList) {
    throw new Error();
  }
  RoomPersistence.updateRooms(roomList);

  if (webClient.clientOptions.autojoinrooms) {
    roomList.forEach(({ autoJoin, roomId }) => {
      if (autoJoin) {
        SessionCommands.joinRoom(roomId!);
      }
    });
  }
}

function removeFromList({ listName, userName }: IEvent_RemoveFromList) {
  switch (listName) {
    case 'buddy': {
      SessionPersistence.removeFromBuddyList(userName!);
      break;
    }
    case 'ignore': {
      SessionPersistence.removeFromIgnoreList(userName!);
      break;
    }
    default: {
      console.log(`Attempted to remove from unknown list: ${listName}`);
    }
  }
}

function serverIdentification(info: IEvent_ServerIdentification) {
  const { serverName, serverVersion, protocolVersion, serverOptions } = info;
  if (protocolVersion !== webClient.protocolVersion) {
    SessionCommands.updateStatus(
      StatusEnum.DISCONNECTED,
      `Protocol version mismatch: ${protocolVersion}`
    );
    SessionCommands.disconnect();
    return;
  }

  const getPasswordSalt = passwordSaltSupported(serverOptions, webClient);
  const { options } = webClient;

  switch (options.reason) {
    case WebSocketConnectReason.LOGIN:
      SessionCommands.updateStatus(StatusEnum.LOGGING_IN, 'Logging In...');
      if (getPasswordSalt) {
        SessionCommands.requestPasswordSalt(options);
      } else {
        SessionCommands.login(options);
      }
      break;
    case WebSocketConnectReason.REGISTER:
      const passwordSalt = getPasswordSalt ? generateSalt() : null;
      SessionCommands.register(options, passwordSalt);
      break;
    case WebSocketConnectReason.ACTIVATE_ACCOUNT:
      if (getPasswordSalt) {
        SessionCommands.requestPasswordSalt(options);
      } else {
        SessionCommands.activateAccount(options);
      }
      break;
    case WebSocketConnectReason.PASSWORD_RESET_REQUEST:
      SessionCommands.resetPasswordRequest(options);
      break;
    case WebSocketConnectReason.PASSWORD_RESET_CHALLENGE:
      SessionCommands.resetPasswordChallenge(options);
      break;
    case WebSocketConnectReason.PASSWORD_RESET:
      if (getPasswordSalt) {
        SessionCommands.requestPasswordSalt(options);
      } else {
        SessionCommands.resetPassword(options);
      }
      break;
    default:
      SessionCommands.updateStatus(
        StatusEnum.DISCONNECTED,
        'Unknown Connection Reason: ' + options.reason
      );
      SessionCommands.disconnect();
      break;
  }

  webClient.options = {};
  SessionPersistence.updateInfo(serverName!, serverVersion!);
}

function serverMessage({ message }: IEvent_ServerMessage) {
  SessionPersistence.serverMessage(message!);
}

function userJoined({ userInfo }: IEvent_UserJoined) {
  SessionPersistence.userJoined(userInfo!);
}

function userLeft({ name }: IEvent_UserLeft) {
  SessionPersistence.userLeft(name!);
}
