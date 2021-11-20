import { combineReducers } from 'redux';

import { roomsReducer } from './rooms';
import { serverReducer } from './server';
import { reducer as formReducer } from 'redux-form'
import { actionReducer } from './actions'

export default combineReducers({
  rooms: roomsReducer,
  server: serverReducer,

  form: formReducer,
  action: actionReducer
});
