// Shared mock setup for session command tests

vi.mock('../../services/BackendService', () => ({
  BackendService: {
    sendSessionCommand: vi.fn(),
  },
}));

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
  return { ...(actual as any), ...makeSessionBarrelMock() };
});

import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';
import { RoomPersistence } from '../../persistence';
import webClient from '../../WebClient';
import * as SessionCommands from './';
import { hashPassword, generateSalt, passwordSaltSupported } from '../../utils';
import {
  Command_AccountEdit_ext,
  Command_AccountImage_ext,
  Command_AccountPassword_ext,
  Command_AddToList_ext,
  Command_GetGamesOfUser_ext,
  Command_GetUserInfo_ext,
  Command_JoinRoom_ext,
  Command_ListRooms_ext,
  Command_ListUsers_ext,
  Command_Message_ext,
  Command_Ping_ext,
  Command_RemoveFromList_ext,
} from 'generated/proto/session_commands_pb';
import { Command_DeckDel_ext } from 'generated/proto/command_deck_del_pb';
import { Command_DeckDelDir_ext } from 'generated/proto/command_deck_del_dir_pb';
import { Command_DeckList_ext } from 'generated/proto/command_deck_list_pb';
import { Command_DeckNewDir_ext } from 'generated/proto/command_deck_new_dir_pb';
import { Command_DeckUpload_ext } from 'generated/proto/command_deck_upload_pb';
import { Command_ReplayDeleteMatch_ext } from 'generated/proto/command_replay_delete_match_pb';
import { Command_ReplayGetCode_ext } from 'generated/proto/command_replay_get_code_pb';
import { Command_ReplayList_ext } from 'generated/proto/command_replay_list_pb';
import { Command_ReplayModifyMatch_ext } from 'generated/proto/command_replay_modify_match_pb';
import { Command_ReplaySubmitCode_ext } from 'generated/proto/command_replay_submit_code_pb';
import { Response_DeckList_ext } from 'generated/proto/response_deck_list_pb';
import { Response_DeckUpload_ext } from 'generated/proto/response_deck_upload_pb';
import { Response_GetGamesOfUser_ext } from 'generated/proto/response_get_games_of_user_pb';
import { Response_GetUserInfo_ext } from 'generated/proto/response_get_user_info_pb';
import { Response_JoinRoom_ext } from 'generated/proto/response_join_room_pb';
import { Response_ListUsers_ext } from 'generated/proto/response_list_users_pb';
import { Response_ReplayGetCode_ext } from 'generated/proto/response_replay_get_code_pb';
import { Response_ReplayList_ext } from 'generated/proto/response_replay_list_pb';
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

const { invokeOnSuccess, invokeCallback } = makeCallbackHelpers(
  BackendService.sendSessionCommand as vi.Mock,
  2
);

beforeEach(() => {
  vi.clearAllMocks();
  (hashPassword as vi.Mock).mockReturnValue('hashed_pw');
  (generateSalt as vi.Mock).mockReturnValue('randSalt');
  (passwordSaltSupported as vi.Mock).mockReturnValue(0);
});

// ----------------------------------------------------------------

describe('accountEdit', () => {
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_AccountEdit with correct params', () => {
    accountEdit('pw', 'Alice', 'a@b.com', 'US');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_AccountEdit_ext,
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
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_AccountImage', () => {
    const img = new Uint8Array([1, 2]);
    accountImage(img);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_AccountImage_ext, expect.objectContaining({ image: img }), expect.any(Object)
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
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_AccountPassword', () => {
    accountPassword('old', 'new', 'hashed');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_AccountPassword_ext,
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
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_DeckDel', () => {
    deckDel(42);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_DeckDel_ext,
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
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_DeckDelDir', () => {
    deckDelDir('/path');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_DeckDelDir_ext, expect.objectContaining({ path: '/path' }), expect.any(Object)
    );
  });

  it('calls deleteServerDeckDir on success', () => {
    deckDelDir('/path');
    invokeOnSuccess();
    expect(SessionPersistence.deleteServerDeckDir).toHaveBeenCalledWith('/path');
  });
});

describe('deckList', () => {
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_DeckList', () => {
    deckList();
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_DeckList_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_DeckList_ext })
    );
  });

  it('calls updateServerDecks on success', () => {
    deckList();
    const resp = { folders: [] };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(SessionPersistence.updateServerDecks).toHaveBeenCalledWith(resp);
  });
});

