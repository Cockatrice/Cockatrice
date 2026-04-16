// Room scenarios — Event_ListRooms handling, auto-join, Response_JoinRoom,
// room chat, and in-room game list updates.

import { create } from '@bufbuild/protobuf';
import { describe, expect, it } from 'vitest';

import { Data } from '@app/types';
import { store } from '@app/store';

import { connectAndHandshake } from './helpers/setup';
import {
  buildResponse,
  buildResponseMessage,
  buildRoomEventMessage,
  buildSessionEventMessage,
  deliverMessage,
} from './helpers/protobuf-builders';
import { findLastSessionCommand } from './helpers/command-capture';

function makeRoom(overrides: Partial<{
  roomId: number;
  name: string;
  autoJoin: boolean;
}> = {}): Data.ServerInfo_Room {
  return create(Data.ServerInfo_RoomSchema, {
    roomId: overrides.roomId ?? 1,
    name: overrides.name ?? 'Lobby',
    description: 'Test room',
    gameCount: 0,
    playerCount: 0,
    autoJoin: overrides.autoJoin ?? false,
    gameList: [],
    userList: [],
    gametypeList: [],
  });
}

describe('rooms', () => {
  it('populates rooms state from Event_ListRooms', () => {
    connectAndHandshake();

    const listRooms = create(Data.Event_ListRoomsSchema, {
      roomList: [
        makeRoom({ roomId: 1, name: 'Lobby' }),
        makeRoom({ roomId: 2, name: 'Legacy' }),
      ],
    });
    deliverMessage(buildSessionEventMessage(Data.Event_ListRooms_ext, listRooms));

    const { rooms } = store.getState().rooms;
    expect(rooms[1]?.info?.name).toBe('Lobby');
    expect(rooms[2]?.info?.name).toBe('Legacy');
  });

  it('auto-joins rooms flagged with autoJoin and flips joinedRoomIds on Response_JoinRoom', () => {
    connectAndHandshake();

    const listRooms = create(Data.Event_ListRoomsSchema, {
      roomList: [
        makeRoom({ roomId: 1, name: 'Lobby', autoJoin: true }),
        makeRoom({ roomId: 2, name: 'Legacy', autoJoin: false }),
      ],
    });
    deliverMessage(buildSessionEventMessage(Data.Event_ListRooms_ext, listRooms));

    const join = findLastSessionCommand(Data.Command_JoinRoom_ext);
    expect(join.value.roomId).toBe(1);

    const joined = create(Data.Response_JoinRoomSchema, {
      roomInfo: makeRoom({ roomId: 1, name: 'Lobby' }),
    });
    deliverMessage(buildResponseMessage(buildResponse({
      cmdId: join.cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
      ext: Data.Response_JoinRoom_ext,
      value: joined,
    })));

    expect(store.getState().rooms.joinedRoomIds[1]).toBe(true);
  });

  it('appends a room chat message on Event_RoomSay', () => {
    connectAndHandshake();

    deliverMessage(buildSessionEventMessage(
      Data.Event_ListRooms_ext,
      create(Data.Event_ListRoomsSchema, { roomList: [makeRoom({ roomId: 1, autoJoin: true })] })
    ));
    const join = findLastSessionCommand(Data.Command_JoinRoom_ext);
    deliverMessage(buildResponseMessage(buildResponse({
      cmdId: join.cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
      ext: Data.Response_JoinRoom_ext,
      value: create(Data.Response_JoinRoomSchema, { roomInfo: makeRoom({ roomId: 1 }) }),
    })));

    const say = create(Data.Event_RoomSaySchema, {
      name: 'bob',
      message: 'hello world',
      messageType: Data.Event_RoomSay_RoomMessageType.UserMessage,
    });
    deliverMessage(buildRoomEventMessage(1, Data.Event_RoomSay_ext, say));

    const messages = store.getState().rooms.messages[1];
    expect(messages).toHaveLength(1);
    expect(messages[0].message).toBe('bob: hello world');
    expect(messages[0].name).toBe('bob');
  });

  it('updates the game list on Event_ListGames', () => {
    connectAndHandshake();

    deliverMessage(buildSessionEventMessage(
      Data.Event_ListRooms_ext,
      create(Data.Event_ListRoomsSchema, { roomList: [makeRoom({ roomId: 1, autoJoin: true })] })
    ));
    const join = findLastSessionCommand(Data.Command_JoinRoom_ext);
    deliverMessage(buildResponseMessage(buildResponse({
      cmdId: join.cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
      ext: Data.Response_JoinRoom_ext,
      value: create(Data.Response_JoinRoomSchema, { roomInfo: makeRoom({ roomId: 1 }) }),
    })));

    const game = create(Data.ServerInfo_GameSchema, {
      gameId: 42,
      description: 'Test Game',
      maxPlayers: 4,
      playerCount: 1,
      startTime: 1,
    });
    const listGames = create(Data.Event_ListGamesSchema, { gameList: [game] });
    deliverMessage(buildRoomEventMessage(1, Data.Event_ListGames_ext, listGames));

    const roomGames = store.getState().rooms.rooms[1]?.games;
    expect(roomGames).toBeDefined();
    expect(roomGames?.[42]?.info?.description).toBe('Test Game');
    expect(roomGames?.[42]?.info?.gameId).toBe(42);
  });
});
