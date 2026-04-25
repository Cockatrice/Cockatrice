import { CaseReducer, PayloadAction } from '@reduxjs/toolkit';
import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';
import { normalizeBannedUserError } from '../common';
import { ServerState } from './server.interfaces';

export const accountReducers = {
  registrationFailed: ((state, action) => {
    const { reason, endTime } = action.payload;
    const error = endTime
      ? normalizeBannedUserError(reason, endTime)
      : reason;
    state.registrationError = error;
  }) as CaseReducer<ServerState, PayloadAction<{ reason: string; endTime?: number }>>,

  clearRegistrationErrors: ((state) => {
    state.registrationError = null;
  }) as CaseReducer<ServerState>,

  accountEditChanged: ((state, action) => {
    if (state.user) {
      state.user = create(Data.ServerInfo_UserSchema, { ...state.user, ...action.payload.user });
    }
  }) as CaseReducer<ServerState, PayloadAction<{ user: Partial<Data.ServerInfo_User> }>>,

  accountImageChanged: ((state, action) => {
    if (state.user) {
      state.user = create(Data.ServerInfo_UserSchema, { ...state.user, ...action.payload.user });
    }
  }) as CaseReducer<ServerState, PayloadAction<{ user: Partial<Data.ServerInfo_User> }>>,
};
