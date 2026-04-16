// Connection-lifecycle scenarios. Exercises the full transport handshake
// from `webClient.connect()` through `onopen`, ServerIdentification, and
// disconnect — with only the browser WebSocket constructor mocked.

import { create } from '@bufbuild/protobuf';
import { describe, expect, it } from 'vitest';

import { App, Data } from '@app/types';
import { store } from '@app/store';

import { PROTOCOL_VERSION } from '../../src/websocket/config';

import { getMockWebSocket, getWebClient, openMockWebSocket } from './helpers/setup';
import {
  buildSessionEventMessage,
  deliverMessage,
} from './helpers/protobuf-builders';
import { findLastSessionCommand } from './helpers/command-capture';

function loginOptions(overrides: Partial<{ userName: string; password: string }> = {}) {
  return {
    reason: App.WebSocketConnectReason.LOGIN,
    host: 'localhost',
    port: '4748',
    userName: overrides.userName ?? 'alice',
    password: overrides.password ?? 'secret',
  } as const;
}

function serverIdentification(
  protocolVersion = PROTOCOL_VERSION,
  serverName = 'TestServer',
  serverVersion = '2.8.0'
): Uint8Array {
  const payload = create(Data.Event_ServerIdentificationSchema, {
    serverName,
    serverVersion,
    protocolVersion,
    serverOptions: Data.Event_ServerIdentification_ServerOptions.NoOptions,
  });
  return buildSessionEventMessage(Data.Event_ServerIdentification_ext, payload);
}

describe('connection lifecycle', () => {
  it('flips status through CONNECTING → CONNECTED on socket open', () => {
    getWebClient().connect(loginOptions());

    expect(store.getState().server.status.connectionAttemptMade).toBe(true);

    openMockWebSocket();

    expect(store.getState().server.status.state).toBe(App.StatusEnum.CONNECTED);
    expect(store.getState().server.status.description).toBe('Connected');
  });

  it('routes a matching ServerIdentification into LOGGING_IN and sends Command_Login', () => {
    getWebClient().connect(loginOptions({ userName: 'alice' }));
    openMockWebSocket();

    deliverMessage(serverIdentification());

    expect(store.getState().server.status.state).toBe(App.StatusEnum.LOGGING_IN);
    expect(store.getState().server.info.name).toBe('TestServer');
    expect(store.getState().server.info.version).toBe('2.8.0');

    const { value, cmdId } = findLastSessionCommand(Data.Command_Login_ext);
    expect(value.userName).toBe('alice');
    expect(cmdId).toBeGreaterThan(0);

    expect(getWebClient().options).toBeNull();
  });

  it('disconnects on protocol version mismatch without sending a login command', () => {
    getWebClient().connect(loginOptions());
    openMockWebSocket();

    deliverMessage(serverIdentification(PROTOCOL_VERSION + 1));

    const mock = getMockWebSocket();
    expect(mock.close).toHaveBeenCalled();
    expect(store.getState().server.status.state).toBe(App.StatusEnum.DISCONNECTED);
    expect(() => findLastSessionCommand(Data.Command_Login_ext)).toThrow();
  });

  it('times out when onopen never fires within the keepalive window', () => {
    getWebClient().connect(loginOptions());

    const mock = getMockWebSocket();
    expect(mock.close).not.toHaveBeenCalled();

    vi.advanceTimersByTime(5000);

    expect(mock.close).toHaveBeenCalled();
    expect(store.getState().server.status.state).toBe(App.StatusEnum.DISCONNECTED);
  });

  it('releases keep-alive ping loop on explicit disconnect', () => {
    getWebClient().connect(loginOptions());
    openMockWebSocket();
    deliverMessage(serverIdentification());

    const mock = getMockWebSocket();
    getWebClient().disconnect();

    expect(mock.close).toHaveBeenCalled();
    expect(store.getState().server.status.state).toBe(App.StatusEnum.DISCONNECTED);
  });
});
