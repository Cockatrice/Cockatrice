export const UserJoined = {
  id: ".Event_UserJoined.ext",
  action: ({ userInfo }, webClient) => {
    webClient.persistence.session.userJoined(userInfo);
  }
};
