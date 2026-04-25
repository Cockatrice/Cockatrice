import { CaseReducer, PayloadAction } from '@reduxjs/toolkit';
import { Data } from '@app/types';
import { normalizeLogs } from '../common';
import { ServerState } from './server.interfaces';

export const moderationReducers = {
  banFromServer: ((state, action) => {
    state.banUser = action.payload.userName;
  }) as CaseReducer<ServerState, PayloadAction<{ userName: string }>>,

  banHistory: ((state, action) => {
    state.banHistory[action.payload.userName] = action.payload.banHistory;
  }) as CaseReducer<ServerState, PayloadAction<{ userName: string; banHistory: Data.ServerInfo_Ban[] }>>,

  warnHistory: ((state, action) => {
    state.warnHistory[action.payload.userName] = action.payload.warnHistory;
  }) as CaseReducer<ServerState, PayloadAction<{ userName: string; warnHistory: Data.ServerInfo_Warning[] }>>,

  warnListOptions: ((state, action) => {
    state.warnListOptions = action.payload.warnList;
  }) as CaseReducer<ServerState, PayloadAction<{ warnList: Data.Response_WarnList[] }>>,

  warnUser: ((state, action) => {
    state.warnUser = action.payload.userName;
  }) as CaseReducer<ServerState, PayloadAction<{ userName: string }>>,

  getAdminNotes: ((state, action) => {
    state.adminNotes[action.payload.userName] = action.payload.notes;
  }) as CaseReducer<ServerState, PayloadAction<{ userName: string; notes: string }>>,

  updateAdminNotes: ((state, action) => {
    state.adminNotes[action.payload.userName] = action.payload.notes;
  }) as CaseReducer<ServerState, PayloadAction<{ userName: string; notes: string }>>,

  adjustMod: ((state, action) => {
    const { userName, shouldBeMod, shouldBeJudge } = action.payload;
    const user = state.users[userName];
    if (!user) {
      return;
    }
    let newLevel = user.userLevel;
    newLevel = shouldBeMod
      ? (newLevel | Data.ServerInfo_User_UserLevelFlag.IsModerator)
      : (newLevel & ~Data.ServerInfo_User_UserLevelFlag.IsModerator);
    newLevel = shouldBeJudge
      ? (newLevel | Data.ServerInfo_User_UserLevelFlag.IsJudge)
      : (newLevel & ~Data.ServerInfo_User_UserLevelFlag.IsJudge);
    user.userLevel = newLevel;
  }) as CaseReducer<ServerState, PayloadAction<{ userName: string; shouldBeMod: boolean; shouldBeJudge: boolean }>>,

  viewLogs: ((state, action) => {
    state.logs = normalizeLogs(action.payload.logs);
  }) as CaseReducer<ServerState, PayloadAction<{ logs: Data.ServerInfo_ChatMessage[] }>>,

  clearLogs: ((state) => {
    state.logs = { room: [], game: [], chat: [] };
  }) as CaseReducer<ServerState>,
};
