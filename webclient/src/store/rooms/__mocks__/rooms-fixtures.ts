import {
  Game,
  GameSortField,
  Room,
  SortDirection,
  User,
  UserSortField,
} from 'types';
import { Message, RoomsState } from '../rooms.interfaces';

export function makeUser(overrides: Partial<User> = {}): User {
  return {
    name: 'TestUser',
    accountageSecs: 0n,
    privlevel: '',
    userLevel: 0,
    ...overrides,
  };
}

export function makeRoom(overrides: Partial<Room> = {}): Room {
  return {
    roomId: 1,
    name: 'Test Room',
    description: '',
    gameCount: 0,
    gameList: [],
    gametypeList: [],
    gametypeMap: {},
    autoJoin: false,
    permissionlevel: 0 as any,
    playerCount: 0,
    privilegelevel: 0 as any,
    userList: [],
    order: 0,
    ...overrides,
  };
}

export function makeGame(overrides: Partial<Game & { startTime: number }> = {}): Game & { startTime: number } {
  return {
    gameId: 1,
    roomId: 1,
    description: 'Test Game',
    gameType: '',
    gameTypes: [],
    started: false,
    startTime: 0,
    ...overrides,
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
