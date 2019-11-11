import { Types } from './server.types';

export const Actions = {
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
  })
}
