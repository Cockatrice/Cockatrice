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

type MakeGameOverrides = MessageInitShape<typeof Data.ServerInfo_GameSchema> & {
  gameType?: string;
};

/**
 * Test fixture for Enriched.Game.
 *
 * Accepts proto field shorthands (gameId, description, etc.) which populate
 * `info`, plus the top-level client field `gameType`.
 */
export function makeGame(overrides: MakeGameOverrides = {}): Enriched.Game {
  const { gameType = '', ...protoFields } = overrides;
  return {
    info: create(Data.ServerInfo_GameSchema, {
      gameId: 1,
      roomId: 1,
      description: 'Test Game',
      gameTypes: [],
      started: false,
      ...protoFields,
    }),
    gameType,
  };
}

type MakeRoomOverrides = MessageInitShape<typeof Data.ServerInfo_RoomSchema> & {
  gametypeMap?: Enriched.GametypeMap;
  order?: number;
  games?: { [gameId: number]: Enriched.Game };
  users?: { [userName: string]: Data.ServerInfo_User };
};

/**
 * Test fixture for Enriched.Room.
 *
 * Accepts proto field shorthands (roomId, name, etc.) which populate `info`,
 * plus normalized collections (games, users, gametypeMap) and the client-only
 * `order` field.
 */
export function makeRoom(overrides: MakeRoomOverrides = {}): Enriched.Room {
  const { gametypeMap = {}, order = 0, games = {}, users = {}, ...protoFields } = overrides;
  return {
    info: create(Data.ServerInfo_RoomSchema, {
      roomId: 1,
      name: 'Test Room',
      description: '',
      gameCount: 0,
      gameList: [],
      gametypeList: [],
      autoJoin: false,
      playerCount: 0,
      userList: [],
      ...protoFields,
    }),
    gametypeMap,
    order,
    games,
    users,
  };
}

export function makeMessage(overrides: Partial<Omit<Enriched.Message, '$typeName' | '$unknown'>> = {}): Enriched.Message {
  const { timeReceived = 0, ...protoOverrides } = overrides;
  return {
    ...create(Data.Event_RoomSaySchema, {
      message: 'hello',
      messageType: 0,
      ...protoOverrides,
    }),
    timeReceived,
  };
}

export function makeRoomsState(overrides: Partial<RoomsState> = {}): RoomsState {
  return {
    rooms: {
      1: makeRoom({ roomId: 1 }),
    },
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
