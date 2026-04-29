/**
 * Shared WebClient mock — the single source of truth for all websocket
 * layer unit tests.
 *
 * Vitest resolves this file whenever a spec calls `vi.mock('...WebClient')`
 * without providing a factory.  Each spec file gets its own module graph
 * (isolate: true), so there are no factory-conflict issues.
 *
 * Usage in spec files:
 *
 *   vi.mock('../../WebClient');
 *   import { WebClient } from '../../WebClient';
 *   // WebClient.instance.response.game.cardMoved   ← vi.fn()
 *   // WebClient.instance.protobuf.sendGameCommand   ← vi.fn()
 *
 * `useWebClientCleanup()` is NOT required — `instance` is a plain
 * property, not a getter that throws.
 */

const session = {
  initialized: vi.fn(),
  connectionAttempted: vi.fn(),
  clearStore: vi.fn(),
  loginSuccessful: vi.fn(),
  loginFailed: vi.fn(),
  connectionFailed: vi.fn(),
  testConnectionSuccessful: vi.fn(),
  testConnectionFailed: vi.fn(),
  updateBuddyList: vi.fn(),
  addToBuddyList: vi.fn(),
  removeFromBuddyList: vi.fn(),
  updateIgnoreList: vi.fn(),
  addToIgnoreList: vi.fn(),
  removeFromIgnoreList: vi.fn(),
  updateInfo: vi.fn(),
  updateStatus: vi.fn(),
  updateUser: vi.fn(),
  updateUsers: vi.fn(),
  userJoined: vi.fn(),
  userLeft: vi.fn(),
  serverMessage: vi.fn(),
  accountAwaitingActivation: vi.fn(),
  accountActivationSuccess: vi.fn(),
  accountActivationFailed: vi.fn(),
  registrationRequiresEmail: vi.fn(),
  registrationSuccess: vi.fn(),
  registrationFailed: vi.fn(),
  registrationEmailError: vi.fn(),
  registrationPasswordError: vi.fn(),
  registrationUserNameError: vi.fn(),
  resetPasswordChallenge: vi.fn(),
  resetPassword: vi.fn(),
  resetPasswordSuccess: vi.fn(),
  resetPasswordFailed: vi.fn(),
  accountPasswordChange: vi.fn(),
  accountEditChanged: vi.fn(),
  accountImageChanged: vi.fn(),
  getUserInfo: vi.fn(),
  getGamesOfUser: vi.fn(),
  gameJoined: vi.fn(),
  notifyUser: vi.fn(),
  playerPropertiesChanged: vi.fn(),
  serverShutdown: vi.fn(),
  userMessage: vi.fn(),
  addToList: vi.fn(),
  removeFromList: vi.fn(),
  deleteServerDeck: vi.fn(),
  updateServerDecks: vi.fn(),
  uploadServerDeck: vi.fn(),
  downloadServerDeck: vi.fn(),
  createServerDeckDir: vi.fn(),
  deleteServerDeckDir: vi.fn(),
  replayList: vi.fn(),
  replayAdded: vi.fn(),
  replayModifyMatch: vi.fn(),
  replayDeleteMatch: vi.fn(),
  replayDownloaded: vi.fn(),
};

const room = {
  clearStore: vi.fn(),
  joinRoom: vi.fn(),
  leaveRoom: vi.fn(),
  updateRooms: vi.fn(),
  updateGames: vi.fn(),
  addMessage: vi.fn(),
  userJoined: vi.fn(),
  userLeft: vi.fn(),
  removeMessages: vi.fn(),
  gameCreated: vi.fn(),
  joinedGame: vi.fn(),
  setJoinGamePending: vi.fn(),
  setJoinGameError: vi.fn(),
};

const game = {
  clearStore: vi.fn(),
  gameStateChanged: vi.fn(),
  playerJoined: vi.fn(),
  playerLeft: vi.fn(),
  playerPropertiesChanged: vi.fn(),
  gameClosed: vi.fn(),
  gameHostChanged: vi.fn(),
  kicked: vi.fn(),
  gameSay: vi.fn(),
  cardMoved: vi.fn(),
  cardFlipped: vi.fn(),
  cardDestroyed: vi.fn(),
  cardAttached: vi.fn(),
  tokenCreated: vi.fn(),
  cardAttrChanged: vi.fn(),
  cardCounterChanged: vi.fn(),
  arrowCreated: vi.fn(),
  arrowDeleted: vi.fn(),
  counterCreated: vi.fn(),
  counterSet: vi.fn(),
  counterDeleted: vi.fn(),
  cardsDrawn: vi.fn(),
  cardsRevealed: vi.fn(),
  zoneShuffled: vi.fn(),
  dieRolled: vi.fn(),
  activePlayerSet: vi.fn(),
  activePhaseSet: vi.fn(),
  turnReversed: vi.fn(),
  zoneDumped: vi.fn(),
  zonePropertiesChanged: vi.fn(),
};

const admin = {
  adjustMod: vi.fn(),
  reloadConfig: vi.fn(),
  shutdownServer: vi.fn(),
  updateServerMessage: vi.fn(),
};

const moderator = {
  banFromServer: vi.fn(),
  banHistory: vi.fn(),
  viewLogs: vi.fn(),
  warnHistory: vi.fn(),
  warnListOptions: vi.fn(),
  warnUser: vi.fn(),
  grantReplayAccess: vi.fn(),
  forceActivateUser: vi.fn(),
  getAdminNotes: vi.fn(),
  updateAdminNotes: vi.fn(),
};

export const WebClient = {
  _instance: null as any,
  instance: {
    connect: vi.fn(),
    testConnect: vi.fn(),
    disconnect: vi.fn(),
    updateStatus: vi.fn(),
    status: 0 as number,
    config: {},
    protobuf: {
      sendSessionCommand: vi.fn(),
      sendRoomCommand: vi.fn(),
      sendGameCommand: vi.fn(),
      sendAdminCommand: vi.fn(),
      sendModeratorCommand: vi.fn(),
      resetCommands: vi.fn(),
    },
    response: { session, room, game, admin, moderator },
  },
};

