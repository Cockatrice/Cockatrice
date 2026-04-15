import { App, Data, Enriched } from '@app/types';
import type { MessageInitShape } from '@bufbuild/protobuf';

import { create } from '@bufbuild/protobuf';
import { RoomsState } from '../rooms.interfaces';

export function makeUser(
  overrides: MessageInitShape<typeof Data.ServerInfo_UserSchema> = {}
): Data.ServerInfo_User {
  return create(Data.ServerInfo_UserSchema, {
    name: 'TestUser',
    accountageSecs: 0n,
    privlevel: '',
    userLevel: 0,
    ...overrides,
  });
}

export function makeRoom(overrides: Partial<Omit<Enriched.Room, '$typeName' | '$unknown'>> = {}): Enriched.Room {
  const { gametypeMap = {}, order = 0, gameList = [], ...protoOverrides } = overrides;
  return {
    ...create(Data.ServerInfo_RoomSchema, {
      roomId: 1,
      name: 'Test Room',
      description: '',
      gameCount: 0,
      gameList: [],
      gametypeList: [],
      autoJoin: false,
      playerCount: 0,
      userList: [],
      ...protoOverrides,
    }),
    gameList,
    gametypeMap,
    order,
  };
}

export function makeGame(
  overrides: Partial<Omit<Enriched.Game & { startTime: number }, '$typeName' | '$unknown'>> = {},
): Enriched.Game & { startTime: number } {
  const { gameType = '', startTime = 0, ...protoOverrides } = overrides;
  return {
    ...create(Data.ServerInfo_GameSchema, {
      gameId: 1,
      roomId: 1,
      description: 'Test Game',
      gameTypes: [],
      started: false,
      ...protoOverrides,
    }),
    gameType,
    startTime,
  };
}

export function makeMessage(overrides: Partial<Enriched.Message> = {}): Enriched.Message {
  return {
    message: 'hello',
    messageType: 0,
    timeReceived: 0,
    ...overrides,
  };
}

export function makeRoomsState(overrides: Partial<RoomsState> = {}): RoomsState {
  return {
    rooms: {
      1: makeRoom({ roomId: 1 }),
    },
    games: {},
    joinedRoomIds: {},
    joinedGameIds: {},
    messages: {},
    sortGamesBy: {
      field: App.GameSortField.START_TIME,
      order: App.SortDirection.DESC,
    },
    sortUsersBy: {
      field: App.UserSortField.NAME,
      order: App.SortDirection.ASC,
    },
    ...overrides,
  };
}
