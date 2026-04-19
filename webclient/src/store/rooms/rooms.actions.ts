import { roomsSlice } from './rooms.reducer';

export const Actions = roomsSlice.actions;

export type RoomsAction = ReturnType<typeof Actions[keyof typeof Actions]>;
