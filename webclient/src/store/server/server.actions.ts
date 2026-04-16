import { serverSlice } from './server.reducer';

export const Actions = serverSlice.actions;

export type ServerAction = ReturnType<typeof Actions[keyof typeof Actions]>;
