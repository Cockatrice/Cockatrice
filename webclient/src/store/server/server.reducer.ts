import { SortDirection, StatusEnum, UserSortField } from 'types';

import { SortUtil } from '../common';

import { ServerState } from './server.interfaces'
import { Types } from './server.types';

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
            ...state.messages[userName],
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
    default:
      return state;
  }
}
