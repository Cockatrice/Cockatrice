import { StatusEnum } from 'types';
import { ProtoController } from '../../services/ProtoController';
import { updateStatus } from '../../commands/session';
import { ConnectionClosedData } from './interfaces';

export function connectionClosed({ reason, reasonStr, endTime }: ConnectionClosedData): void {
  let message: string;

  // @TODO (5)
  if (reasonStr) {
    message = reasonStr;
  } else {
    const { CloseReason } = ProtoController.root.Event_ConnectionClosed;
    switch (reason) {
      case CloseReason.USER_LIMIT_REACHED:
        message = 'The server has reached its maximum user capacity';
        break;
      case CloseReason.TOO_MANY_CONNECTIONS:
        message = 'There are too many concurrent connections from your address';
        break;
      case CloseReason.BANNED:
        message = endTime > 0
          ? `You are banned until ${new Date(endTime * 1000).toLocaleString()}`
          : 'You are banned';
        break;
      case CloseReason.DEMOTED:
        message = 'You were demoted';
        break;
      case CloseReason.SERVER_SHUTDOWN:
        message = 'Scheduled server shutdown';
        break;
      case CloseReason.USERNAMEINVALID:
        message = 'Invalid username';
        break;
      case CloseReason.LOGGEDINELSEWERE:
        message = 'You have been logged out due to logging in at another location';
        break;
      case CloseReason.OTHER:
      default:
        message = 'Unknown reason';
        break;
    }
  }

  updateStatus(StatusEnum.DISCONNECTED, message);
}
