import { combineReducers } from '@reduxjs/toolkit';

import { gamesReducer } from './game';
import { roomsReducer } from './rooms';
import { serverReducer } from './server';
import { actionReducer } from './actions';

export default combineReducers({
  games: gamesReducer,
  rooms: roomsReducer,
  server: serverReducer,
  action: actionReducer
});
