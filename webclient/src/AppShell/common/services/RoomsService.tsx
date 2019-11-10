import webClient from 'WebClient/WebClient';

export class RoomsService {
	static joinRoom(roomId) {
		webClient.services.server.joinRoom(roomId);
	}

	static roomSay(roomId, message) {
		webClient.commands.room.roomSay(roomId, message);
	}
}