// Moderator command pipeline smoke tests — validates that sendModeratorCommand
// encodes, correlates, and persists correctly end-to-end. One test per
// distinct response pattern (simple vs. extension-payload).

import { create } from '@bufbuild/protobuf';
import { describe, expect, it } from 'vitest';

import { Data } from '@app/types';
import { store } from '@app/store';
import { ModeratorCommands } from '@app/websocket';

import { connectAndLogin } from '../helpers/setup';
import {
  buildResponse,
  buildResponseMessage,
  deliverMessage,
} from '../helpers/protobuf-builders';
import { findLastModeratorCommand } from '../helpers/command-capture';

describe('moderator commands', () => {
  it('getBanHistory populates server.banHistory on success', () => {
    connectAndLogin();

    ModeratorCommands.getBanHistory('baduser');

    const { cmdId, value } = findLastModeratorCommand(Data.Command_GetBanHistory_ext);
    expect(value.userName).toBe('baduser');

    const banEntry = create(Data.ServerInfo_BanSchema, {
      adminId: 'admin1',
      adminName: 'Admin',
      banTime: '2026-01-01',
      banLength: '60',
      visibleReason: 'spamming',
    });
    deliverMessage(buildResponseMessage(buildResponse({
      cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
      ext: Data.Response_BanHistory_ext,
      value: create(Data.Response_BanHistorySchema, { banList: [banEntry] }),
    })));

    const history = store.getState().server.banHistory.baduser;
    expect(history).toHaveLength(1);
    expect(history[0].visibleReason).toBe('spamming');
  });

  it('viewLogHistory populates server.logs on success', () => {
    connectAndLogin();

    ModeratorCommands.viewLogHistory({ dateRange: 30 });

    const { cmdId } = findLastModeratorCommand(Data.Command_ViewLogHistory_ext);

    const logMsg = create(Data.ServerInfo_ChatMessageSchema, {
      senderName: 'alice',
      message: 'test message',
    });
    deliverMessage(buildResponseMessage(buildResponse({
      cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
      ext: Data.Response_ViewLogHistory_ext,
      value: create(Data.Response_ViewLogHistorySchema, { logMessage: [logMsg] }),
    })));

    const logs = store.getState().server.logs;
    expect(Object.keys(logs).length).toBeGreaterThan(0);
  });

  it('warnUser sends command and updates state on success', () => {
    connectAndLogin();

    ModeratorCommands.warnUser('troublemaker', 'spamming chat');

    const { cmdId, value } = findLastModeratorCommand(Data.Command_WarnUser_ext);
    expect(value.userName).toBe('troublemaker');
    expect(value.reason).toBe('spamming chat');

    deliverMessage(buildResponseMessage(buildResponse({
      cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
    })));

    expect(store.getState().server.warnUser).toBe('troublemaker');
  });

  it('banFromServer sends command and updates state on success', () => {
    connectAndLogin();

    ModeratorCommands.banFromServer(60, 'baduser', undefined, 'repeated offenses', 'rule violation');

    const { cmdId, value } = findLastModeratorCommand(Data.Command_BanFromServer_ext);
    expect(value.userName).toBe('baduser');
    expect(value.minutes).toBe(60);
    expect(value.visibleReason).toBe('rule violation');

    deliverMessage(buildResponseMessage(buildResponse({
      cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
    })));

    expect(store.getState().server.banUser).toBe('baduser');
  });
});