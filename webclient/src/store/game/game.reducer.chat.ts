import { CaseReducer, PayloadAction } from '@reduxjs/toolkit';
import { Data } from '@app/types';
import { GamesState } from './game.interfaces';
import { MAX_GAME_MESSAGES, pushEventMessage } from './game.reducer.helpers';
import {
  formatDieRolled,
  formatZoneDumped,
  formatZoneShuffled,
} from './messageLog';

export const chatReducers = {
  gameSay: ((state, action) => {
    const { gameId, playerId, message, timeReceived } = action.payload;
    const game = state.games[gameId];
    if (!game) {
      return;
    }
    if (game.messages.length >= MAX_GAME_MESSAGES) {
      game.messages = game.messages.slice(game.messages.length - MAX_GAME_MESSAGES + 1);
    }
    game.messages.push({ playerId, message, timeReceived, kind: 'chat' });
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number; playerId: number; message: string; timeReceived: number }>>,

  // Logged-only actions: no state mutation but an event-log entry.
  zoneShuffled: ((state, action) => {
    const { gameId, playerId } = action.payload;
    const game = state.games[gameId];
    if (!game) {
      return;
    }
    pushEventMessage(game, playerId, formatZoneShuffled(game, playerId));
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number; playerId: number; data: Data.Event_Shuffle }>>,

  zoneDumped: ((state, action) => {
    const { gameId, playerId, data } = action.payload;
    const game = state.games[gameId];
    if (!game) {
      return;
    }
    pushEventMessage(game, playerId, formatZoneDumped(game, playerId, data));
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number; playerId: number; data: Data.Event_DumpZone }>>,

  dieRolled: ((state, action) => {
    const { gameId, playerId, data } = action.payload;
    const game = state.games[gameId];
    if (!game) {
      return;
    }
    pushEventMessage(game, playerId, formatDieRolled(game, playerId, data));
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number; playerId: number; data: Data.Event_RollDie }>>,
};
