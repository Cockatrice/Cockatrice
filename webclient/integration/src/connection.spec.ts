// Connection-lifecycle scenarios. Exercises the full transport handshake
// from webClient.connect() through onopen, ServerIdentification, and
// disconnect — with only the browser WebSocket constructor mocked.

import { create } from '@bufbuild/protobuf';
import { describe, expect, it } from 'vitest';

import { Data } from '@app/types';
import { store } from '@app/store';
import { StatusEnum } from '@app/websocket';

import { PROTOCOL_VERSION } from '../../src/websocket/config';

import {
  getMockWebSocket,
  getWebClient,
  openMockWebSocket,
  setPendingOptions,
  connectAndHandshake,
} from './helpers/setup';
import type { WebSocketConnectOptions } from '@app/websocket';
import { WebSocketConnectReason } from '@app/websocket';
import {
  buildSessionEventMessage,
  deliverMessage,
} from './helpers/protobuf-builders';
import { findLastSessionCommand } from './helpers/command-capture';

function loginOptions(overrides: Partial<{ userName: string; password: string }> = {}): WebSocketConnectOptions {
  return {
    reason: WebSocketConnectReason.LOGIN,
    host: 'localhost',
    port: '4748',
    userName: overrides.userName ?? 'alice',
    password: overrides.password ?? 'secret',
  };
}

function connectWithOptions(opts: WebSocketConnectOptions): void {
  setPendingOptions(opts);
  getWebClient().connect({ host: opts.host, port: opts.port });
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
    connectWithOptions(loginOptions());

    expect(store.getState().server.status.connectionAttemptMade).toBe(true);

    openMockWebSocket();

    expect(store.getState().server.status.state).toBe(StatusEnum.CONNECTED);
    expect(store.getState().server.status.description).toBe('Connected');
  });

  it('routes a matching ServerIdentification into LOGGING_IN and sends Command_Login', () => {
    connectWithOptions(loginOptions({ userName: 'alice' }));
    openMockWebSocket();

    deliverMessage(serverIdentification());

    expect(store.getState().server.status.state).toBe(StatusEnum.LOGGING_IN);
    expect(store.getState().server.info.name).toBe('TestServer');
    expect(store.getState().server.info.version).toBe('2.8.0');

    const { value, cmdId } = findLastSessionCommand(Data.Command_Login_ext);
    expect(value.userName).toBe('alice');
    expect(cmdId).toBeGreaterThan(0);
  });

  it('disconnects on protocol version mismatch without sending a login command', () => {
    connectWithOptions(loginOptions());
    openMockWebSocket();

    deliverMessage(serverIdentification(PROTOCOL_VERSION + 1));

    const mock = getMockWebSocket();
    expect(mock.close).toHaveBeenCalled();
    expect(store.getState().server.status.state).toBe(StatusEnum.DISCONNECTED);
    expect(() => findLastSessionCommand(Data.Command_Login_ext)).toThrow();
  });

  it('times out when onopen never fires within the keepalive window', () => {
    connectWithOptions(loginOptions());

    const mock = getMockWebSocket();
    expect(mock.close).not.toHaveBeenCalled();

    vi.advanceTimersByTime(5000);

    expect(mock.close).toHaveBeenCalled();
    expect(store.getState().server.status.state).toBe(StatusEnum.DISCONNECTED);
  });

  it('releases keep-alive ping loop on explicit disconnect', () => {
    connectWithOptions(loginOptions());
    openMockWebSocket();
    deliverMessage(serverIdentification());

    const mock = getMockWebSocket();
    getWebClient().disconnect();

    expect(mock.close).toHaveBeenCalled();
    expect(store.getState().server.status.state).toBe(StatusEnum.DISCONNECTED);
  });

  it('drops pending commands and clears state on unexpected socket close', () => {
    connectAndHandshake();

    // A login command is now pending (sent during handshake)
    expect(() => findLastSessionCommand(Data.Command_Login_ext)).not.toThrow();

    // Simulate unexpected socket close
    const mock = getMockWebSocket();
    mock.readyState = 3;
    mock.onclose?.({ code: 1006, reason: '', wasClean: false } as CloseEvent);

    expect(store.getState().server.status.state).toBe(StatusEnum.DISCONNECTED);
  });
});