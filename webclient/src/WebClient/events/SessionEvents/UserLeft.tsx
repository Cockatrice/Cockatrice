export const UserLeft = {
  id: '.Event_UserLeft.ext',
  action: ({ name }, webClient) => {
    console.info('Event_UserLeft', name);
    webClient.services.session.userLeft(name);
  }
};