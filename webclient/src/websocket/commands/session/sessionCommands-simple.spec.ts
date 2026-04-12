// Shared mock setup for session command tests

jest.mock('../../services/BackendService', () => ({
  BackendService: {
    sendSessionCommand: jest.fn(),
  },
}));

jest.mock('../../persistence', () => {
  const { makeSessionPersistenceMock } = require('../../__mocks__/sessionCommandMocks');
  return {
    SessionPersistence: makeSessionPersistenceMock(),
    RoomPersistence: { joinRoom: jest.fn() },
  };
});

jest.mock('../../WebClient', () => {
  const { makeWebClientMock } = require('../../__mocks__/sessionCommandMocks');
  return { __esModule: true, default: makeWebClientMock() };
});

jest.mock('../../services/ProtoController', () => {
  const { makeProtoControllerRootMock } = require('../../__mocks__/sessionCommandMocks');
  return { ProtoController: { root: makeProtoControllerRootMock() } };
});

jest.mock('../../utils', () => {
  const { makeUtilsMock } = require('../../__mocks__/sessionCommandMocks');
  return makeUtilsMock();
});

// Mock session commands barrel to allow cross-command calls while keeping real implementations
jest.mock('./', () => {
  const actual = jest.requireActual('./');
  const { makeSessionBarrelMock } = require('../../__mocks__/sessionCommandMocks');
  return { ...actual, ...makeSessionBarrelMock() };
});

import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';
import { RoomPersistence } from '../../persistence';
import webClient from '../../WebClient';
import * as SessionCommands from './';
import { hashPassword, generateSalt, passwordSaltSupported } from '../../utils';

const { invokeOnSuccess, invokeCallback } = makeCallbackHelpers(
  BackendService.sendSessionCommand as jest.Mock
);

beforeEach(() => {
  jest.clearAllMocks();
  (hashPassword as jest.Mock).mockReturnValue('hashed_pw');
  (generateSalt as jest.Mock).mockReturnValue('randSalt');
  (passwordSaltSupported as jest.Mock).mockReturnValue(0);
});

// ----------------------------------------------------------------

describe('accountEdit', () => {
  const { accountEdit } = jest.requireActual('./accountEdit');
  beforeEach(() => jest.clearAllMocks());

  it('sends Command_AccountEdit with correct params', () => {
    accountEdit('pw', 'Alice', 'a@b.com', 'US');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      'Command_AccountEdit',
      { passwordCheck: 'pw', realName: 'Alice', email: 'a@b.com', country: 'US' },
      expect.any(Object)
    );
  });

  it('calls SessionPersistence.accountEditChanged on success', () => {
    accountEdit('pw', 'Alice', 'a@b.com', 'US');
    invokeOnSuccess();
    expect(SessionPersistence.accountEditChanged).toHaveBeenCalledWith('Alice', 'a@b.com', 'US');
  });
});

describe('accountImage', () => {
  const { accountImage } = jest.requireActual('./accountImage');
  beforeEach(() => jest.clearAllMocks());

  it('sends Command_AccountImage', () => {
    const img = new Uint8Array([1, 2]);
    accountImage(img);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith('Command_AccountImage', { image: img }, expect.any(Object));
  });

  it('calls SessionPersistence.accountImageChanged on success', () => {
    const img = new Uint8Array([1, 2]);
    accountImage(img);
    invokeOnSuccess();
    expect(SessionPersistence.accountImageChanged).toHaveBeenCalledWith(img);
  });
});

describe('accountPassword', () => {
  const { accountPassword } = jest.requireActual('./accountPassword');
  beforeEach(() => jest.clearAllMocks());

  it('sends Command_AccountPassword', () => {
    accountPassword('old', 'new', 'hashed');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      'Command_AccountPassword',
      { oldPassword: 'old', newPassword: 'new', hashedNewPassword: 'hashed' },
      expect.any(Object)
    );
  });

  it('calls SessionPersistence.accountPasswordChange on success', () => {
    accountPassword('old', 'new', 'hashed');
    invokeOnSuccess();
    expect(SessionPersistence.accountPasswordChange).toHaveBeenCalled();
  });
});

describe('deckDel', () => {
  const { deckDel } = jest.requireActual('./deckDel');
  beforeEach(() => jest.clearAllMocks());

  it('sends Command_DeckDel', () => {
    deckDel(42);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith('Command_DeckDel', { deckId: 42 }, expect.any(Object));
  });

  it('calls deleteServerDeck on success', () => {
    deckDel(42);
    invokeOnSuccess();
    expect(SessionPersistence.deleteServerDeck).toHaveBeenCalledWith(42);
  });
});

describe('deckDelDir', () => {
  const { deckDelDir } = jest.requireActual('./deckDelDir');
  beforeEach(() => jest.clearAllMocks());

  it('sends Command_DeckDelDir', () => {
    deckDelDir('/path');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith('Command_DeckDelDir', { path: '/path' }, expect.any(Object));
  });

  it('calls deleteServerDeckDir on success', () => {
    deckDelDir('/path');
    invokeOnSuccess();
    expect(SessionPersistence.deleteServerDeckDir).toHaveBeenCalledWith('/path');
  });
});

