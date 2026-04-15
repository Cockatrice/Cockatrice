import { DeckStorageFolder, DeckStorageTreeItem, SortDirection, StatusEnum, UserLevelFlag, UserSortField } from 'types';

import { SortUtil } from '../common';

import { ServerState } from './server.interfaces'
import { Types } from './server.types';

function splitPath(path: string): string[] {
  return path ? path.split('/') : [];
}

function insertAtPath(folder: DeckStorageFolder, pathSegments: string[], item: DeckStorageTreeItem): DeckStorageFolder {
  if (pathSegments.length === 0 || (pathSegments.length === 1 && pathSegments[0] === '')) {
    return { items: [...folder.items, item] };
  }
  const [head, ...tail] = pathSegments;
  const match = folder.items.find(child => child.name === head && child.folder);
  if (match) {
    return {
      items: folder.items.map(child =>
        child === match
          ? { ...child, folder: insertAtPath(child.folder!, tail, item) }
          : child
      ),
    };
  }
  const created: DeckStorageTreeItem = { id: 0, name: head, file: null, folder: insertAtPath({ items: [] }, tail, item) };
  return { items: [...folder.items, created] };
}

function removeById(folder: DeckStorageFolder, id: number): DeckStorageFolder {
  return {
    items: folder.items
      .filter(item => item.id !== id)
      .map(item =>
        item.folder ? { ...item, folder: removeById(item.folder, id) } : item
      ),
  };
}

function removeByPath(folder: DeckStorageFolder, pathSegments: string[]): DeckStorageFolder {
  if (pathSegments.length === 0 || (pathSegments.length === 1 && pathSegments[0] === '')) {
    return folder;
  }
  const [head, ...tail] = pathSegments;
  if (tail.length === 0) {
    return { items: folder.items.filter(item => !(item.name === head && item.folder !== null)) };
  }
  return {
    items: folder.items.map(item =>
      item.name === head && item.folder
        ? { ...item, folder: removeByPath(item.folder, tail) }
        : item
    ),
  };
}

const initialState: ServerState = {
  initialized: false,
  buddyList: [],
  ignoreList: [],

  status: {
    state: StatusEnum.DISCONNECTED,
    description: null
  },
  info: {
    message: null,
    name: null,
    version: null
  },
  logs: {
    room: [],
    game: [],
    chat: []
  },
  user: null,
  users: [],
  sortUsersBy: {
    field: UserSortField.NAME,
    order: SortDirection.ASC
  },
  connectOptions: {},
  messages: {},
  userInfo: {},
  notifications: [],
  serverShutdown: null,
  banUser: '',
  banHistory: {},
  warnHistory: {},
  warnListOptions: [],
  warnUser: '',
  adminNotes: {},
  replays: [],
  backendDecks: null,
};

