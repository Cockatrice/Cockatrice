// Shared mock setup for session command tests

vi.mock('../../persistence', async () => {
  const { makeSessionPersistenceMock } = await import('../../__mocks__/sessionCommandMocks');
  return {
    SessionPersistence: makeSessionPersistenceMock(),
    RoomPersistence: { joinRoom: vi.fn() },
  };
});

vi.mock('../../WebClient', async () => {
  const { makeWebClientMock } = await import('../../__mocks__/sessionCommandMocks');
  return { __esModule: true, default: makeWebClientMock() };
});

vi.mock('../../utils', async () => {
  const { makeUtilsMock } = await import('../../__mocks__/sessionCommandMocks');
  return makeUtilsMock();
});

// Mock session commands barrel to allow cross-command calls while keeping real implementations
vi.mock('./', async () => {
  const actual = await vi.importActual('./');
  const { makeSessionBarrelMock } = await import('../../__mocks__/sessionCommandMocks');
  return { ...(actual as Record<string, unknown>), ...makeSessionBarrelMock() };
});

import { Mock } from 'vitest';
import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import { SessionPersistence } from '../../persistence';
import { RoomPersistence } from '../../persistence';
import webClient from '../../WebClient';
import { hashPassword, generateSalt, passwordSaltSupported } from '../../utils';

import { accountEdit } from './accountEdit';
import { accountImage } from './accountImage';
import { accountPassword } from './accountPassword';
import { deckDel } from './deckDel';
import { deckDelDir } from './deckDelDir';
import { deckList } from './deckList';
import { deckNewDir } from './deckNewDir';
import { deckUpload } from './deckUpload';
import { disconnect } from './disconnect';
import { getGamesOfUser } from './getGamesOfUser';
import { getUserInfo } from './getUserInfo';
import { joinRoom } from './joinRoom';
import { listRooms } from './listRooms';
import { listUsers } from './listUsers';
import { message } from './message';
import { ping } from './ping';
import { replayDeleteMatch } from './replayDeleteMatch';
import { replayList } from './replayList';
import { replayModifyMatch } from './replayModifyMatch';
import { addToList, addToBuddyList, addToIgnoreList } from './addToList';
import { removeFromList, removeFromBuddyList, removeFromIgnoreList } from './removeFromList';
import { replayGetCode } from './replayGetCode';
import { replaySubmitCode } from './replaySubmitCode';
import { Data } from '@app/types';

const { invokeOnSuccess, invokeCallback } = makeCallbackHelpers(
  webClient.protobuf.sendSessionCommand as Mock,
  2
);

beforeEach(() => {
  (hashPassword as Mock).mockReturnValue('hashed_pw');
  (generateSalt as Mock).mockReturnValue('randSalt');
  (passwordSaltSupported as Mock).mockReturnValue(0);
});

// ----------------------------------------------------------------

describe('accountEdit', () => {
  it('sends Command_AccountEdit with correct params', () => {
    accountEdit('pw', 'Alice', 'a@b.com', 'US');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_AccountEdit_ext,
      expect.objectContaining({ passwordCheck: 'pw', realName: 'Alice', email: 'a@b.com', country: 'US' }),
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
  it('sends Command_AccountImage', () => {
    const img = new Uint8Array([1, 2]);
    accountImage(img);
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_AccountImage_ext, expect.objectContaining({ image: img }), expect.any(Object)
    );
  });

  it('calls SessionPersistence.accountImageChanged on success', () => {
    const img = new Uint8Array([1, 2]);
    accountImage(img);
    invokeOnSuccess();
    expect(SessionPersistence.accountImageChanged).toHaveBeenCalledWith(img);
  });
});

describe('accountPassword', () => {
  it('sends Command_AccountPassword', () => {
    accountPassword('old', 'new', 'hashed');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_AccountPassword_ext,
      expect.objectContaining({ oldPassword: 'old', newPassword: 'new', hashedNewPassword: 'hashed' }),
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
  it('sends Command_DeckDel', () => {
    deckDel(42);
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_DeckDel_ext,
      expect.objectContaining({ deckId: 42 }),
      expect.any(Object)
    );
  });

  it('calls deleteServerDeck on success', () => {
    deckDel(42);
    invokeOnSuccess();
    expect(SessionPersistence.deleteServerDeck).toHaveBeenCalledWith(42);
  });
});

describe('deckDelDir', () => {
  it('sends Command_DeckDelDir', () => {
    deckDelDir('/path');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_DeckDelDir_ext, expect.objectContaining({ path: '/path' }), expect.any(Object)
    );
  });

  it('calls deleteServerDeckDir on success', () => {
    deckDelDir('/path');
    invokeOnSuccess();
    expect(SessionPersistence.deleteServerDeckDir).toHaveBeenCalledWith('/path');
  });
});

describe('deckList', () => {
  it('sends Command_DeckList', () => {
    deckList();
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_DeckList_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Data.Response_DeckList_ext })
    );
  });

  it('calls updateServerDecks on success', () => {
    deckList();
    const root = { items: [] };
    invokeOnSuccess({ root }, { responseCode: 0 });
    expect(SessionPersistence.updateServerDecks).toHaveBeenCalledWith({ root });
  });
});

