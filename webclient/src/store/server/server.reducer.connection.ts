import { CaseReducer, PayloadAction } from '@reduxjs/toolkit';
import { App, Data } from '@app/types';
import { WebsocketTypes } from '@app/websocket/types';
import { ServerState, ServerStateStatus } from './server.interfaces';

export const initialState: ServerState = {
  initialized: false,
  testConnectionStatus: null,
  buddyList: {},
  ignoreList: {},

  status: {
    connectionAttemptMade: false,
    state: WebsocketTypes.StatusEnum.DISCONNECTED,
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
  users: {},
  sortUsersBy: {
    field: App.UserSortField.NAME,
    order: App.SortDirection.ASC
  },
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
  replays: {},
  backendDecks: null,
  downloadedDeck: null,
  downloadedReplay: null,
  gamesOfUser: {},
  registrationError: null,
};

export const connectionReducers = {
  initialized: (() => ({
    ...initialState,
    initialized: true,
  })) as CaseReducer<ServerState>,

  connectionAttempted: ((state) => {
    state.status.connectionAttemptMade = true;
  }) as CaseReducer<ServerState>,

  testConnectionStarted: ((state) => {
    state.testConnectionStatus = 'testing';
  }) as CaseReducer<ServerState>,

  // `supportsHashedPassword` is typed on the action so `useReduxEffect`
  // subscribers (see useKnownHostsComponent) can persist it to the host
  // record in Dexie. It's deliberately not stored in redux state since
  // only the lifecycle matters here; per-host capability lives in Dexie.
  testConnectionSuccessful: ((state, _action) => {
    state.testConnectionStatus = 'success';
  }) as CaseReducer<ServerState, PayloadAction<{ supportsHashedPassword: boolean }>>,

  testConnectionFailed: ((state) => {
    state.testConnectionStatus = 'failed';
  }) as CaseReducer<ServerState>,

  clearStore: ((state) => ({
    ...initialState,
    status: { ...state.status },
  })) as CaseReducer<ServerState>,

  serverMessage: ((state, action) => {
    state.info.message = action.payload.message;
  }) as CaseReducer<ServerState, PayloadAction<{ message: string }>>,

  updateInfo: ((state, action) => {
    const { name, version } = action.payload.info;
    state.info.name = name;
    state.info.version = version;
  }) as CaseReducer<ServerState, PayloadAction<{ info: { name: string; version: string } }>>,

  updateStatus: ((state, action) => {
    const { status } = action.payload;
    state.status.state = status.state;
    state.status.description = status.description;

    if (status.state === WebsocketTypes.StatusEnum.DISCONNECTED) {
      state.status.connectionAttemptMade = false;
    }
  }) as CaseReducer<ServerState, PayloadAction<{ status: Pick<ServerStateStatus, 'state' | 'description'> }>>,

  serverShutdown: ((state, action) => {
    state.serverShutdown = action.payload.data;
  }) as CaseReducer<ServerState, PayloadAction<{ data: Data.Event_ServerShutdown }>>,
};
