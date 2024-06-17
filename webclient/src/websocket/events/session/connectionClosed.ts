import { StatusEnum } from 'types';
import webClient from '../../WebClient';
import { updateStatus } from '../../commands/session';
import { ConnectionClosedData } from './interfaces';

export function connectionClosed({ reason, reasonStr }: ConnectionClosedData): void {
  let message: string;

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

  updateStatus(StatusEnum.DISCONNECTED, message);
}
