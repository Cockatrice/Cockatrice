import { gamesSlice } from './game.reducer';

export const Actions = gamesSlice.actions;

export type GameAction = ReturnType<typeof Actions[keyof typeof Actions]>;
