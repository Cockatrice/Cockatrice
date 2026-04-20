import type { WebClient } from '@app/websocket';

/**
 * Creates a mock WebClient whose `request` property has vi.fn() stubs
 * for every service method that containers/forms call. Inject via a
 * vi.hoisted reference returned from a `vi.mock('@app/hooks', ...)` stub
 * of `useWebClient`; see LoginForm.spec.tsx for the canonical pattern.
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
        joinGame: vi.fn(),
      },
      game: {
        leaveGame: vi.fn(),
        kickFromGame: vi.fn(),
        gameSay: vi.fn(),
        readyStart: vi.fn(),
        concede: vi.fn(),
        unconcede: vi.fn(),
        judge: vi.fn(),
        nextTurn: vi.fn(),
        setActivePhase: vi.fn(),
        reverseTurn: vi.fn(),
        moveCard: vi.fn(),
        flipCard: vi.fn(),
        attachCard: vi.fn(),
        createToken: vi.fn(),
        setCardAttr: vi.fn(),
        setCardCounter: vi.fn(),
        incCardCounter: vi.fn(),
        drawCards: vi.fn(),
        undoDraw: vi.fn(),
        createArrow: vi.fn(),
        deleteArrow: vi.fn(),
        createCounter: vi.fn(),
        setCounter: vi.fn(),
        incCounter: vi.fn(),
        delCounter: vi.fn(),
        shuffle: vi.fn(),
        dumpZone: vi.fn(),
        revealCards: vi.fn(),
        changeZoneProperties: vi.fn(),
        deckSelect: vi.fn(),
        setSideboardPlan: vi.fn(),
        setSideboardLock: vi.fn(),
        mulligan: vi.fn(),
        rollDie: vi.fn(),
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
