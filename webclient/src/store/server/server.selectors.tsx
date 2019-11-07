import { ServerState } from './server.interfaces';

interface State {
	server: ServerState
}

export const Selectors = {
	getMessage: ({ server }: State) => server.message,
	getStatus: ({ server }: State) => server.status
}