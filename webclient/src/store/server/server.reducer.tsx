// eslint-disable-next-line
import { ServerState } from './server.interfaces'
import { Types } from './server.types';
// import { Types } from './server.types';
import { StatusEnum, getStatusEnumLabel } from '../../common/types';

const initialState: ServerState = {
	message: '',
	status: {
		state: StatusEnum.DISCONNECTED,
		description: getStatusEnumLabel(StatusEnum.DISCONNECTED)
	},
};

export const serverReducer = (state = initialState, action: any) => {
	switch(action.type) {
		case Types.CONNECT_SERVER:
		case Types.CONNECTION_CLOSED:
			return {
				...initialState
			}
		case Types.SERVER_MESSAGE:
			const { message } = action;

			return {
				...state,
				message
			}
		case Types.UPDATE_STATUS:
			const { status } = action;

			return {
				...state,
				status: {
					...status
				}
			}
		default:
			return state;
	}
}
