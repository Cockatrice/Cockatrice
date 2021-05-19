export const UserLeft = {
  id: ".Event_UserLeft.ext",
  action: ({ name }, webClient) => {
    webClient.persistence.session.userLeft(name);
  }
};
