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
