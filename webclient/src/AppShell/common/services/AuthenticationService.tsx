import { StatusEnum } from 'types';
import webClient from 'WebClient/WebClient';

export class AuthenticationService {
	static connect(options) {
		webClient.services.server.connectServer(options);
	}

	static isConnected(state) {
		return state === StatusEnum.LOGGEDIN;
	}
}