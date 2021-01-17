import { Types } from "./rooms.types";

export const Actions = {
  clearStore: () => ({
    type: Types.CLEAR_STORE
  }),
  
  updateRooms: rooms => ({
    type: Types.UPDATE_ROOMS,
    rooms
  }),

  joinRoom: roomInfo => ({
    type: Types.JOIN_ROOM,
    roomInfo
  }),

  leaveRoom: roomId => ({
    type: Types.LEAVE_ROOM,
    roomId
  }),

  addMessage: (roomId, message) => ({
    type: Types.ADD_MESSAGE,
    roomId,
    message
  }),

  updateGames: (roomId, games) => ({
    type: Types.UPDATE_GAMES,
    roomId,
    games
  }),

  userJoined: (roomId, user) => ({
    type: Types.USER_JOINED,
    roomId,
    user
  }),

  userLeft: (roomId, name) => ({
    type: Types.USER_LEFT,
    roomId,
    name
  }),

  sortGames: (roomId, field, order) => ({
    type: Types.SORT_GAMES,
    roomId,
    field,
    order
  })
}
