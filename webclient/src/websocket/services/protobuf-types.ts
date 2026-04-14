import type { GenExtension } from '@bufbuild/protobuf/codegenv2';
import type { RoomEvent } from 'generated/proto/room_event_pb';
import type { SessionEvent } from 'generated/proto/session_event_pb';
import type { GameEvent } from 'generated/proto/game_event_pb';
import type { GameEventMeta } from 'types';

type SessionRegistryEntry<V = unknown> = [
  GenExtension<SessionEvent, V>,
  (value: V) => void
];
export type SessionExtensionRegistry = SessionRegistryEntry[];

type RoomRegistryEntry<V = unknown> = [
  GenExtension<RoomEvent, V>,
  (value: V, roomEvent: RoomEvent) => void
];
export type RoomExtensionRegistry = RoomRegistryEntry[];

type GameRegistryEntry<V = unknown> = [
  GenExtension<GameEvent, V>,
  (value: V, meta: GameEventMeta) => void
];
export type GameExtensionRegistry = GameRegistryEntry[];

export function makeSessionEntry<V>(
  ext: GenExtension<SessionEvent, V>,
  handler: (value: V) => void
): SessionRegistryEntry {
  return [ext as GenExtension<SessionEvent, unknown>, handler as (value: unknown) => void];
}

export function makeRoomEntry<V>(
  ext: GenExtension<RoomEvent, V>,
  handler: (value: V, roomEvent: RoomEvent) => void
): RoomRegistryEntry {
  return [ext as GenExtension<RoomEvent, unknown>, handler as RoomRegistryEntry[1]];
}

export function makeGameEntry<V>(
  ext: GenExtension<GameEvent, V>,
  handler: (value: V, meta: GameEventMeta) => void
): GameRegistryEntry {
  return [ext as GenExtension<GameEvent, unknown>, handler as GameRegistryEntry[1]];
}
