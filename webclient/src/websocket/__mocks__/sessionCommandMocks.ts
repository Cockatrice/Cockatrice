/**
 * Shared mock shape factories for session command specs.
 *
 * Usage inside jest.mock() factory callbacks (require is used because
 * jest.mock() is hoisted above imports):
 *
 *   jest.mock('../../WebClient', () => {
 *     const { makeWebClientMock } = require('../../__mocks__/sessionCommandMocks');
 *     return { __esModule: true, default: makeWebClientMock() };
 *   });
 */

/** Superset WebClient mock — covers all properties used across both session spec files. */
export function makeWebClientMock() {
  return {
    connect: jest.fn(),
    testConnect: jest.fn(),
    disconnect: jest.fn(),
    updateStatus: jest.fn(),
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
    hashPassword: jest.fn().mockReturnValue('hashed_pw'),
    generateSalt: jest.fn().mockReturnValue('randSalt'),
    passwordSaltSupported: jest.fn().mockReturnValue(0),
  };
}

/** Superset SessionPersistence mock — covers all methods used across both session spec files. */
export function makeSessionPersistenceMock() {
  return {
    loginSuccessful: jest.fn(),
    loginFailed: jest.fn(),
    updateBuddyList: jest.fn(),
    updateIgnoreList: jest.fn(),
    updateUser: jest.fn(),
    updateUsers: jest.fn(),
    accountAwaitingActivation: jest.fn(),
    accountActivationSuccess: jest.fn(),
    accountActivationFailed: jest.fn(),
    updateStatus: jest.fn(),
    directMessageSent: jest.fn(),
    addToList: jest.fn(),
    removeFromList: jest.fn(),
    deleteServerDeck: jest.fn(),
    deleteServerDeckDir: jest.fn(),
    updateServerDecks: jest.fn(),
    uploadServerDeck: jest.fn(),
    createServerDeckDir: jest.fn(),
    getGamesOfUser: jest.fn(),
    getUserInfo: jest.fn(),
    accountPasswordChange: jest.fn(),
    accountEditChanged: jest.fn(),
    accountImageChanged: jest.fn(),
    replayList: jest.fn(),
    replayAdded: jest.fn(),
    replayModifyMatch: jest.fn(),
    replayDeleteMatch: jest.fn(),
    resetPasswordChallenge: jest.fn(),
    resetPassword: jest.fn(),
    resetPasswordFailed: jest.fn(),
    resetPasswordSuccess: jest.fn(),
    registrationFailed: jest.fn(),
    registrationSuccess: jest.fn(),
    registrationUserNameError: jest.fn(),
    registrationPasswordError: jest.fn(),
    registrationEmailError: jest.fn(),
    registrationRequiresEmail: jest.fn(),
  };
}

/**
 * Session barrel mock — pure jest.fn() map for all cross-command calls.
 * Used as-is by sessionCommands-complex.spec.ts, or spread over jest.requireActual
 * by sessionCommands-simple.spec.ts to preserve real implementations for
 * the commands under test.
 */
export function makeSessionBarrelMock() {
  return {
    login: jest.fn(),
    register: jest.fn(),
    activate: jest.fn(),
    forgotPasswordReset: jest.fn(),
    forgotPasswordRequest: jest.fn(),
    forgotPasswordChallenge: jest.fn(),
    requestPasswordSalt: jest.fn(),
    listUsers: jest.fn(),
    listRooms: jest.fn(),
    updateStatus: jest.fn(),
    disconnect: jest.fn(),
  };
}
