import * as _ from 'lodash';
import { RoomsState } from './rooms.interfaces';

interface State {
	rooms: RoomsState
}

export const Selectors = {
	getRooms: ({ rooms }: State) => rooms.rooms,
	getRoom: ({ rooms }: State, id: number) =>
		_.find(rooms.rooms, ({roomId}) => roomId === id),
	getActive: ({ rooms }: State) => rooms.active,
	getJoined: ({ rooms }: State) => rooms.joined,
	getMessages: ({ rooms }: State) => rooms.messages,

	getActiveRoom: (state: State) => Selectors.getRoom(state, Selectors.getActive(state)),
	getJoinedRooms: (state: State) => {
		const joined = Selectors.getJoined(state);
		return _.filter(Selectors.getRooms(state), room => joined[room.roomId]);
	},
	getRoomMessages: (state: State, roomId: number) => Selectors.getMessages(state)[roomId]
}

