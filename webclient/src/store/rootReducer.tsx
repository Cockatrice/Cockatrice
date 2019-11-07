import { combineReducers } from 'redux';

import { serverReducer } from './server/server.reducer';

export default combineReducers({
	server: serverReducer
});