describe('deckNewDir', () => {
  it('sends Command_DeckNewDir', () => {
    deckNewDir('/path', 'dir');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_DeckNewDir_ext, expect.objectContaining({ path: '/path', dirName: 'dir' }), expect.any(Object)
    );
  });

  it('calls createServerDeckDir on success', () => {
    deckNewDir('/path', 'dir');
    invokeOnSuccess();
    expect(SessionPersistence.createServerDeckDir).toHaveBeenCalledWith('/path', 'dir');
  });
});

describe('deckUpload', () => {
  it('sends Command_DeckUpload', () => {
    deckUpload('/path', 1, 'content');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_DeckUpload_ext,
      expect.objectContaining({ path: '/path', deckId: 1, deckList: 'content' }),
      expect.objectContaining({ responseExt: Data.Response_DeckUpload_ext })
    );
  });

  it('calls uploadServerDeck on success', () => {
    deckUpload('/path', 1, 'content');
    const resp = { newFile: { id: 1 } };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(SessionPersistence.uploadServerDeck).toHaveBeenCalledWith('/path', resp.newFile);
  });
});

describe('disconnect', () => {
  it('calls webClient.disconnect', () => {
    disconnect();
    expect(webClient.disconnect).toHaveBeenCalled();
  });
});

describe('getGamesOfUser', () => {
  it('sends Command_GetGamesOfUser', () => {
    getGamesOfUser('alice');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_GetGamesOfUser_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Data.Response_GetGamesOfUser_ext })
    );
  });

  it('calls getGamesOfUser on success', () => {
    getGamesOfUser('alice');
    const resp = { gameList: [] };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(SessionPersistence.getGamesOfUser).toHaveBeenCalledWith('alice', resp);
  });
});

describe('getUserInfo', () => {
  it('sends Command_GetUserInfo', () => {
    getUserInfo('alice');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_GetUserInfo_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Data.Response_GetUserInfo_ext })
    );
  });

  it('calls getUserInfo on success', () => {
    getUserInfo('alice');
    const resp = { userInfo: { name: 'alice' } };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(SessionPersistence.getUserInfo).toHaveBeenCalledWith(resp.userInfo);
  });
});

describe('joinRoom', () => {
  it('sends Command_JoinRoom', () => {
    joinRoom(5);
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_JoinRoom_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Data.Response_JoinRoom_ext })
    );
  });

  it('calls RoomPersistence.joinRoom on success', () => {
    joinRoom(5);
    const resp = { roomInfo: { roomId: 5 } };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(RoomPersistence.joinRoom).toHaveBeenCalledWith(resp.roomInfo);
  });
});

describe('listRooms (command)', () => {
  it('sends Command_ListRooms', () => {
    listRooms();
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(Data.Command_ListRooms_ext, expect.any(Object));
  });
});

describe('listUsers', () => {
  it('sends Command_ListUsers', () => {
    listUsers();
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_ListUsers_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Data.Response_ListUsers_ext })
    );
  });

  it('calls SessionPersistence.updateUsers with the user list on success', () => {
    listUsers();
    const resp = { userList: [{ name: 'Alice' }] };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(SessionPersistence.updateUsers).toHaveBeenCalledWith([{ name: 'Alice' }]);
  });
});

describe('message', () => {
  it('sends Command_Message', () => {
    message('bob', 'hi');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_Message_ext, expect.objectContaining({ userName: 'bob', message: 'hi' })
    );
  });

});

describe('ping', () => {
  it('sends Command_Ping', () => {
    const pingReceived = vi.fn();
    ping(pingReceived);
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(Data.Command_Ping_ext, expect.any(Object), expect.any(Object));
  });

  it('calls pingReceived via onResponse', () => {
    const pingReceived = vi.fn();
    ping(pingReceived);
    invokeCallback('onResponse', {});
    expect(pingReceived).toHaveBeenCalled();
  });
});

