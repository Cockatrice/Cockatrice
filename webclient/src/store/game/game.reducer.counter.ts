import { CaseReducer, PayloadAction } from '@reduxjs/toolkit';
import { Data } from '@app/types';
import { GamesState } from './game.interfaces';
import { pushEventMessage } from './game.reducer.helpers';
import { formatCounterSet } from './messageLog';

export const counterReducers = {
  counterCreated: ((state, action) => {
    const { gameId, playerId, data } = action.payload;
    const player = state.games[gameId]?.players[playerId];
    if (player && data.counterInfo) {
      player.counters[data.counterInfo.id] = { ...data.counterInfo };
    }
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number; playerId: number; data: Data.Event_CreateCounter }>>,

  counterSet: ((state, action) => {
    const { gameId, playerId, data } = action.payload;
    const game = state.games[gameId];
    const counter = game?.players[playerId]?.counters[data.counterId];
    if (!game || !counter) {
      return;
    }
    const previousValue = counter.count;
    counter.count = data.value;
    pushEventMessage(
      game,
      playerId,
      formatCounterSet(game, playerId, data, counter.name, previousValue),
    );
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number; playerId: number; data: Data.Event_SetCounter }>>,

  counterDeleted: ((state, action) => {
    const { gameId, playerId, data } = action.payload;
    const player = state.games[gameId]?.players[playerId];
    if (player) {
      delete player.counters[data.counterId];
    }
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number; playerId: number; data: Data.Event_DelCounter }>>,
};
