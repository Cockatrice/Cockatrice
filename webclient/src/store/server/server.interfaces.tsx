export interface ServerConnectParams {
	host: string;
	port: string;
	user: string;
	pass: string;
}

export interface ServerState {
	message: string;
	status: ServerStateStatus;
}

export interface ServerStateStatus {
	state: number;
	description: string;
}