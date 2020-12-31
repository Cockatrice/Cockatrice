export const UserLeft = {
  id: ".Event_UserLeft.ext",
  action: ({ name }, webClient) => {
    webClient.services.session.userLeft(name);
  }
};