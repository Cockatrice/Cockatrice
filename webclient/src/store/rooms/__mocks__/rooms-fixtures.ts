import {
  Game,
  GameSortField,
  Message,
  ProtoInit,
  Room,
  SortDirection,
  User,
  UserSortField,
} from 'types';
import { create } from '@bufbuild/protobuf';
import { ServerInfo_UserSchema } from 'generated/proto/serverinfo_user_pb';
import { ServerInfo_GameSchema } from 'generated/proto/serverinfo_game_pb';
import { ServerInfo_RoomSchema } from 'generated/proto/serverinfo_room_pb';
import { RoomsState } from '../rooms.interfaces';

export function makeUser(overrides: ProtoInit<User> = {}): User {
  return create(ServerInfo_UserSchema, {
    name: 'TestUser',
    accountageSecs: 0n,
    privlevel: '',
    userLevel: 0,
    ...overrides,
  });
}

export function makeRoom(overrides: ProtoInit<Room> = {}): Room {
  const { gametypeMap = {}, order = 0, gameList = [], ...protoOverrides } = overrides;
  return {
    ...create(ServerInfo_RoomSchema, {
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

export function makeGame(overrides: ProtoInit<Game & { startTime: number }> = {}): Game & { startTime: number } {
  const { gameType = '', startTime = 0, ...protoOverrides } = overrides;
  return {
    ...create(ServerInfo_GameSchema, {
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

export function makeMessage(overrides: Partial<Message> = {}): Message {
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
      field: GameSortField.START_TIME,
      order: SortDirection.DESC,
    },
    sortUsersBy: {
      field: UserSortField.NAME,
      order: SortDirection.ASC,
    },
    ...overrides,
  };
}
