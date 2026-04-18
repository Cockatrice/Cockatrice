import type { WebClient } from '@app/websocket';

/**
 * Creates a mock WebClient whose `request` property has vi.fn() stubs
 * for every service method that containers/forms call. Inject this into
 * tests via `renderWithProviders({ webClient: createMockWebClient() })`.
 */
export function createMockWebClient() {
  return {
    request: {
      authentication: {
        login: vi.fn(),
        register: vi.fn(),
        disconnect: vi.fn(),
        activateAccount: vi.fn(),
        resetPasswordRequest: vi.fn(),
        resetPasswordChallenge: vi.fn(),
        resetPassword: vi.fn(),
      },
      session: {
        addToBuddyList: vi.fn(),
        removeFromBuddyList: vi.fn(),
        addToIgnoreList: vi.fn(),
        removeFromIgnoreList: vi.fn(),
        getUserInfo: vi.fn(),
        accountEdit: vi.fn(),
        accountPassword: vi.fn(),
        accountImage: vi.fn(),
        listUsers: vi.fn(),
      },
      rooms: {
        joinRoom: vi.fn(),
        leaveRoom: vi.fn(),
        roomSay: vi.fn(),
        createGame: vi.fn(),
      },
      game: {
        joinGame: vi.fn(),
        leaveGame: vi.fn(),
      },
      admin: {
        adjustMod: vi.fn(),
        reloadConfig: vi.fn(),
        shutdownServer: vi.fn(),
        updateServerMessage: vi.fn(),
      },
      moderator: {
        viewLogHistory: vi.fn(),
        banFromServer: vi.fn(),
        warnUser: vi.fn(),
        warnHistory: vi.fn(),
        banHistory: vi.fn(),
      },
    },
  } as unknown as WebClient;
}
