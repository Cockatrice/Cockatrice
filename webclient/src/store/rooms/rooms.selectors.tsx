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
	getActiveRoom: (state: State) => Selectors.getRoom(state, Selectors.getActive(state))
}

