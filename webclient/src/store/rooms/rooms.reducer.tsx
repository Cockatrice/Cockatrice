import * as _ from 'lodash';

import { RoomsState } from './rooms.interfaces'
import { Types } from './rooms.types';

const initialState: RoomsState = {
	rooms: {},
	joined: {},
	active: null
};

export const roomsReducer = (state = initialState, action: any) => {
	switch(action.type) {
		case Types.UPDATE_ROOMS: {
			const rooms = {
				...state.rooms
			};

			// Server does not send everything everytime
			_.each(action.rooms, (room) => {
				const existing = rooms[room.roomId] || {};

				rooms[room.roomId] = {
					...existing,
					...room
				};
			});

			return { ...state, rooms };
		}
		case Types.JOIN_ROOM: {
			const { roomId } = action;
			const { joined } = state;

			return {
				...state,

				joined: {
					...joined,
					[roomId]: true
				},

				active: roomId
			}
		}
		case Types.LEAVE_ROOM: {
			const { roomId } = action;
			const { joined, active } = state;

			const _joined = {
				...joined
			};

			delete _joined[roomId];

			return {
				...state,

				joined: _joined,

				active: active === roomId ? null : active
			}
		}
		default:
			return state;
	}
}
