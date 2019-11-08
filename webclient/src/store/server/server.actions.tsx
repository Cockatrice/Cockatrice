import { Types } from './server.types';

export const Actions = {
	connectServer: () => ({
		type: Types.CONNECT_SERVER
	}),
	connectionClosed: reason => ({
		type: Types.CONNECTION_CLOSED,
		reason
	}),
	joinRoom: roomId => ({
		type: Types.JOIN_ROOM,
		roomId
	}),
	serverMessage: message => ({
		type: Types.SERVER_MESSAGE,
		message
	}),
	updateInfo: info => ({
		type: Types.UPDATE_INFO,
		info
	}),
	updateRooms: rooms => ({
		type: Types.UPDATE_ROOMS,
		rooms
	}),
	updateRoom: room => ({
		type: Types.UPDATE_ROOM,
		room
	}),
	updateStatus: status => ({
		type: Types.UPDATE_STATUS,
		status
	}),
	updateUser: user => ({
		type: Types.UPDATE_USER,
		user
	})
}
