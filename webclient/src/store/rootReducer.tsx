import { combineReducers } from 'redux';

import { serverReducer } from './server/server.reducer';
import { roomsReducer } from './rooms/rooms.reducer';
import { reducer as formReducer } from 'redux-form'

export default combineReducers({
  server: serverReducer,
  rooms: roomsReducer,
  
  form: formReducer
});
