// Integration test setup — installs a mock WebSocket constructor, wires up
// fake timers for KeepAliveService control, and resets the webClient + Redux
// singletons between tests so real event handlers and reducers can run
// against a clean slate each time.
//
// Only `globalThis.WebSocket` is mocked. Everything downstream of it
// (ProtobufService, event registries, persistence, store, reducers) runs as
// real code, which is the whole point of the integration suite.

import '@testing-library/jest-dom/vitest';
import '../../../src/polyfills';

import { create } from '@bufbuild/protobuf';
import { afterEach, beforeEach, vi } from 'vitest';

import { ServerDispatch, RoomsDispatch, GameDispatch } from '@app/store';
import { App, Data, Enriched } from '@app/types';
import { WebClient } from '@app/websocket';
import { PROTOCOL_VERSION } from '../../../src/websocket/config';
import { createWebClientResponse, createWebClientRequest } from '@app/api';

import {
  buildResponse,
  buildResponseMessage,
  buildSessionEventMessage,
  deliverMessage,
} from './protobuf-builders';
import { findLastSessionCommand } from './command-capture';

export interface MockWebSocketInstance {
  send: ReturnType<typeof vi.fn>;
  close: ReturnType<typeof vi.fn>;
  readyState: number;
  binaryType: BinaryType;
  url: string;
  onopen: ((ev?: Event) => void) | null;
  onclose: ((ev?: CloseEvent) => void) | null;
  onerror: ((ev?: Event) => void) | null;
  onmessage: ((ev: MessageEvent) => void) | null;
}

let currentMockInstance: MockWebSocketInstance | null = null;

export function getMockWebSocket(): MockWebSocketInstance {
  if (!currentMockInstance) {
    throw new Error(
      'No mock WebSocket has been constructed yet. Call webClient.connect(...) before reading the mock instance.'
    );
  }
  return currentMockInstance;
}

function makeMockInstance(url: string): MockWebSocketInstance {
  return {
    send: vi.fn(),
    close: vi.fn(function close(this: MockWebSocketInstance) {
      this.readyState = 3; // CLOSED
      this.onclose?.({ code: 1000, reason: '', wasClean: true } as CloseEvent);
    }),
    readyState: 0, // CONNECTING
    binaryType: 'arraybuffer',
    url,
    onopen: null,
    onclose: null,
    onerror: null,
    onmessage: null,
  };
}

function installMockWebSocket(): void {
  const MockWS = vi.fn(function MockWebSocket(url: string) {
    currentMockInstance = makeMockInstance(url);
    return currentMockInstance;
  }) as unknown as typeof WebSocket;
  (MockWS as unknown as { CONNECTING: number }).CONNECTING = 0;
  (MockWS as unknown as { OPEN: number }).OPEN = 1;
  (MockWS as unknown as { CLOSING: number }).CLOSING = 2;
  (MockWS as unknown as { CLOSED: number }).CLOSED = 3;
  globalThis.WebSocket = MockWS;
}

export function openMockWebSocket(): void {
  const mock = getMockWebSocket();
  mock.readyState = 1; // OPEN
  mock.onopen?.(new Event('open'));
}

export function getWebClient(): WebClient {
  return WebClient.instance;
}

function resetAll(): void {
  const client = WebClient.instance;

  if (currentMockInstance && currentMockInstance.readyState === 1) {
    client.disconnect();
  }

  client.protobuf.resetCommands();
  client.options = null;
  client.status = App.StatusEnum.DISCONNECTED;

  ServerDispatch.clearStore();
  RoomsDispatch.clearStore();
  GameDispatch.clearStore();

  if (currentMockInstance) {
    currentMockInstance.onopen = null;
    currentMockInstance.onclose = null;
    currentMockInstance.onerror = null;
    currentMockInstance.onmessage = null;
    currentMockInstance = null;
  }

  (WebClient as unknown as { _instance: WebClient | null })._instance = null;
}

// ── Shared connect helpers ──────────────────────────────────────────────────

const DEFAULT_LOGIN_OPTIONS: Enriched.LoginConnectOptions = {
  reason: App.WebSocketConnectReason.LOGIN,
  host: 'localhost',
  port: '4748',
  userName: 'alice',
  password: 'secret',
};

export function connectRaw(
  overrides: Partial<Enriched.LoginConnectOptions> = {}
): void {
  getWebClient().connect({ ...DEFAULT_LOGIN_OPTIONS, ...overrides });
  openMockWebSocket();
}

export function connectAndHandshake(
  overrides: Partial<Enriched.LoginConnectOptions> = {}
): void {
  connectRaw(overrides);
  deliverMessage(buildSessionEventMessage(
    Data.Event_ServerIdentification_ext,
    create(Data.Event_ServerIdentificationSchema, {
      serverName: 'TestServer',
      serverVersion: '2.8.0',
      protocolVersion: PROTOCOL_VERSION,
    })
  ));
}

export function connectAndLogin(userName: string = 'alice'): void {
  connectAndHandshake({ userName });

  const login = findLastSessionCommand(Data.Command_Login_ext);
  const userInfo = create(Data.ServerInfo_UserSchema, {
    name: userName,
    userLevel: Data.ServerInfo_User_UserLevelFlag.IsRegistered,
  });
  deliverMessage(buildResponseMessage(buildResponse({
    cmdId: login.cmdId,
    responseCode: Data.Response_ResponseCode.RespOk,
    ext: Data.Response_Login_ext,
    value: create(Data.Response_LoginSchema, {
      userInfo,
      buddyList: [],
      ignoreList: [],
    }),
  })));
}

// ── Lifecycle hooks ─────────────────────────────────────────────────────────

installMockWebSocket();

beforeEach(() => {
  vi.useFakeTimers();
  new WebClient(createWebClientResponse(), createWebClientRequest());
});

afterEach(() => {
  resetAll();
  vi.clearAllMocks();
  vi.useRealTimers();
});
