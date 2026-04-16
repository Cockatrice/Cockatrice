// Authentication scenarios — login success/failure, register, and activate.

import { create } from '@bufbuild/protobuf';
import { describe, expect, it } from 'vitest';

import { App, Data } from '@app/types';
import { store } from '@app/store';

import { connectAndHandshake } from './helpers/setup';
import {
  buildResponse,
  buildResponseMessage,
  deliverMessage,
} from './helpers/protobuf-builders';
import { findLastSessionCommand } from './helpers/command-capture';

function makeUser(name: string): Data.ServerInfo_User {
  return create(Data.ServerInfo_UserSchema, {
    name,
    userLevel: Data.ServerInfo_User_UserLevelFlag.IsRegistered,
  });
}

describe('authentication', () => {
  describe('login', () => {
    it('drives LOGIN → LOGGED_IN and populates user info + buddy/ignore lists', () => {
      connectAndHandshake({ userName: 'alice' });

      const { cmdId, value } = findLastSessionCommand(Data.Command_Login_ext);
      expect(value.userName).toBe('alice');

      const loginPayload = create(Data.Response_LoginSchema, {
        userInfo: makeUser('alice'),
        buddyList: [makeUser('bob')],
        ignoreList: [makeUser('mallory')],
      });
      deliverMessage(buildResponseMessage(buildResponse({
        cmdId,
        responseCode: Data.Response_ResponseCode.RespOk,
        ext: Data.Response_Login_ext,
        value: loginPayload,
      })));

      const state = store.getState().server;
      expect(state.status.state).toBe(App.StatusEnum.LOGGED_IN);
      expect(state.status.description).toBe('Logged in.');
      expect(state.user?.name).toBe('alice');
      expect(Object.keys(state.buddyList)).toEqual(['bob']);
      expect(Object.keys(state.ignoreList)).toEqual(['mallory']);

      expect(() => findLastSessionCommand(Data.Command_ListUsers_ext)).not.toThrow();
      expect(() => findLastSessionCommand(Data.Command_ListRooms_ext)).not.toThrow();
    });

    it('flips status to DISCONNECTED on RespWrongPassword', () => {
      connectAndHandshake();

      const { cmdId } = findLastSessionCommand(Data.Command_Login_ext);
      deliverMessage(buildResponseMessage(buildResponse({
        cmdId,
        responseCode: Data.Response_ResponseCode.RespWrongPassword,
      })));

      const state = store.getState().server;
      expect(state.status.state).toBe(App.StatusEnum.DISCONNECTED);
      expect(state.user).toBeNull();
      expect(state.buddyList).toEqual({});
    });
  });

  describe('register', () => {
    const registerOptions = {
      reason: App.WebSocketConnectReason.REGISTER,
      host: 'localhost',
      port: '4748',
      userName: 'newbie',
      password: 'hunter2',
      email: 'newbie@example.com',
      country: 'US',
      realName: 'New Bie',
    } as const;

    it('auto-logs-in on RespRegistrationAccepted', () => {
      connectAndHandshake(registerOptions as any);

      const register = findLastSessionCommand(Data.Command_Register_ext);
      expect(register.value.userName).toBe('newbie');

      deliverMessage(buildResponseMessage(buildResponse({
        cmdId: register.cmdId,
        responseCode: Data.Response_ResponseCode.RespRegistrationAccepted,
      })));

      const login = findLastSessionCommand(Data.Command_Login_ext);
      expect(login.value.userName).toBe('newbie');
      expect(login.cmdId).toBeGreaterThan(register.cmdId);
    });

    it('parks registration in awaiting-activation on RespRegistrationAcceptedNeedsActivation', () => {
      connectAndHandshake(registerOptions as any);

      const register = findLastSessionCommand(Data.Command_Register_ext);
      deliverMessage(buildResponseMessage(buildResponse({
        cmdId: register.cmdId,
        responseCode: Data.Response_ResponseCode.RespRegistrationAcceptedNeedsActivation,
      })));

      expect(store.getState().server.status.state).toBe(App.StatusEnum.DISCONNECTED);
      expect(() => findLastSessionCommand(Data.Command_Login_ext)).toThrow();
    });
  });

  describe('activate', () => {
    it('auto-logs-in on RespActivationAccepted', () => {
      connectAndHandshake({
        reason: App.WebSocketConnectReason.ACTIVATE_ACCOUNT,
        host: 'localhost',
        port: '4748',
        userName: 'alice',
        token: 'abc-123',
        password: 'secret',
      } as any);

      const activate = findLastSessionCommand(Data.Command_Activate_ext);
      expect(activate.value.userName).toBe('alice');

      deliverMessage(buildResponseMessage(buildResponse({
        cmdId: activate.cmdId,
        responseCode: Data.Response_ResponseCode.RespActivationAccepted,
      })));

      const login = findLastSessionCommand(Data.Command_Login_ext);
      expect(login.value.userName).toBe('alice');
    });
  });
});
