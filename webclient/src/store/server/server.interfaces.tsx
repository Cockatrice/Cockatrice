export interface ServerConnectParams {
	host: string;
	port: string;
	user: string;
	pass: string;
}

export interface ServerState {
	info: ServerStateInfo;
	status: ServerStateStatus;
	rooms: ServerStateRooms;
	user: ServerStateUser;
}

export interface ServerStateStatus {
	description: string;
	state: number;
}

export interface ServerStateInfo {
	message: string;
	name: string;
	version: string;
}

export interface ServerStateRooms {
	[roomId: string]: any;
}

export interface ServerStateRoom {
	autoJoin: boolean
	description: string;
	gameCount: number;
	gameList: any[];
	gametypeList: any[];
	name: string;
	permissionlevel: ServerStateRoomAccessLevel;
	playerCount: number;
	privilegelevel: ServerStateRoomAccessLevel;
	roomId: number;
	userList: any[];
}

export interface ServerStateUser {
	accountageSecs: number;
	avatarBmp: Uint8Array;
	country: string;
	gender: number;
	name: string;
	privlevel: ServerStateUserAccessLevel;
	userLevel: ServerStateUserPrivLevel;
}

export enum ServerStateRoomAccessLevel {
	"none"
}

export enum ServerStateUserAccessLevel {
	"NONE"
}

export enum ServerStateUserPrivLevel {
	"unknown 1",
	"unknown 2",
	"unknown 3",
	"unknown 4"
}