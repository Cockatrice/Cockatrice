import { CaseReducer, PayloadAction } from '@reduxjs/toolkit';
import { Data } from '@app/types';
import { mergeSetFields } from '../common';
import { GamesState } from './game.interfaces';
import { MAX_GAME_MESSAGES, formatLeaveMessage, pushEventMessage } from './game.reducer.helpers';
import {
  diffPlayerProperties,
  formatPlayerJoined,
  formatPropertyDiff,
} from './messageLog';

export const playerReducers = {
  playerJoined: ((state, action) => {
    const { gameId, playerProperties } = action.payload;
    const game = state.games[gameId];
    if (!game) {
      return;
    }
    game.players[playerProperties.playerId] = {
      properties: playerProperties,
      deckList: '',
      zones: {},
      counters: {},
      arrows: {},
    };
    pushEventMessage(game, playerProperties.playerId, formatPlayerJoined(game, playerProperties.playerId));
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number; playerProperties: Data.ServerInfo_PlayerProperties }>>,

  playerLeft: ((state, action) => {
    const { gameId, playerId, reason, timeReceived } = action.payload;
    const game = state.games[gameId];
    if (!game) {
      return;
    }
    const player = game.players[playerId];
    const playerName = player?.properties.userInfo?.name ?? 'Unknown player';

    if (game.messages.length >= MAX_GAME_MESSAGES) {
      game.messages = game.messages.slice(game.messages.length - MAX_GAME_MESSAGES + 1);
    }
    game.messages.push({
      playerId,
      message: formatLeaveMessage(playerName, reason),
      timeReceived,
      kind: 'event',
    });

    delete game.players[playerId];
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number; playerId: number; reason: number; timeReceived: number }>>,

  playerPropertiesChanged: ((state, action) => {
    const { gameId, playerId, properties } = action.payload;
    const game = state.games[gameId];
    const player = game?.players[playerId];
    if (!game || !player) {
      return;
    }
    const previous = { ...player.properties };
    mergeSetFields(Data.ServerInfo_PlayerPropertiesSchema, player.properties, properties);
    const diff = diffPlayerProperties(previous, player.properties);
    for (const msg of formatPropertyDiff(game, playerId, diff)) {
      pushEventMessage(game, playerId, msg);
    }
  }) as CaseReducer<GamesState, PayloadAction<{ gameId: number; playerId: number; properties: Data.ServerInfo_PlayerProperties }>>,
};