describe('deckList', () => {
  const { deckList } = jest.requireActual('./deckList');
  beforeEach(() => jest.clearAllMocks());

  it('sends Command_DeckList', () => {
    deckList();
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith('Command_DeckList', {}, expect.any(Object));
  });

  it('calls updateServerDecks on success', () => {
    deckList();
    const resp = { folders: [] };
    invokeOnSuccess(resp, { responseCode: 0, '.Response_DeckList.ext': resp });
    expect(SessionPersistence.updateServerDecks).toHaveBeenCalledWith(resp);
  });
});

describe('deckNewDir', () => {
  const { deckNewDir } = jest.requireActual('./deckNewDir');
  beforeEach(() => jest.clearAllMocks());

  it('sends Command_DeckNewDir', () => {
    deckNewDir('/path', 'dir');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      'Command_DeckNewDir', { path: '/path', dirName: 'dir' }, expect.any(Object)
    );
  });

  it('calls createServerDeckDir on success', () => {
    deckNewDir('/path', 'dir');
    invokeOnSuccess();
    expect(SessionPersistence.createServerDeckDir).toHaveBeenCalledWith('/path', 'dir');
  });
});

describe('deckUpload', () => {
  const { deckUpload } = jest.requireActual('./deckUpload');
  beforeEach(() => jest.clearAllMocks());

  it('sends Command_DeckUpload', () => {
    deckUpload('/path', 1, 'content');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      'Command_DeckUpload',
      { path: '/path', deckId: 1, deckList: 'content' },
      expect.any(Object)
    );
  });

  it('calls uploadServerDeck on success', () => {
    deckUpload('/path', 1, 'content');
    const resp = { newFile: { id: 1 } };
    invokeOnSuccess(resp, { responseCode: 0, '.Response_DeckUpload.ext': resp });
    expect(SessionPersistence.uploadServerDeck).toHaveBeenCalledWith('/path', resp.newFile);
  });
});

describe('disconnect', () => {
  const { disconnect } = jest.requireActual('./disconnect');
  beforeEach(() => jest.clearAllMocks());

  it('calls webClient.disconnect', () => {
    disconnect();
    expect(webClient.disconnect).toHaveBeenCalled();
  });
});

describe('getGamesOfUser', () => {
  const { getGamesOfUser } = jest.requireActual('./getGamesOfUser');
  beforeEach(() => jest.clearAllMocks());

  it('sends Command_GetGamesOfUser', () => {
    getGamesOfUser('alice');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith('Command_GetGamesOfUser', { userName: 'alice' }, expect.any(Object));
  });

  it('calls getGamesOfUser on success', () => {
    getGamesOfUser('alice');
    const resp = { gameList: [] };
    invokeOnSuccess(resp, { responseCode: 0, '.Response_GetGamesOfUser.ext': resp });
    expect(SessionPersistence.getGamesOfUser).toHaveBeenCalledWith('alice', resp);
  });
});

describe('getUserInfo', () => {
  const { getUserInfo } = jest.requireActual('./getUserInfo');
  beforeEach(() => jest.clearAllMocks());

  it('sends Command_GetUserInfo', () => {
    getUserInfo('alice');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith('Command_GetUserInfo', { userName: 'alice' }, expect.any(Object));
  });

  it('calls getUserInfo on success', () => {
    getUserInfo('alice');
    const resp = { userInfo: { name: 'alice' } };
    invokeOnSuccess(resp, { responseCode: 0, '.Response_GetUserInfo.ext': resp });
    expect(SessionPersistence.getUserInfo).toHaveBeenCalledWith(resp.userInfo);
  });
});

describe('joinRoom', () => {
  const { joinRoom } = jest.requireActual('./joinRoom');
  beforeEach(() => jest.clearAllMocks());

  it('sends Command_JoinRoom', () => {
    joinRoom(5);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith('Command_JoinRoom', { roomId: 5 }, expect.any(Object));
  });

  it('calls RoomPersistence.joinRoom on success', () => {
    joinRoom(5);
    const resp = { roomInfo: { roomId: 5 } };
    invokeOnSuccess(resp, { responseCode: 0, '.Response_JoinRoom.ext': resp });
    expect(RoomPersistence.joinRoom).toHaveBeenCalledWith(resp.roomInfo);
  });
});

describe('listRooms (command)', () => {
  const { listRooms } = jest.requireActual('./listRooms');
  beforeEach(() => jest.clearAllMocks());

  it('sends Command_ListRooms', () => {
    listRooms();
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith('Command_ListRooms', {}, {});
  });
});

