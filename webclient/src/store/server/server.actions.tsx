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

	updateStatus: status => ({
		type: Types.UPDATE_STATUS,
		status
	})
}
