import * as _ from 'lodash';

import { StatusEnum, getStatusEnumLabel } from 'types';

import { ServerState } from './server.interfaces'
import { Types } from './server.types';

const initialState: ServerState = {
	status: {
		state: StatusEnum.DISCONNECTED,
		description: getStatusEnumLabel(StatusEnum.DISCONNECTED)
	},
	info: {
		message: null,
		name: null,
		version: null
	},
	rooms: [],
	user: null
};

export const serverReducer = (state = initialState, action: any) => {
	switch(action.type) {
		case Types.CONNECT_SERVER:
		case Types.CONNECTION_CLOSED: {
			return {
				...initialState,
				status: {
					...state.status
				}
			}
		}
		case Types.JOIN_ROOM: {
			return {
				...state
			};
		}
		case Types.SERVER_MESSAGE: {
			const { message } = action;
			const { info } = state;

			return {
				...state,
				info: { ...info, message }
			}
		}
		case Types.UPDATE_INFO: {
			const { name, version } = action.info;
			const { info } = state;
			
			return {
				...state,
				info: { ...info, name, version }
			}
		}
		case Types.UPDATE_ROOMS: {
			const rooms = {
				...state.rooms
			};

			_.each(action.rooms, (room) => {
				const existing = rooms[room.roomId] || {};

				rooms[room.roomId] = {
					...existing,
					...room
				};
			});

			return { ...state, rooms };
		}
		case Types.UPDATE_ROOM: {
			return {
				...state,
				room: {
					...action.room
				}
			};
		}
		case Types.UPDATE_STATUS: {
			const { status } = action;

			return {
				...state,
				status: { ...status }
			}
		}
		case Types.UPDATE_USER: {
			const { user } = action;

			return {
				...state,
				user: {
					...state.user,
					...user
				}
			}
		}
		default:
			return state;
	}
}
