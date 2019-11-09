import { Types } from './rooms.types';

export const Actions = {
	updateRooms: rooms => ({
		type: Types.UPDATE_ROOMS,
		rooms
	}),

	joinRoom: roomId => ({
		type: Types.JOIN_ROOM,
		roomId
	}),

	leaveRoom: roomId => ({
		type: Types.LEAVE_ROOM,
		roomId
	}),

	addMessage: (roomId, message) => ({
		type: Types.ADD_MESSAGE,
		roomId,
		message
	})
}
