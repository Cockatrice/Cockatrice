import { createAction } from '@reduxjs/toolkit';

import { roomsSlice } from './rooms.reducer';

const SignalActions = {
  gameCreated: createAction<{ roomId: number }>('rooms/gameCreated'),
};

export const Actions = { ...roomsSlice.actions, ...SignalActions };

export type RoomsAction = ReturnType<typeof Actions[keyof typeof Actions]>;