describe('deckNewDir', () => {
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_DeckNewDir', () => {
    deckNewDir('/path', 'dir');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_DeckNewDir_ext, expect.objectContaining({ path: '/path', dirName: 'dir' }), expect.any(Object)
    );
  });

  it('calls createServerDeckDir on success', () => {
    deckNewDir('/path', 'dir');
    invokeOnSuccess();
    expect(SessionPersistence.createServerDeckDir).toHaveBeenCalledWith('/path', 'dir');
  });
});

describe('deckUpload', () => {
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_DeckUpload', () => {
    deckUpload('/path', 1, 'content');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_DeckUpload_ext,
      expect.objectContaining({ path: '/path', deckId: 1, deckList: 'content' }),
      expect.objectContaining({ responseExt: Response_DeckUpload_ext })
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
  beforeEach(() => vi.clearAllMocks());

  it('calls webClient.disconnect', () => {
    disconnect();
    expect(webClient.disconnect).toHaveBeenCalled();
  });
});

describe('getGamesOfUser', () => {
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_GetGamesOfUser', () => {
    getGamesOfUser('alice');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_GetGamesOfUser_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_GetGamesOfUser_ext })
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
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_GetUserInfo', () => {
    getUserInfo('alice');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_GetUserInfo_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_GetUserInfo_ext })
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
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_JoinRoom', () => {
    joinRoom(5);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_JoinRoom_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_JoinRoom_ext })
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
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_ListRooms', () => {
    listRooms();
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(Command_ListRooms_ext, expect.any(Object));
  });
});

describe('listUsers', () => {
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_ListUsers', () => {
    listUsers();
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_ListUsers_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_ListUsers_ext })
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
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_Message', () => {
    message('bob', 'hi');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_Message_ext, expect.objectContaining({ userName: 'bob', message: 'hi' })
    );
  });

});

describe('ping', () => {
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_Ping', () => {
    const pingReceived = vi.fn();
    ping(pingReceived);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(Command_Ping_ext, expect.any(Object), expect.any(Object));
  });

  it('calls pingReceived via onResponse', () => {
    const pingReceived = vi.fn();
    ping(pingReceived);
    const raw = {};
    invokeCallback('onResponse', raw);
    expect(pingReceived).toHaveBeenCalledWith(raw);
  });
});

describe('replayDeleteMatch', () => {
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_ReplayDeleteMatch', () => {
    replayDeleteMatch(7);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_ReplayDeleteMatch_ext,
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
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_ReplayList', () => {
    replayList();
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_ReplayList_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_ReplayList_ext })
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
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_ReplayModifyMatch', () => {
    replayModifyMatch(7, true);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_ReplayModifyMatch_ext, expect.objectContaining({ gameId: 7, doNotHide: true }), expect.any(Object)
    );
  });

  it('calls replayModifyMatch on success', () => {
    replayModifyMatch(7, true);
    invokeOnSuccess();
    expect(SessionPersistence.replayModifyMatch).toHaveBeenCalledWith(7, true);
  });
});

describe('addToList / addToBuddyList / addToIgnoreList', () => {
  beforeEach(() => vi.clearAllMocks());

  it('addToBuddyList sends Command_AddToList with list=buddy', () => {
    addToBuddyList('alice');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_AddToList_ext,
      expect.objectContaining({ list: 'buddy' }),
      expect.any(Object)
    );
  });

  it('addToIgnoreList sends Command_AddToList with list=ignore', () => {
    addToIgnoreList('bob');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_AddToList_ext,
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
  beforeEach(() => vi.clearAllMocks());

  it('removeFromBuddyList sends Command_RemoveFromList with list=buddy', () => {
    removeFromBuddyList('alice');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_RemoveFromList_ext,
      expect.objectContaining({ list: 'buddy' }),
      expect.any(Object)
    );
  });

  it('removeFromIgnoreList sends Command_RemoveFromList with list=ignore', () => {
    removeFromIgnoreList('bob');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_RemoveFromList_ext,
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
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_ReplayGetCode with gameId and responseExt', () => {
    replayGetCode(42, vi.fn());
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_ReplayGetCode_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_ReplayGetCode_ext })
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
  beforeEach(() => vi.clearAllMocks());

  it('sends Command_ReplaySubmitCode with replayCode', () => {
    replaySubmitCode('42-abc123');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_ReplaySubmitCode_ext, expect.objectContaining({ replayCode: '42-abc123' }), expect.any(Object)
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
