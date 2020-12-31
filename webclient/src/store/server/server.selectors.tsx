import { ServerState } from "./server.interfaces";

interface State {
  server: ServerState
}

export const Selectors = {
  getMessage: ({ server }: State) => server.info.message,
  getName: ({ server }: State) => server.info.name,
  getVersion: ({ server }: State) => server.info.version,
  getDescription: ({ server }: State) => server.status.description,
  getState: ({ server }: State) => server.status.state,
  getUser: ({ server }: State) => server.user,
  getUsers: ({ server }: State) => server.users,
  getLogs: ({ server }: State) => server.logs,
  getBuddyList: ({ server }: State) => server.buddyList,
  getIgnoreList: ({ server }: State) => server.ignoreList
}