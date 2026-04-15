import { App, Data } from '@app/types';
import { updateStatus } from '../../commands/session';

export function connectionClosed({ reason, reasonStr, endTime }: Data.Event_ConnectionClosed): void {
  let message: string;

  // @TODO (5)
  if (reasonStr) {
    message = reasonStr;
  } else {
    switch (reason) {
      case Data.Event_ConnectionClosed_CloseReason.USER_LIMIT_REACHED:
        message = 'The server has reached its maximum user capacity';
        break;
      case Data.Event_ConnectionClosed_CloseReason.TOO_MANY_CONNECTIONS:
        message = 'There are too many concurrent connections from your address';
        break;
      case Data.Event_ConnectionClosed_CloseReason.BANNED:
        message = typeof endTime === 'number' && endTime > 0 && Number.isFinite(endTime)
          ? `You are banned until ${new Date(endTime * 1000).toLocaleString()}`
          : 'You are banned';
        break;
      case Data.Event_ConnectionClosed_CloseReason.DEMOTED:
        message = 'You were demoted';
        break;
      case Data.Event_ConnectionClosed_CloseReason.SERVER_SHUTDOWN:
        message = 'Scheduled server shutdown';
        break;
      case Data.Event_ConnectionClosed_CloseReason.USERNAMEINVALID:
        message = 'Invalid username';
        break;
      case Data.Event_ConnectionClosed_CloseReason.LOGGEDINELSEWERE:
        message = 'You have been logged out due to logging in at another location';
        break;
      case Data.Event_ConnectionClosed_CloseReason.OTHER:
      default:
        message = 'Unknown reason';
        break;
    }
  }

  updateStatus(App.StatusEnum.DISCONNECTED, message);
}
