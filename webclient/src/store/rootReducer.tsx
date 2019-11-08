import { combineReducers } from 'redux';

import { serverReducer } from './server/server.reducer';
import { roomsReducer } from './rooms/rooms.reducer';

export default combineReducers({
	server: serverReducer,
	rooms: roomsReducer
});