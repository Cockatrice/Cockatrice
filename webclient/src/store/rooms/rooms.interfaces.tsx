export interface RoomsState {
	rooms: RoomsStateRooms;
	joined: {
		[roomId: number]: true;
	};
	active: number;
}

export interface RoomsStateRooms {
	[id: number]: any;
}

export interface RoomsStateRoom {
	autoJoin: boolean
	description: string;
	gameCount: number;
	gameList: any[];
	gametypeList: any[];
	name: string;
	permissionlevel: RoomsStateRoomAccessLevel;
	playerCount: number;
	privilegelevel: RoomsStateRoomAccessLevel;
	roomId: number;
	userList: any[];
}

export enum RoomsStateRoomAccessLevel {
	"none"
}
