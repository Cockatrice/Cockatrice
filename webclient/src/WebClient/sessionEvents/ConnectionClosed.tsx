import { ServerService } from 'common/services/data';
import { StatusEnum } from 'common/types';

import WebClient from '../WebClient';

export const ConnectionClosed = {
  id: '.Event_ConnectionClosed.ext',
  action: ({ reason }) => {
    const webClient = WebClient.getInstance();
    let message = '';

    switch(reason) {
      case webClient.pb.Event_ConnectionClosed.CloseReason.USER_LIMIT_REACHED:
        message = 'The server has reached its maximum user capacity';
        break;
      case webClient.pb.Event_ConnectionClosed.CloseReason.TOO_MANY_CONNECTIONS:
        message = 'There are too many concurrent connections from your address';
        break;
      case webClient.pb.Event_ConnectionClosed.CloseReason.BANNED:
        message = 'You are banned';
        break;
      case webClient.pb.Event_ConnectionClosed.CloseReason.SERVER_SHUTDOWN:
        message = 'Scheduled server shutdown';
        break;
      case webClient.pb.Event_ConnectionClosed.CloseReason.USERNAMEINVALID:
        message = 'Invalid username';
        break;
      case webClient.pb.Event_ConnectionClosed.CloseReason.LOGGEDINELSEWERE:
        message = 'You have been logged out due to logging in at another location';
        break;
      case webClient.pb.Event_ConnectionClosed.CloseReason.OTHER:
      default:
        message = 'Unknown reason';
        break;
    }

    ServerService.updateStatus(StatusEnum.DISCONNECTED, message);
  }
};