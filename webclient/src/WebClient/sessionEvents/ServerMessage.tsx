export const ServerMessage = {
  id: '.Event_ServerMessage.ext',
  action: ({ message }, webClient) => {
    webClient.services.server.serverMessage(message);
  }
};