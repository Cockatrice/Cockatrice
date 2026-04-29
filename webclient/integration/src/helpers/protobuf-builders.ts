// Factory helpers that build encoded `ServerMessage` binaries for the four
// top-level message types the client consumes (RESPONSE, SESSION_EVENT,
// ROOM_EVENT, GAME_EVENT_CONTAINER). Tests call these to simulate incoming
// server traffic and then hand the resulting bytes to `deliverMessage()`.
//
// No mocking of `@bufbuild/protobuf` — every builder uses the real `create`/
// `setExtension`/`toBinary` path so the bytes that land in ProtobufService
// are byte-for-byte identical to what a Servatrice would send.

import { create, setExtension, toBinary } from '@bufbuild/protobuf';
import type { GenExtension, GenMessage } from '@bufbuild/protobuf/codegenv2';
import type { MessageInitShape } from '@bufbuild/protobuf';

import { Data } from '@app/types';

import { getMockWebSocket } from './setup';

/**
 * Convenience wrapper around `create` for schemas that accept an init shape.
 * Mirrors the pattern used throughout the webclient codebase.
 */
export function make<S extends GenMessage<any>>(
  schema: S,
  init?: MessageInitShape<S>
): ReturnType<typeof create<S>> {
  return create(schema, init);
}

/** Build a top-level ServerMessage wrapping a Response. */
export function buildResponseMessage(response: Data.Response): Uint8Array {
  const msg = create(Data.ServerMessageSchema, {
    messageType: Data.ServerMessage_MessageType.RESPONSE,
    response,
  });
  return toBinary(Data.ServerMessageSchema, msg);
}

/**
 * Build a Response with an optional response-payload extension attached.
 * `cmdId` must match the outbound command the test is responding to —
 * callers typically read it from `captureOutbound()`.
 */
export function buildResponse<V>(params: {
  cmdId: number;
  responseCode?: Data.Response_ResponseCode;
  ext?: GenExtension<Data.Response, V>;
  value?: V;
}): Data.Response {
  const response = create(Data.ResponseSchema, {
    cmdId: BigInt(params.cmdId),
    responseCode: params.responseCode ?? Data.Response_ResponseCode.RespOk,
  });
  if (params.ext && params.value !== undefined) {
    setExtension(response, params.ext, params.value);
  }
  return response;
}

/** Build a top-level ServerMessage wrapping a SessionEvent with the given extension. */
export function buildSessionEventMessage<V>(
  ext: GenExtension<Data.SessionEvent, V>,
  value: V
): Uint8Array {
  const sessionEvent = create(Data.SessionEventSchema);
  setExtension(sessionEvent, ext, value);
  const msg = create(Data.ServerMessageSchema, {
    messageType: Data.ServerMessage_MessageType.SESSION_EVENT,
    sessionEvent,
  });
  return toBinary(Data.ServerMessageSchema, msg);
}

/** Build a top-level ServerMessage wrapping a RoomEvent with the given extension. */
export function buildRoomEventMessage<V>(
  roomId: number,
  ext: GenExtension<Data.RoomEvent, V>,
  value: V
): Uint8Array {
  const roomEvent = create(Data.RoomEventSchema, { roomId });
  setExtension(roomEvent, ext, value);
  const msg = create(Data.ServerMessageSchema, {
    messageType: Data.ServerMessage_MessageType.ROOM_EVENT,
    roomEvent,
  });
  return toBinary(Data.ServerMessageSchema, msg);
}

/**
 * Build a top-level ServerMessage wrapping a GameEventContainer whose
 * `eventList` contains a single GameEvent with the given extension attached.
 */
export function buildGameEventMessage<V>(
  params: {
    gameId: number;
    playerId?: number;
    ext: GenExtension<Data.GameEvent, V>;
    value: V;
  }
): Uint8Array {
  const gameEvent = create(Data.GameEventSchema, {
    playerId: params.playerId ?? -1,
  });
  setExtension(gameEvent, params.ext, params.value);
  const container = create(Data.GameEventContainerSchema, {
    gameId: params.gameId,
    eventList: [gameEvent],
  });
  const msg = create(Data.ServerMessageSchema, {
    messageType: Data.ServerMessage_MessageType.GAME_EVENT_CONTAINER,
    gameEventContainer: container,
  });
  return toBinary(Data.ServerMessageSchema, msg);
}

/**
 * Deliver an encoded ServerMessage to the currently-connected mock socket.
 * WebSocketService wires `onmessage` to push events into its RxJS subject,
 * which ProtobufService subscribes to — so this triggers the full inbound
 * pipeline synchronously.
 */
export function deliverMessage(binary: Uint8Array): void {
  const mock = getMockWebSocket();
  const event = { data: binary.buffer } as MessageEvent;
  mock.onmessage?.(event);
}
