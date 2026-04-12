/**
 * Shared mock factories for websocket layer unit tests.
 * Import the helpers you need in each spec file via:
 *   import { makeMockProtoRoot, makeMockWebSocket } from '../__mocks__/helpers';
 */

/** Builds a minimal mock of ProtoController.root */
export function makeMockProtoRoot() {
  const encode = { finish: jest.fn().mockReturnValue(new Uint8Array()) };
  return {
    CommandContainer: {
      create: jest.fn(args => ({ ...args })),
      encode: jest.fn().mockReturnValue(encode),
    },
    SessionCommand: { create: jest.fn(args => ({ ...args })) },
    RoomCommand:      { create: jest.fn(args => ({ ...args })) },
    ModeratorCommand: { create: jest.fn(args => ({ ...args })) },
    AdminCommand:     { create: jest.fn(args => ({ ...args })) },
    ServerMessage: {
      decode: jest.fn(),
      MessageType: {
        RESPONSE: 'RESPONSE',
        ROOM_EVENT: 'ROOM_EVENT',
        SESSION_EVENT: 'SESSION_EVENT',
        GAME_EVENT_CONTAINER: 'GAME_EVENT_CONTAINER',
      },
    },
    Response: {
      ResponseCode: {
        RespOk: 0,
        RespRegistrationRequired: 1,
      },
    },
    Event_ServerIdentification: {
      ServerOptions: { SupportsPasswordHash: 2 },
    },
    Event_ConnectionClosed: {
      CloseReason: {
        USER_LIMIT_REACHED: 1,
        TOO_MANY_CONNECTIONS: 2,
        BANNED: 3,
        DEMOTED: 4,
        SERVER_SHUTDOWN: 5,
        USERNAMEINVALID: 6,
        LOGGEDINELSEWERE: 7,
        OTHER: 8,
      },
    },
  };
}

/** Builds a mock WebSocket instance */
export function makeMockWebSocketInstance() {
  return {
    send: jest.fn(),
    close: jest.fn(),
    readyState: WebSocket.OPEN,
    binaryType: '' as BinaryType,
    onopen: null as any,
    onclose: null as any,
    onerror: null as any,
    onmessage: null as any,
  };
}

/** Installs a mock WebSocket constructor on global. Returns the mock instance. */
export function installMockWebSocket() {
  const mockInstance = makeMockWebSocketInstance();
  const MockWS = jest.fn(() => mockInstance) as any;
  MockWS.OPEN = 1;
  MockWS.CLOSED = 3;
  (global as any).WebSocket = MockWS;
  return { MockWS, mockInstance };
}
