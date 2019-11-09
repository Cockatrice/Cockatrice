import webClient from 'WebClient/WebClient';

export class RoomsService {
	static joinRoom(roomId) {
		webClient.services.server.joinRoom(roomId);
	}
}