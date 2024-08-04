export interface Game {
  description: string;
  gameId: number;
  gameType: string;
  gameTypes: string[];
  roomId: number;
  started: boolean;
}

export enum GameSortField {
  START_TIME = 'startTime'
}

export interface GameConfig {
  description: string;
  password: string;
  maxPlayer: number;
  onlyBuddies: boolean;
  onlyRegistered: boolean;
  spectatorsAllowed: boolean;
  spectatorsNeedPassword: boolean;
  spectatorsCanTalk: boolean;
  spectatorsSeeEverything: boolean;
  gameTypeIds: number[];
  joinAsJudge: boolean;
  joinAsSpectator: boolean;
}

export interface JoinGameParams {
  gameId: number;
  password: string;
  spectator: boolean;
  overrideRestrictions: boolean;
  joinAsJudge: boolean;
}

export enum LeaveGameReason {
  OTHER = 1,
  USER_KICKED = 2,
  USER_LEFT = 3,
  USER_DISCONNECTED = 4
}