describe('listUsers', () => {
  const { listUsers } = jest.requireActual('./listUsers');
  beforeEach(() => jest.clearAllMocks());

  it('sends Command_ListUsers', () => {
    listUsers();
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith('Command_ListUsers', {}, expect.any(Object));
  });

  it('calls SessionPersistence.updateUsers with the user list on success', () => {
    listUsers();
    const resp = { userList: [{ name: 'Alice' }] };
    invokeOnSuccess(resp, { responseCode: 0, '.Response_ListUsers.ext': resp });
    expect(SessionPersistence.updateUsers).toHaveBeenCalledWith([{ name: 'Alice' }]);
  });
});

describe('message', () => {
  const { message } = jest.requireActual('./message');
  beforeEach(() => jest.clearAllMocks());

  it('sends Command_Message', () => {
    message('bob', 'hi');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      'Command_Message', { userName: 'bob', message: 'hi' }, expect.any(Object)
    );
  });

});

describe('ping', () => {
  const { ping } = jest.requireActual('./ping');
  beforeEach(() => jest.clearAllMocks());

  it('sends Command_Ping', () => {
    const pingReceived = jest.fn();
    ping(pingReceived);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith('Command_Ping', {}, expect.any(Object));
  });

  it('calls pingReceived via onResponse', () => {
    const pingReceived = jest.fn();
    ping(pingReceived);
    const raw = {};
    invokeCallback('onResponse', raw);
    expect(pingReceived).toHaveBeenCalledWith(raw);
  });
});

describe('replayDeleteMatch', () => {
  const { replayDeleteMatch } = jest.requireActual('./replayDeleteMatch');
  beforeEach(() => jest.clearAllMocks());

  it('sends Command_ReplayDeleteMatch', () => {
    replayDeleteMatch(7);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith('Command_ReplayDeleteMatch', { gameId: 7 }, expect.any(Object));
  });

  it('calls replayDeleteMatch on success', () => {
    replayDeleteMatch(7);
    invokeOnSuccess();
    expect(SessionPersistence.replayDeleteMatch).toHaveBeenCalledWith(7);
  });
});

describe('replayList', () => {
  const { replayList } = jest.requireActual('./replayList');
  beforeEach(() => jest.clearAllMocks());

  it('sends Command_ReplayList', () => {
    replayList();
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith('Command_ReplayList', {}, expect.any(Object));
  });

  it('calls replayList on success', () => {
    replayList();
    const resp = { matchList: [] };
    invokeOnSuccess(resp, { responseCode: 0, '.Response_ReplayList.ext': resp });
    expect(SessionPersistence.replayList).toHaveBeenCalledWith([]);
  });
});

describe('replayModifyMatch', () => {
  const { replayModifyMatch } = jest.requireActual('./replayModifyMatch');
  beforeEach(() => jest.clearAllMocks());

  it('sends Command_ReplayModifyMatch', () => {
    replayModifyMatch(7, true);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      'Command_ReplayModifyMatch', { gameId: 7, doNotHide: true }, expect.any(Object)
    );
  });

  it('calls replayModifyMatch on success', () => {
    replayModifyMatch(7, true);
    invokeOnSuccess();
    expect(SessionPersistence.replayModifyMatch).toHaveBeenCalledWith(7, true);
  });
});

describe('addToList / addToBuddyList / addToIgnoreList', () => {
  const { addToList, addToBuddyList, addToIgnoreList } = jest.requireActual('./addToList');
  beforeEach(() => jest.clearAllMocks());

  it('addToBuddyList sends Command_AddToList with list=buddy', () => {
    addToBuddyList('alice');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      'Command_AddToList', { list: 'buddy', userName: 'alice' }, expect.any(Object)
    );
  });

  it('addToIgnoreList sends Command_AddToList with list=ignore', () => {
    addToIgnoreList('bob');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      'Command_AddToList', { list: 'ignore', userName: 'bob' }, expect.any(Object)
    );
  });

  it('onSuccess calls SessionPersistence.addToList', () => {
    addToList('buddy', 'alice');
    invokeOnSuccess();
    expect(SessionPersistence.addToList).toHaveBeenCalledWith('buddy', 'alice');
  });
});

describe('removeFromList / removeFromBuddyList / removeFromIgnoreList', () => {
  const { removeFromList, removeFromBuddyList, removeFromIgnoreList } = jest.requireActual('./removeFromList');
  beforeEach(() => jest.clearAllMocks());

  it('removeFromBuddyList sends Command_RemoveFromList with list=buddy', () => {
    removeFromBuddyList('alice');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      'Command_RemoveFromList', { list: 'buddy', userName: 'alice' }, expect.any(Object)
    );
  });

  it('removeFromIgnoreList sends Command_RemoveFromList with list=ignore', () => {
    removeFromIgnoreList('bob');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      'Command_RemoveFromList', { list: 'ignore', userName: 'bob' }, expect.any(Object)
    );
  });

  it('onSuccess calls SessionPersistence.removeFromList', () => {
    removeFromList('buddy', 'alice');
    invokeOnSuccess();
    expect(SessionPersistence.removeFromList).toHaveBeenCalledWith('buddy', 'alice');
  });
});
