// User-list and social scenarios — user presence, buddy/ignore lists, DMs.

import { create } from '@bufbuild/protobuf';
import { describe, expect, it } from 'vitest';

import { Data } from '@app/types';
import { store } from '@app/store';

import { connectAndLogin } from '../helpers/setup';
import {
  buildResponse,
  buildResponseMessage,
  buildSessionEventMessage,
  deliverMessage,
} from '../helpers/protobuf-builders';
import { findLastSessionCommand } from '../helpers/command-capture';

function makeUser(name: string): Data.ServerInfo_User {
  return create(Data.ServerInfo_UserSchema, {
    name,
    userLevel: Data.ServerInfo_User_UserLevelFlag.IsRegistered,
  });
}

describe('users', () => {
  it('populates state.users from the Response_ListUsers post-login', () => {
    connectAndLogin();

    const listUsers = findLastSessionCommand(Data.Command_ListUsers_ext);
    const users = [makeUser('alice'), makeUser('bob'), makeUser('carol')];
    deliverMessage(buildResponseMessage(buildResponse({
      cmdId: listUsers.cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
      ext: Data.Response_ListUsers_ext,
      value: create(Data.Response_ListUsersSchema, { userList: users }),
    })));

    expect(Object.keys(store.getState().server.users).sort()).toEqual(['alice', 'bob', 'carol']);
  });

  it('appends on Event_UserJoined and removes on Event_UserLeft', () => {
    connectAndLogin();

    deliverMessage(buildSessionEventMessage(
      Data.Event_UserJoined_ext,
      create(Data.Event_UserJoinedSchema, { userInfo: makeUser('bob') })
    ));
    expect('bob' in store.getState().server.users).toBe(true);

    deliverMessage(buildSessionEventMessage(
      Data.Event_UserLeft_ext,
      create(Data.Event_UserLeftSchema, { name: 'bob' })
    ));
    expect('bob' in store.getState().server.users).toBe(false);
  });

  it('adds a user to buddyList on Event_AddToList with listName=buddy', () => {
    connectAndLogin();

    deliverMessage(buildSessionEventMessage(
      Data.Event_AddToList_ext,
      create(Data.Event_AddToListSchema, {
        listName: 'buddy',
        userInfo: makeUser('bob'),
      })
    ));

    expect('bob' in store.getState().server.buddyList).toBe(true);
    expect(store.getState().server.ignoreList).toEqual({});
  });

  it('adds a user to ignoreList on Event_AddToList with listName=ignore', () => {
    connectAndLogin();

    deliverMessage(buildSessionEventMessage(
      Data.Event_AddToList_ext,
      create(Data.Event_AddToListSchema, {
        listName: 'ignore',
        userInfo: makeUser('mallory'),
      })
    ));

    expect('mallory' in store.getState().server.ignoreList).toBe(true);
    expect(store.getState().server.buddyList).toEqual({});
  });

  it('files an incoming direct message under the sender', () => {
    connectAndLogin('alice');

    deliverMessage(buildSessionEventMessage(
      Data.Event_UserMessage_ext,
      create(Data.Event_UserMessageSchema, {
        senderName: 'bob',
        receiverName: 'alice',
        message: 'hi alice',
      })
    ));

    const { messages } = store.getState().server;
    expect(messages.bob).toHaveLength(1);
    expect(messages.bob[0].message).toBe('hi alice');
  });

  it('files an outgoing direct message under the recipient', () => {
    connectAndLogin('alice');

    deliverMessage(buildSessionEventMessage(
      Data.Event_UserMessage_ext,
      create(Data.Event_UserMessageSchema, {
        senderName: 'alice',
        receiverName: 'bob',
        message: 'hey bob',
      })
    ));

    const { messages } = store.getState().server;
    expect(messages.bob).toHaveLength(1);
    expect(messages.bob[0].message).toBe('hey bob');
  });
});