export const ServerMessage = {
  id: ".Event_ServerMessage.ext",
  action: ({ message }, webClient) => {
    webClient.persistence.session.serverMessage(message);
  }
};
