import { createSlice } from '@reduxjs/toolkit';

import { accountReducers } from './server.reducer.account';
import { buddyReducers } from './server.reducer.buddies';
import { connectionReducers, initialState } from './server.reducer.connection';
import { deckReducers } from './server.reducer.decks';
import { moderationReducers } from './server.reducer.moderation';
import { replayReducers } from './server.reducer.replays';
import { userReducers } from './server.reducer.users';

export { MAX_USER_MESSAGES } from './server.reducer.users';

export const serverSlice = createSlice({
  name: 'server',
  initialState,
  reducers: {
    ...connectionReducers,
    ...buddyReducers,
    ...userReducers,
    ...moderationReducers,
    ...replayReducers,
    ...deckReducers,
    ...accountReducers,
  },
});

export const serverReducer = serverSlice.reducer;
