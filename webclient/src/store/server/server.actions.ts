import { Types } from "./server.types";

export const Actions = {
  clearStore: () => ({
    type: Types.CLEAR_STORE
  }),
  connectServer: () => ({
    type: Types.CONNECT_SERVER
  }),
  connectionClosed: reason => ({
    type: Types.CONNECTION_CLOSED,
    reason
  }),
  serverMessage: message => ({
    type: Types.SERVER_MESSAGE,
    message
  }),
  updateBuddyList: buddyList => ({
    type: Types.UPDATE_BUDDY_LIST,
    buddyList
  }),
  addToBuddyList: user => ({
    type: Types.ADD_TO_BUDDY_LIST,
    user
  }),
  removeFromBuddyList: userName => ({
    type: Types.REMOVE_FROM_BUDDY_LIST,
    userName
  }),
  updateIgnoreList: ignoreList => ({
    type: Types.UPDATE_IGNORE_LIST,
    ignoreList
  }),
  addToIgnoreList: user => ({
    type: Types.ADD_TO_IGNORE_LIST,
    user
  }),
  removeFromIgnoreList: userName => ({
    type: Types.REMOVE_FROM_IGNORE_LIST,
    userName
  }),
  updateInfo: info => ({
    type: Types.UPDATE_INFO,
    info
  }),
  updateStatus: status => ({
    type: Types.UPDATE_STATUS,
    status
  }),
  updateUser: user => ({
    type: Types.UPDATE_USER,
    user
  }),
  updateUsers: users => ({
    type: Types.UPDATE_USERS,
    users
  }),
  userJoined: user => ({
    type: Types.USER_JOINED,
    user
  }),
  userLeft: name => ({
    type: Types.USER_LEFT,
    name
  }),
  viewLogs: logs => ({
    type: Types.VIEW_LOGS,
    logs
  }),
  clearLogs: () => ({
    type: Types.CLEAR_LOGS
  })
}
