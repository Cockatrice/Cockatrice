export interface Replay {
  replayId: number;
  replayName: string;
  duration: number;
}

export interface ReplayMatch {
  replayList: Replay[];
  gameId: number;
  roomName: string;
  timeStarted: number;
  length: number;
  gameName: string;
  playerNames: string[];
  doNotHide: boolean;
}
