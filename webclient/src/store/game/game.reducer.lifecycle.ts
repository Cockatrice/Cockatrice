import { CaseReducer, PayloadAction } from '@reduxjs/toolkit';
import { Data } from '@app/types';
import { GamesState } from './game.interfaces';

const initialState: GamesState = { games: {} };

export const lifecycleReducers = {
  clearStore: (() => initialState) as CaseReducer<GamesState>,

  gameJoined: ((state, action) => {
    const { data } = action.payload;
    const gameInfo = data.gameInfo;
    if (!gameInfo) {
      return;
    }
    state.games[gameInfo.gameId] = {
      info: gameInfo,
      hostId: data.hostId,
      localPlayerId: data.playerId,
      spectator: data.spectator,
      judge: data.judge,
      resuming: data.resuming,
      started: gameInfo.started,
      activePlayerId: -1,
      activePhase: -1,
      secondsElapsed: 0,
      reversed: false,
      players: {},
      messages: [],
    };
  }) as CaseReducer<GamesState, PayloadAction<{ data: Data.Event_GameJoined }>>,

  gameLeft: ((state, action) => {
    delete state.games[action.payload.gameId];
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number }>>,

  gameClosed: ((state, action) => {
    delete state.games[action.payload.gameId];
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number }>>,

  kicked: ((state, action) => {
    delete state.games[action.payload.gameId];
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number }>>,
};
