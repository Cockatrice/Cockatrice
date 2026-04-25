import { CaseReducer, PayloadAction } from '@reduxjs/toolkit';
import { isFieldSet } from '@bufbuild/protobuf';
import { Data } from '@app/types';
import { GamesState } from './game.interfaces';
import { normalizePlayers, pushEventMessage } from './game.reducer.helpers';
import {
  EVENT_PLAYER_ID_SYSTEM,
  formatActivePhaseSet,
  formatActivePlayerSet,
  formatGameStart,
  formatTurnReversed,
} from './messageLog';

export const turnReducers = {
  gameHostChanged: ((state, action) => {
    const { gameId, hostId } = action.payload;
    const game = state.games[gameId];
    if (game) {
      game.hostId = hostId;
    }
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number; hostId: number }>>,

  gameStateChanged: ((state, action) => {
    const { gameId, data } = action.payload;
    const game = state.games[gameId];
    if (!game) {
      return;
    }

    if (data.playerList?.length > 0) {
      game.players = normalizePlayers(data.playerList);
    }
    const wasStarted = game.started;
    if (isFieldSet(data, Data.Event_GameStateChangedSchema.field.gameStarted)) {
      game.started = data.gameStarted;
    }
    if (isFieldSet(data, Data.Event_GameStateChangedSchema.field.activePlayerId)) {
      game.activePlayerId = data.activePlayerId;
    }
    if (isFieldSet(data, Data.Event_GameStateChangedSchema.field.activePhase)) {
      game.activePhase = data.activePhase;
    }
    if (isFieldSet(data, Data.Event_GameStateChangedSchema.field.secondsElapsed)) {
      game.secondsElapsed = data.secondsElapsed;
    }
    if (!wasStarted && game.started) {
      pushEventMessage(game, EVENT_PLAYER_ID_SYSTEM, formatGameStart());
    }
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number; data: Data.Event_GameStateChanged }>>,

  activePlayerSet: ((state, action) => {
    const game = state.games[action.payload.gameId];
    if (!game) {
      return;
    }
    const previous = game.activePlayerId;
    game.activePlayerId = action.payload.activePlayerId;
    if (previous !== action.payload.activePlayerId) {
      pushEventMessage(game, action.payload.activePlayerId, formatActivePlayerSet(game, action.payload.activePlayerId));
    }
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number; activePlayerId: number }>>,

  activePhaseSet: ((state, action) => {
    const game = state.games[action.payload.gameId];
    if (!game) {
      return;
    }
    const previous = game.activePhase;
    game.activePhase = action.payload.phase;
    if (previous !== action.payload.phase && game.started) {
      pushEventMessage(game, EVENT_PLAYER_ID_SYSTEM, formatActivePhaseSet(action.payload.phase));
    }
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number; phase: number }>>,

  turnReversed: ((state, action) => {
    const game = state.games[action.payload.gameId];
    if (!game) {
      return;
    }
    game.reversed = action.payload.reversed;
    pushEventMessage(game, game.activePlayerId, formatTurnReversed(game, game.activePlayerId, action.payload.reversed));
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number; reversed: boolean }>>,
};
