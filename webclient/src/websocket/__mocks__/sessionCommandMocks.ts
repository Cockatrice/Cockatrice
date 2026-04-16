/**
 * Shared mock shape factories for session command specs.
 *
 * Usage inside vi.mock() factory callbacks:
 *
 *   vi.mock('../../WebClient', async () => {
 *     const { makeWebClientMock } = await import('../../__mocks__/sessionCommandMocks');
 *     return { WebClient: { instance: makeWebClientMock() } };
 *   });
 */

/** Superset WebClient instance mock — covers all properties used across both session spec files. */
export function makeWebClientMock() {
  return {
    connect: vi.fn(),
    testConnect: vi.fn(),
    disconnect: vi.fn(),
    updateStatus: vi.fn(),
    options: {},
    status: 0,
    protobuf: {
      sendSessionCommand: vi.fn(),
      sendRoomCommand: vi.fn(),
      sendGameCommand: vi.fn(),
      sendAdminCommand: vi.fn(),
      sendModeratorCommand: vi.fn(),
    },
    response: {
      session: makeSessionPersistenceMock(),
      room: { joinRoom: vi.fn() },
      game: {},
      admin: {},
      moderator: {},
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
