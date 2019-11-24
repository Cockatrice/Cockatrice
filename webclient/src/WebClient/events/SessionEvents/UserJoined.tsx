export const UserJoined = {
  id: ".Event_UserJoined.ext",
  action: ({ userInfo }, webClient) => {
    webClient.services.session.userJoined(userInfo);
  }
};