describe('replayDeleteMatch', () => {
  it('sends Command_ReplayDeleteMatch', () => {
    replayDeleteMatch(7);
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_ReplayDeleteMatch_ext,
      expect.objectContaining({ gameId: 7 }),
      expect.any(Object)
    );
  });

  it('calls replayDeleteMatch on success', () => {
    replayDeleteMatch(7);
    invokeOnSuccess();
    expect(SessionPersistence.replayDeleteMatch).toHaveBeenCalledWith(7);
  });
});

describe('replayList', () => {
  it('sends Command_ReplayList', () => {
    replayList();
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_ReplayList_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Data.Response_ReplayList_ext })
    );
  });

  it('calls replayList on success', () => {
    replayList();
    const resp = { matchList: [] };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(SessionPersistence.replayList).toHaveBeenCalledWith([]);
  });
});

describe('replayModifyMatch', () => {
  it('sends Command_ReplayModifyMatch', () => {
    replayModifyMatch(7, true);
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_ReplayModifyMatch_ext, expect.objectContaining({ gameId: 7, doNotHide: true }), expect.any(Object)
    );
  });

  it('calls replayModifyMatch on success', () => {
    replayModifyMatch(7, true);
    invokeOnSuccess();
    expect(SessionPersistence.replayModifyMatch).toHaveBeenCalledWith(7, true);
  });
});

describe('addToList / addToBuddyList / addToIgnoreList', () => {
  it('addToBuddyList sends Command_AddToList with list=buddy', () => {
    addToBuddyList('alice');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_AddToList_ext,
      expect.objectContaining({ list: 'buddy' }),
      expect.any(Object)
    );
  });

  it('addToIgnoreList sends Command_AddToList with list=ignore', () => {
    addToIgnoreList('bob');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_AddToList_ext,
      expect.objectContaining({ list: 'ignore' }),
      expect.any(Object)
    );
  });

  it('onSuccess calls SessionPersistence.addToList', () => {
    addToList('buddy', 'alice');
    invokeOnSuccess();
    expect(SessionPersistence.addToList).toHaveBeenCalledWith('buddy', 'alice');
  });
});

describe('removeFromList / removeFromBuddyList / removeFromIgnoreList', () => {
  it('removeFromBuddyList sends Command_RemoveFromList with list=buddy', () => {
    removeFromBuddyList('alice');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_RemoveFromList_ext,
      expect.objectContaining({ list: 'buddy' }),
      expect.any(Object)
    );
  });

  it('removeFromIgnoreList sends Command_RemoveFromList with list=ignore', () => {
    removeFromIgnoreList('bob');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_RemoveFromList_ext,
      expect.objectContaining({ list: 'ignore' }),
      expect.any(Object)
    );
  });

  it('onSuccess calls SessionPersistence.removeFromList', () => {
    removeFromList('buddy', 'alice');
    invokeOnSuccess();
    expect(SessionPersistence.removeFromList).toHaveBeenCalledWith('buddy', 'alice');
  });
});

describe('replayGetCode', () => {
  it('sends Command_ReplayGetCode with gameId and responseExt', () => {
    replayGetCode(42, vi.fn());
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_ReplayGetCode_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Data.Response_ReplayGetCode_ext })
    );
  });

  it('calls onCodeReceived with replayCode on success', () => {
    const onCodeReceived = vi.fn();
    replayGetCode(42, onCodeReceived);
    invokeOnSuccess({ replayCode: 'abc123-xyz' });
    expect(onCodeReceived).toHaveBeenCalledWith('abc123-xyz');
  });
});

describe('replaySubmitCode', () => {
  it('sends Command_ReplaySubmitCode with replayCode', () => {
    replaySubmitCode('42-abc123');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_ReplaySubmitCode_ext, expect.objectContaining({ replayCode: '42-abc123' }), expect.any(Object)
    );
  });

  it('forwards onSuccess callback', () => {
    const onSuccess = vi.fn();
    replaySubmitCode('42-abc123', onSuccess);
    invokeOnSuccess();
    expect(onSuccess).toHaveBeenCalled();
  });

  it('forwards onError callback', () => {
    const onError = vi.fn();
    replaySubmitCode('42-abc123', undefined, onError);
    invokeCallback('onError', 404);
    expect(onError).toHaveBeenCalledWith(404);
  });
});
