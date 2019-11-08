import { ServerState } from './server.interfaces';

interface State {
	server: ServerState
}

export const Selectors = {
	getMessage: ({ server }: State) => server.info.message,
	getName: ({ server }: State) => server.info.name,
	getDescription: ({ server }: State) => server.status.description,
	getState: ({ server }: State) => server.status.state,
	getRooms: ({ server }: State) => server.rooms,
	getRoom: ({ server }: State, id: string) =>
		server.rooms.filter(({roomId}) => roomId === id)(0),
	getUser: ({ server }: State) => server.user
}