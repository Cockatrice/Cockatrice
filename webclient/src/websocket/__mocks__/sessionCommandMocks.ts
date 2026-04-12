/**
 * Shared mock shape factories for session command specs.
 *
 * Usage inside vi.mock() factory callbacks (require is used because
 * vi.mock() is hoisted above imports):
 *
 *   vi.mock('../../WebClient', () => {
 *     const { makeWebClientMock } = require('../../__mocks__/sessionCommandMocks');
 *     return { __esModule: true, default: makeWebClientMock() };
 *   });
 */

/** Superset WebClient mock — covers all properties used across both session spec files. */
export function makeWebClientMock() {
  return {
    connect: vi.fn(),
    testConnect: vi.fn(),
    disconnect: vi.fn(),
    updateStatus: vi.fn(),
    clientConfig: { clientid: 'webatrice', clientver: '1.0', clientfeatures: [] },
    options: {},
    protocolVersion: 14,
    status: 0,
    connectionAttemptMade: false,
  };
}

/** Superset ProtoController.root mock — includes all ResponseCode values and Event_ServerIdentification. */
export function makeProtoControllerRootMock() {
  return {
    Response: {
      ResponseCode: {
        RespOk: 0,
        RespClientUpdateRequired: 1,
        RespWrongPassword: 2,
        RespUsernameInvalid: 3,
        RespWouldOverwriteOldSession: 4,
        RespUserIsBanned: 5,
        RespRegistrationRequired: 6,
        RespClientIdRequired: 7,
        RespContextError: 8,
        RespAccountNotActivated: 9,
        RespRegistrationAccepted: 10,
        RespRegistrationAcceptedNeedsActivation: 11,
        RespUserAlreadyExists: 12,
        RespPasswordTooShort: 13,
        RespEmailRequiredToRegister: 14,
        RespEmailBlackListed: 15,
        RespTooManyRequests: 16,
        RespRegistrationDisabled: 17,
        RespActivationAccepted: 18,
      },
    },
    Event_ServerIdentification: {
      ServerOptions: { SupportsPasswordHash: 2 },
    },
  };
}

/** Utils mock with unified return values. */
export function makeUtilsMock() {
  return {
    hashPassword: vi.fn().mockReturnValue('hashed_pw'),
    generateSalt: vi.fn().mockReturnValue('randSalt'),
    passwordSaltSupported: vi.fn().mockReturnValue(0),
  };
}

/** Superset SessionPersistence mock — covers all methods used across both session spec files. */
export function makeSessionPersistenceMock() {
  return {
    loginSuccessful: vi.fn(),
    loginFailed: vi.fn(),
    updateBuddyList: vi.fn(),
    updateIgnoreList: vi.fn(),
    updateUser: vi.fn(),
    updateUsers: vi.fn(),
    accountAwaitingActivation: vi.fn(),
    accountActivationSuccess: vi.fn(),
    accountActivationFailed: vi.fn(),
    updateStatus: vi.fn(),
    addToList: vi.fn(),
    removeFromList: vi.fn(),
    deleteServerDeck: vi.fn(),
    deleteServerDeckDir: vi.fn(),
    updateServerDecks: vi.fn(),
    uploadServerDeck: vi.fn(),
    createServerDeckDir: vi.fn(),
    getGamesOfUser: vi.fn(),
    getUserInfo: vi.fn(),
    accountPasswordChange: vi.fn(),
    accountEditChanged: vi.fn(),
    accountImageChanged: vi.fn(),
    replayList: vi.fn(),
    replayAdded: vi.fn(),
    replayModifyMatch: vi.fn(),
    replayDeleteMatch: vi.fn(),
    resetPasswordChallenge: vi.fn(),
    resetPassword: vi.fn(),
    resetPasswordFailed: vi.fn(),
    resetPasswordSuccess: vi.fn(),
    registrationFailed: vi.fn(),
    registrationSuccess: vi.fn(),
    registrationUserNameError: vi.fn(),
    registrationPasswordError: vi.fn(),
    registrationEmailError: vi.fn(),
    registrationRequiresEmail: vi.fn(),
  };
}

/**
 * Session barrel mock — pure vi.fn() map for all cross-command calls.
 * Used as-is by sessionCommands-complex.spec.ts, or spread over jest.requireActual
 * by sessionCommands-simple.spec.ts to preserve real implementations for
 * the commands under test.
 */
export function makeSessionBarrelMock() {
  return {
    login: vi.fn(),
    register: vi.fn(),
    activate: vi.fn(),
    forgotPasswordReset: vi.fn(),
    forgotPasswordRequest: vi.fn(),
    forgotPasswordChallenge: vi.fn(),
    requestPasswordSalt: vi.fn(),
    listUsers: vi.fn(),
    listRooms: vi.fn(),
    updateStatus: vi.fn(),
    disconnect: vi.fn(),
  };
}