export const serverReducer = (state = initialState, action: any) => {
  switch (action.type) {
    case Types.INITIALIZED: {
      return {
        ...initialState,
        initialized: true
      }
    }
    case Types.ACCOUNT_AWAITING_ACTIVATION: {
      return {
        ...state,
        connectOptions: {
          ...action.options
        }
      }
    }
    case Types.ACCOUNT_ACTIVATION_FAILED:
    case Types.ACCOUNT_ACTIVATION_SUCCESS: {
      return {
        ...state,
        connectOptions: {}
      }
    }
    case Types.CLEAR_STORE: {
      return {
        ...initialState,
        status: {
          ...state.status
        }
      }
    }
    case Types.SERVER_MESSAGE: {
      const { message } = action;
      const { info } = state;

      return {
        ...state,
        info: { ...info, message }
      }
    }
    case Types.UPDATE_BUDDY_LIST: {
      const { buddyList } = action;
      const { sortUsersBy } = state;

      SortUtil.sortUsersByField(buddyList, sortUsersBy);

      return {
        ...state,
        buddyList: [
          ...buddyList
        ]
      };
    }
    case Types.ADD_TO_BUDDY_LIST: {
      const { user } = action;
      const { sortUsersBy } = state;

      const buddyList = [...state.buddyList];

      buddyList.push(user);
      SortUtil.sortUsersByField(buddyList, sortUsersBy);

      return {
        ...state,
        buddyList
      };
    }
    case Types.REMOVE_FROM_BUDDY_LIST: {
      const { userName } = action;
      const buddyList = state.buddyList.filter(user => user.name !== userName);

      return {
        ...state,
        buddyList
      };
    }
    case Types.UPDATE_IGNORE_LIST: {
      const { ignoreList } = action;
      const { sortUsersBy } = state;

      SortUtil.sortUsersByField(ignoreList, sortUsersBy);

      return {
        ...state,
        ignoreList: [
          ...ignoreList
        ]
      };
    }
    case Types.ADD_TO_IGNORE_LIST: {
      const { user } = action;
      const { sortUsersBy } = state;

      const ignoreList = [...state.ignoreList];

      ignoreList.push(user);
      SortUtil.sortUsersByField(ignoreList, sortUsersBy);

      return {
        ...state,
        ignoreList
      };
    }
    case Types.REMOVE_FROM_IGNORE_LIST: {
      const { userName } = action;
      const ignoreList = state.ignoreList.filter(user => user.name !== userName);

      return {
        ...state,
        ignoreList
      };
    }
    case Types.UPDATE_INFO: {
      const { name, version } = action.info;
      const { info } = state;

      return {
        ...state,
        info: { ...info, name, version }
      }
    }
    case Types.UPDATE_STATUS: {
      const { status } = action;

      return {
        ...state,
        status: { ...status }
      }
    }
    case Types.UPDATE_USER:
    case Types.ACCOUNT_EDIT_CHANGED:
    case Types.ACCOUNT_IMAGE_CHANGED: {
      const { user } = action;

      return {
        ...state,
        user: {
          ...state.user,
          ...user
        }
      }
    }
    case Types.UPDATE_USERS: {
      const users = [...action.users];
      const { sortUsersBy } = state;


      SortUtil.sortUsersByField(users, sortUsersBy);

      return {
        ...state,
        users
      };
    }
    case Types.USER_JOINED: {
      const { sortUsersBy } = state;

      const users = [
        ...state.users,
        { ...action.user }
      ];

      SortUtil.sortUsersByField(users, sortUsersBy);

      return {
        ...state,
        users
      };
    }
    case Types.USER_LEFT: {
      const { name } = action;
      const users = state.users.filter(user => user.name !== name);

      return {
        ...state,
        users
      };
    }
    case Types.VIEW_LOGS: {
      const { logs } = action;

      return {
        ...state,
        logs: {
          ...logs
        }
      };
    }
    case Types.CLEAR_LOGS: {
      return {
        ...state,
        logs: {
          ...initialState.logs
        }
      }
    }
    case Types.USER_MESSAGE: {
      const { senderName, receiverName } = action.messageData;
      const userName = state.user.name === senderName ? receiverName : senderName;

      return {
        ...state,
        messages: {
          ...state.messages,
          [userName]: [
            ...(state.messages[userName] ?? []),
            action.messageData,
          ],
        }
      };
    }
    case Types.GET_USER_INFO: {
      const { userInfo } = action;

      return {
        ...state,
        userInfo: {
          ...state.userInfo,
          [userInfo.name]: userInfo,
        }
      };
    }
    case Types.NOTIFY_USER: {
      const { notification } = action;

      return {
        ...state,
        notifications: [
          ...state.notifications,
          notification
        ]
      };
    }
    case Types.SERVER_SHUTDOWN: {
      const { data } = action;

      return {
        ...state,
        serverShutdown: data,
      };
    }
    case Types.BAN_FROM_SERVER: {
      const { userName } = action;

      return {
        ...state,
        banUser: userName,
      };
    }
    case Types.BAN_HISTORY: {
      const { userName, banHistory } = action;

      return {
        ...state,
        banHistory: {
          ...state.banHistory,
          [userName]: banHistory,
        }
      };
    }
    case Types.WARN_HISTORY: {
      const { userName, warnHistory } = action;

      return {
        ...state,
        warnHistory: {
          ...state.warnHistory,
          [userName]: warnHistory,
        }
      };
    }
    case Types.WARN_LIST_OPTIONS: {
      const { warnList } = action;

      return {
        ...state,
        warnListOptions: warnList,
      };
    }
    case Types.WARN_USER: {
      const { userName } = action;
      return {
        ...state,
        warnUser: userName,
      };
    }
    case Types.GET_ADMIN_NOTES:
    case Types.UPDATE_ADMIN_NOTES: {
      const { userName, notes } = action;
      return {
        ...state,
        adminNotes: {
          ...state.adminNotes,
          [userName]: notes,
        }
      };
    }
    case Types.ADJUST_MOD: {
      const { userName, shouldBeMod, shouldBeJudge } = action;

      return {
        ...state,
        users: state.users.map((user) => {
          if (user.name !== userName) {
            return user;
          }
          const judgeFlag = shouldBeJudge ? UserLevelFlag.IsJudge : UserLevelFlag.IsNothing;
          const modFlag = shouldBeMod ? UserLevelFlag.IsModerator : UserLevelFlag.IsNothing;
          return {
            ...user,
            userLevel: user.userLevel & (judgeFlag | modFlag)
          }
        })
      };
    }
    case Types.REPLAY_LIST: {
      return { ...state, replays: [...action.matchList] };
    }
    case Types.REPLAY_ADDED: {
      return { ...state, replays: [...state.replays, action.matchInfo] };
    }
    case Types.REPLAY_MODIFY_MATCH: {
      return {
        ...state,
        replays: state.replays.map(r =>
          r.gameId === action.gameId ? { ...r, doNotHide: action.doNotHide } : r
        ),
      };
    }
    case Types.REPLAY_DELETE_MATCH: {
      return { ...state, replays: state.replays.filter(r => r.gameId !== action.gameId) };
    }
    case Types.BACKEND_DECKS: {
      return { ...state, backendDecks: action.deckList };
    }
    case Types.DECK_UPLOAD: {
      if (!state.backendDecks) {
        return state;
      }
      return {
        ...state,
        backendDecks: {
          root: insertAtPath(state.backendDecks.root, splitPath(action.path), action.treeItem),
        },
      };
    }
    case Types.DECK_DELETE: {
      if (!state.backendDecks) {
        return state;
      }
      return {
        ...state,
        backendDecks: {
          root: removeById(state.backendDecks.root, action.deckId),
        },
      };
    }
    case Types.DECK_NEW_DIR: {
      if (!state.backendDecks) {
        return state;
      }
      const newFolder: DeckStorageTreeItem = { id: 0, name: action.dirName, file: null, folder: { items: [] } };
      return {
        ...state,
        backendDecks: {
          root: insertAtPath(state.backendDecks.root, splitPath(action.path), newFolder),
        },
      };
    }
    case Types.DECK_DEL_DIR: {
      if (!state.backendDecks) {
        return state;
      }
      return {
        ...state,
        backendDecks: {
          root: removeByPath(state.backendDecks.root, splitPath(action.path)),
        },
      };
    }
    default:
      return state;
  }
}
