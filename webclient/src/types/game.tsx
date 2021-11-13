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
