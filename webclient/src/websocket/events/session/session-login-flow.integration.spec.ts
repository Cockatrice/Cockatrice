// Integration coverage of the full login chain through the real session
// event handler + real session commands. Exercises:
//   serverIdentification(info)
//     → requestPasswordSalt (real)
//       → login (real)
//         → WebClient.instance.response.session.loginSuccessful(...)
//
// Only the transport (sendSessionCommand), the connection-state shim, and the
// password-hash bitmask helper are mocked — everything in between is the
// production code path. This triangulates the "loginSuccessful payload is
// empty" symptom reported in plans/when-the-login-quirky-crane.md.

vi.mock('../../WebClient');

vi.mock('../../config', () => ({
  CLIENT_CONFIG: { clientver: 'test-client', clientfeatures: [] },
  CLIENT_OPTIONS: { autojoinrooms: false, keepalive: 5000 },
  PROTOCOL_VERSION: 14,
}));

vi.mock('../../utils/connectionState', () => ({
  consumePendingOptions: vi.fn().mockReturnValue(null),
}));

// Only the bitmask check is mocked; hashPassword stays real so the test
// asserts against a genuinely computed hash (not a sentinel string).
vi.mock('../../utils', async (importOriginal) => ({
  ...(await importOriginal<typeof import('../../utils')>()),
  passwordSaltSupported: vi.fn().mockReturnValue(0),
}));

import { create } from '@bufbuild/protobuf';
import { Mock } from 'vitest';
import { Event_ServerIdentificationSchema } from '@app/generated';

import { WebClient } from '../../WebClient';
import { consumePendingOptions } from '../../utils/connectionState';
import { passwordSaltSupported, hashPassword } from '../../utils';
import { WebSocketConnectReason } from '../../types/ConnectOptions';
import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import { serverIdentification } from './serverIdentification';

const { invokeOnSuccess } = makeCallbackHelpers(
  WebClient.instance.protobuf.sendSessionCommand as Mock,
  2,
);

const makeLoginOptions = () => ({
  host: 'h',
  port: '1',
  userName: 'alice',
  password: 'mypass',
  reason: WebSocketConnectReason.LOGIN as const,
});

const makeInfo = (overrides: Record<string, unknown> = {}) =>
  create(Event_ServerIdentificationSchema, {
    serverName: 'TestServer',
    serverVersion: '1.0',
    protocolVersion: 14,
    serverOptions: 1,
    ...overrides,
  });

beforeEach(() => {
  (WebClient.instance.protobuf.sendSessionCommand as Mock).mockClear();
  (WebClient.instance.response.session.loginSuccessful as Mock).mockClear();
  (WebClient.instance.response.session.loginFailed as Mock).mockClear();
  (WebClient.instance.response.session.updateStatus as Mock).mockClear();
  (consumePendingOptions as Mock).mockReset();
  (passwordSaltSupported as Mock).mockReset();
});

describe('integration: fresh login on a hashed-password server', () => {
  it('loginSuccessful carries a non-empty hashedPassword matching hashPassword(salt, password)', () => {
    (consumePendingOptions as Mock).mockReturnValue(makeLoginOptions());
    (passwordSaltSupported as Mock).mockReturnValue(1);

    // 1. Server identifies with the hashed-password bit set.
    serverIdentification(makeInfo({ serverOptions: 1 }));

    // 2. RequestPasswordSalt command was sent; resolve it with a salt.
    invokeOnSuccess({ passwordSalt: 'xyz' });

    // 3. Login command was sent; resolve it with a minimal success response.
    invokeOnSuccess({ buddyList: [], ignoreList: [], userInfo: { name: 'alice' } });

    // 4. The response layer must receive the SAME hash the client computed.
    const expected = hashPassword('xyz', 'mypass');
    expect(WebClient.instance.response.session.loginSuccessful).toHaveBeenCalledTimes(1);
    expect(WebClient.instance.response.session.loginSuccessful).toHaveBeenCalledWith({
      hashedPassword: expected,
    });
    expect(expected.length).toBeGreaterThan(16);
  });

  it('Command_Login carries hashedPassword and no plaintext password value', () => {
    (consumePendingOptions as Mock).mockReturnValue(makeLoginOptions());
    (passwordSaltSupported as Mock).mockReturnValue(1);

    serverIdentification(makeInfo({ serverOptions: 1 }));
    invokeOnSuccess({ passwordSalt: 'xyz' });

    // Second sendSessionCommand call is the Command_Login. Protobuf always
    // materializes the `password` field (proto-default ""), so we assert on
    // the VALUE, not the presence: hashedPassword holds the real hash and
    // plaintext password is empty.
    const calls = (WebClient.instance.protobuf.sendSessionCommand as Mock).mock.calls;
    const loginPayload = calls[1]?.[1];
    expect(loginPayload.hashedPassword).toBe(hashPassword('xyz', 'mypass'));
    expect(loginPayload.password).toBe('');
  });
});

describe('integration: server returns empty passwordSalt (effectively unsupported)', () => {
  it('falls back to a plain-password Command_Login', () => {
    (consumePendingOptions as Mock).mockReturnValue(makeLoginOptions());
    (passwordSaltSupported as Mock).mockReturnValue(1);

    serverIdentification(makeInfo({ serverOptions: 1 }));
    // Server's Response_PasswordSalt has the proto default "" — simulate that.
    invokeOnSuccess({ passwordSalt: '' });

    const calls = (WebClient.instance.protobuf.sendSessionCommand as Mock).mock.calls;
    // Two commands: RequestPasswordSalt, then the fallback plain Command_Login.
    expect(calls.length).toBeGreaterThanOrEqual(2);
    const loginPayload = calls[1][1];
    expect(loginPayload.password).toBe('mypass');
    expect(loginPayload.hashedPassword).toBe('');
  });

  it('does not surface as a login failure', () => {
    (consumePendingOptions as Mock).mockReturnValue(makeLoginOptions());
    (passwordSaltSupported as Mock).mockReturnValue(1);

    serverIdentification(makeInfo({ serverOptions: 1 }));
    invokeOnSuccess({ passwordSalt: '' });

    // Missing salt is treated as "unsupported", not a failure — the client
    // should let the plain login proceed without dispatching loginFailed.
    expect(WebClient.instance.response.session.loginFailed).not.toHaveBeenCalled();
  });
});
