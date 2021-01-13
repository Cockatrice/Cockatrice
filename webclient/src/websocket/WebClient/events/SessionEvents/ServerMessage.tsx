export const ServerMessage = {
  id: ".Event_ServerMessage.ext",
  action: ({ message }, webClient) => {
    webClient.services.session.serverMessage(message);
  }
};