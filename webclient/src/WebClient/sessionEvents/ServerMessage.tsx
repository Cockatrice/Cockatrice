import { ServerService } from 'common/services/data';

export const ServerMessage = {
  id: '.Event_ServerMessage.ext',
  action: ({ message }) => {
    ServerService.serverMessage(message);
  }
};