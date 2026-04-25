import { CaseReducer, PayloadAction } from '@reduxjs/toolkit';
import { Data } from '@app/types';
import { ServerState } from './server.interfaces';

export const replayReducers = {
  replayList: ((state, action) => {
    const replays: { [gameId: number]: Data.ServerInfo_ReplayMatch } = {};
    for (const match of action.payload.matchList) {
      replays[match.gameId] = match;
    }
    state.replays = replays;
  }) as CaseReducer<ServerState, PayloadAction<{ matchList: Data.ServerInfo_ReplayMatch[] }>>,

  replayAdded: ((state, action) => {
    const { matchInfo } = action.payload;
    state.replays[matchInfo.gameId] = matchInfo;
  }) as CaseReducer<ServerState, PayloadAction<{ matchInfo: Data.ServerInfo_ReplayMatch }>>,

  replayModifyMatch: ((state, action) => {
    const { gameId, doNotHide } = action.payload;
    const existing = state.replays[gameId];
    if (!existing) {
      return;
    }
    existing.doNotHide = doNotHide;
  }) as CaseReducer<ServerState, PayloadAction<{ gameId: number; doNotHide: boolean }>>,

  replayDeleteMatch: ((state, action) => {
    delete state.replays[action.payload.gameId];
  }) as CaseReducer<ServerState, PayloadAction<{ gameId: number }>>,

  replayDownloaded: ((state, action) => {
    state.downloadedReplay = action.payload;
  }) as CaseReducer<ServerState, PayloadAction<{ replayId: number; replayData: Uint8Array }>>,
};
