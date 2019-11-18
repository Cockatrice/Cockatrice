import { StatusEnum, getStatusEnumLabel } from 'types';

import { ServerState } from './server.interfaces'
import { Types } from './server.types';

const initialState: ServerState = {
  status: {
    state: StatusEnum.DISCONNECTED,
    description: null
  },
  info: {
    message: null,
    name: null,
    version: null
  },
  user: null,
  users: []
};

export const serverReducer = (state = initialState, action: any) => {
  switch(action.type) {
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
    case Types.UPDATE_USER: {
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
      const users = [ ...action.users ];
      users.sort((a, b) => a.name.localeCompare(b.name));

      return {
        ...state,
        users
      };
    }
    case Types.USER_JOINED: {
      const users = [
        ...state.users,
        { ...action.user }
      ];

      users.sort((a, b) => a.name.localeCompare(b.name));

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
    default:
      return state;
  }
}
