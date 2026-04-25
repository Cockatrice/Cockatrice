import { CaseReducer, PayloadAction } from '@reduxjs/toolkit';
import { Data } from '@app/types';
import { GamesState } from './game.interfaces';
import { pushEventMessage } from './game.reducer.helpers';
import { formatArrowCreated } from './messageLog';

export const arrowReducers = {
  arrowCreated: ((state, action) => {
    const { gameId, playerId, data } = action.payload;
    const game = state.games[gameId];
    const player = game?.players[playerId];
    if (!game || !player || !data.arrowInfo) {
      return;
    }
    player.arrows[data.arrowInfo.id] = { ...data.arrowInfo };
    pushEventMessage(game, playerId, formatArrowCreated(game, playerId, data.arrowInfo));
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number; playerId: number; data: Data.Event_CreateArrow }>>,

  arrowDeleted: ((state, action) => {
    const { gameId, playerId, data } = action.payload;
    const player = state.games[gameId]?.players[playerId];
    if (player) {
      delete player.arrows[data.arrowId];
    }
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number; playerId: number; data: Data.Event_DeleteArrow }>>,
};
