export const ServerShutdown = {
  id: '.Event_ServerShutdown.ext',
  action: (payload) => {
    console.info('Event_ServerShutdown', payload);
  }
};