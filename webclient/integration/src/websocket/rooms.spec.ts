// Room scenarios — Event_ListRooms handling, auto-join, Response_JoinRoom,
// room chat (inbound + outbound), game list updates, and leaveRoom.

import { create } from '@bufbuild/protobuf';
import { describe, expect, it } from 'vitest';

import { Data } from '@app/types';
import { store } from '@app/store';
import { RoomCommands } from '@app/websocket';

import { connectAndHandshake } from '../helpers/setup';
import {
  buildResponse,
  buildResponseMessage,
  buildRoomEventMessage,
  buildSessionEventMessage,
  deliverMessage,
} from '../helpers/protobuf-builders';
import { findLastSessionCommand, findLastRoomCommand, captureAllOutbound } from '../helpers/command-capture';
import { fromBinary, hasExtension, getExtension } from '@bufbuild/protobuf';

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

/** Deliver Event_ListRooms then join a single auto-join room, returning the roomId. */
function setupJoinedRoom(roomId = 1): void {
  deliverMessage(buildSessionEventMessage(
    Data.Event_ListRooms_ext,
    create(Data.Event_ListRoomsSchema, { roomList: [makeRoom({ roomId, autoJoin: true })] })
  ));
  const join = findLastSessionCommand(Data.Command_JoinRoom_ext);
  deliverMessage(buildResponseMessage(buildResponse({
    cmdId: join.cmdId,
    responseCode: Data.Response_ResponseCode.RespOk,
    ext: Data.Response_JoinRoom_ext,
    value: create(Data.Response_JoinRoomSchema, { roomInfo: makeRoom({ roomId }) }),
  })));
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
    setupJoinedRoom(1);

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
    setupJoinedRoom(1);

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

  it('auto-join filters correctly across multiple rooms', () => {
    connectAndHandshake();

    deliverMessage(buildSessionEventMessage(
      Data.Event_ListRooms_ext,
      create(Data.Event_ListRoomsSchema, {
        roomList: [
          makeRoom({ roomId: 1, name: 'Lobby', autoJoin: true }),
          makeRoom({ roomId: 2, name: 'Legacy', autoJoin: false }),
          makeRoom({ roomId: 3, name: 'Modern', autoJoin: true }),
        ],
      })
    ));

    // Count outbound JoinRoom commands
    const containers = captureAllOutbound();
    const joinCommands: number[] = [];
    for (const container of containers) {
      for (const cmd of container.sessionCommand ?? []) {
        if (hasExtension(cmd, Data.Command_JoinRoom_ext)) {
          joinCommands.push(getExtension(cmd, Data.Command_JoinRoom_ext).roomId);
        }
      }
    }
    expect(joinCommands).toHaveLength(2);
    expect(joinCommands).toContain(1);
    expect(joinCommands).toContain(3);
    expect(joinCommands).not.toContain(2);
  });

  it('sends outbound Command_RoomSay with trimmed message', () => {
    connectAndHandshake();
    setupJoinedRoom(1);

    RoomCommands.roomSay(1, '  hello  ');

    const { value } = findLastRoomCommand(Data.Command_RoomSay_ext);
    expect(value.message).toBe('hello');
  });

  it('removes room from joinedRoomIds on leaveRoom round-trip', () => {
    connectAndHandshake();
    setupJoinedRoom(1);
    expect(store.getState().rooms.joinedRoomIds[1]).toBe(true);

    RoomCommands.leaveRoom(1);

    const leave = findLastRoomCommand(Data.Command_LeaveRoom_ext);
    deliverMessage(buildResponseMessage(buildResponse({
      cmdId: leave.cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
    })));

    expect(store.getState().rooms.joinedRoomIds[1]).toBeUndefined();
  });

  it('tracks user join and leave within a room', () => {
    connectAndHandshake();
    setupJoinedRoom(1);

    deliverMessage(buildRoomEventMessage(1, Data.Event_JoinRoom_ext, create(Data.Event_JoinRoomSchema, {
      userInfo: create(Data.ServerInfo_UserSchema, { name: 'bob' }),
    })));

    expect(store.getState().rooms.rooms[1]?.users?.bob).toBeDefined();

    deliverMessage(buildRoomEventMessage(1, Data.Event_LeaveRoom_ext, create(Data.Event_LeaveRoomSchema, {
      name: 'bob',
    })));

    expect(store.getState().rooms.rooms[1]?.users?.bob).toBeUndefined();
  });

  it('tracks game creation and join within a room', () => {
    connectAndHandshake();
    setupJoinedRoom(1);

    RoomCommands.createGame(1, { description: 'Casual', maxPlayers: 2 });

    const create_ = findLastRoomCommand(Data.Command_CreateGame_ext);
    expect(create_.value.description).toBe('Casual');

    deliverMessage(buildResponseMessage(buildResponse({
      cmdId: create_.cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
    })));

    RoomCommands.joinGame(1, { gameId: 99 });

    const join = findLastRoomCommand(Data.Command_JoinGame_ext);
    deliverMessage(buildResponseMessage(buildResponse({
      cmdId: join.cmdId,
      responseCode: Data.Response_ResponseCode.RespOk,
    })));

    expect(store.getState().rooms.joinedGameIds[1]?.[99]).toBe(true);
  });
});