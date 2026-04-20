import { createAction } from '@reduxjs/toolkit';
import { WebsocketTypes } from '@app/websocket/types';

import { serverSlice } from './server.reducer';

const SignalActions = {
  accountAwaitingActivation: createAction<{ options: WebsocketTypes.PendingActivationContext }>('server/accountAwaitingActivation'),
  accountActivationFailed: createAction('server/accountActivationFailed'),
  accountActivationSuccess: createAction('server/accountActivationSuccess'),
  loginSuccessful: createAction<{ options: WebsocketTypes.LoginSuccessContext }>('server/loginSuccessful'),
  loginFailed: createAction('server/loginFailed'),
  connectionFailed: createAction('server/connectionFailed'),
  testConnectionSuccessful: createAction('server/testConnectionSuccessful'),
  testConnectionFailed: createAction('server/testConnectionFailed'),
  registrationRequiresEmail: createAction('server/registrationRequiresEmail'),
  registrationSuccess: createAction('server/registrationSuccess'),
  registrationEmailError: createAction<{ error: string }>('server/registrationEmailError'),
  registrationPasswordError: createAction<{ error: string }>('server/registrationPasswordError'),
  registrationUserNameError: createAction<{ error: string }>('server/registrationUserNameError'),
  resetPassword: createAction('server/resetPassword'),
  resetPasswordFailed: createAction('server/resetPasswordFailed'),
  resetPasswordChallenge: createAction('server/resetPasswordChallenge'),
  resetPasswordSuccess: createAction('server/resetPasswordSuccess'),
  reloadConfig: createAction('server/reloadConfig'),
  shutdownServer: createAction('server/shutdownServer'),
  updateServerMessage: createAction('server/updateServerMessage'),
  accountPasswordChange: createAction('server/accountPasswordChange'),
  addToList: createAction<{ list: string; userName: string }>('server/addToList'),
  removeFromList: createAction<{ list: string; userName: string }>('server/removeFromList'),
  grantReplayAccess: createAction<{ replayId: number; moderatorName: string }>('server/grantReplayAccess'),
  forceActivateUser: createAction<{ usernameToActivate: string; moderatorName: string }>('server/forceActivateUser'),
};

export const Actions = { ...serverSlice.actions, ...SignalActions };

export type ServerAction = ReturnType<typeof Actions[keyof typeof Actions]>;
