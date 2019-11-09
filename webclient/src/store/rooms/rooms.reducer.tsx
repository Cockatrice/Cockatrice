import * as _ from 'lodash';

import { RoomsState } from './rooms.interfaces'
import { Types } from './rooms.types';

const initialState: RoomsState = {
	rooms: {},
	joined: {},
	messages: {},
	active: null,
};

export const roomsReducer = (state = initialState, action: any) => {
	switch(action.type) {
		case Types.UPDATE_ROOMS: {
			const rooms = {
				...state.rooms
			};

			// Server does not send everything everytime
			_.each(action.rooms, (room, order) => {
				const existing = rooms[room.roomId] || {};

				rooms[room.roomId] = {
					...existing,
					...room,
					order
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
			const { joined, messages, active } = state;

			const _joined = {
				...joined
			};

			const _messages = {
				...messages
			};

			delete _joined[roomId];
			delete _messages[roomId];

			return {
				...state,

				joined: _joined,
				messages: _messages,

				active: active === roomId ? null : active
			}
		}
		case Types.ADD_MESSAGE: {
			const { roomId, message } = action;
			const { messages } = state;

			let roomMessages = [ ...(messages[roomId] || []) ];

			// @TODO add this value to a const somewhere higher up
			if (roomMessages.length === 1000) {
				roomMessages.shift();
			}

			roomMessages.push(message);

			return {
				...state,
				messages: {
					...messages,

					[roomId]: [
						...roomMessages
					]
				}
			}
		}
		default:
			return state;
	}
}
