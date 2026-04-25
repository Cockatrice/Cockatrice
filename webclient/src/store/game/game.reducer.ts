import { createSlice } from '@reduxjs/toolkit';
import { GamesState } from './game.interfaces';
import { arrowReducers } from './game.reducer.arrow';
import { cardReducers } from './game.reducer.card';
import { chatReducers } from './game.reducer.chat';
import { counterReducers } from './game.reducer.counter';
import { lifecycleReducers } from './game.reducer.lifecycle';
import { playerReducers } from './game.reducer.player';
import { turnReducers } from './game.reducer.turn';

export { MAX_GAME_MESSAGES } from './game.reducer.helpers';

const initialState: GamesState = { games: {} };

export const gamesSlice = createSlice({
  name: 'games',
  initialState,
  reducers: {
    ...lifecycleReducers,
    ...turnReducers,
    ...playerReducers,
    ...cardReducers,
    ...counterReducers,
    ...arrowReducers,
    ...chatReducers,
  },
});

export const gamesReducer = gamesSlice.reducer;
