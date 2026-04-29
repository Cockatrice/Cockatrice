// Helpers for inspecting outbound commands. WebSocketService calls
// `this.socket.send(bytes)` with the encoded CommandContainer; the mock
// WebSocket records those calls on its `send` vi.fn. These helpers decode
// the bytes back into a CommandContainer so tests can assert on what was
// sent and extract the `cmdId` needed to build a correlated response.

import { fromBinary, getExtension, hasExtension } from '@bufbuild/protobuf';
import type { GenExtension } from '@bufbuild/protobuf/codegenv2';

import { Data } from '@app/types';

import { getMockWebSocket } from './setup';

/** The command scopes a CommandContainer can carry in practice. */
type SessionCmd = Data.SessionCommand;
type RoomCmd = Data.RoomCommand;
type GameCmd = Data.GameCommand;
type AdminCmd = Data.AdminCommand;
type ModeratorCmd = Data.ModeratorCommand;

/** Decode every CommandContainer sent through the mock socket so far. */
export function captureAllOutbound(): Data.CommandContainer[] {
  const mock = getMockWebSocket();
  return mock.send.mock.calls.map(([bytes]: [Uint8Array]) =>
    fromBinary(Data.CommandContainerSchema, bytes)
  );
}

/** Decode the most recent CommandContainer. Throws if none has been sent. */
export function captureLastOutbound(): Data.CommandContainer {
  const all = captureAllOutbound();
  if (all.length === 0) {
    throw new Error('No outbound command has been sent through the mock WebSocket.');
  }
  return all[all.length - 1];
}

/** Numeric cmdId of the most recently sent command (the BigInt cast back to number). */
export function lastCmdId(): number {
  return Number(captureLastOutbound().cmdId);
}

/**
 * Find the most recently sent CommandContainer whose session-scope command
 * carries the given extension, and return both the container and the
 * unwrapped extension value. Handy for "the login() call fired — grab its
 * cmdId and the Command_Login payload it sent".
 */
export function findLastSessionCommand<V>(
  ext: GenExtension<SessionCmd, V>
): { container: Data.CommandContainer; value: V; cmdId: number } {
  const containers = captureAllOutbound();
  for (let i = containers.length - 1; i >= 0; i--) {
    const container = containers[i];
    for (const sessionCmd of container.sessionCommand ?? []) {
      if (hasExtension(sessionCmd, ext)) {
        return {
          container,
          value: getExtension(sessionCmd, ext),
          cmdId: Number(container.cmdId),
        };
      }
    }
  }
  throw new Error(
    `No outbound session command with extension ${ext.typeName} has been sent.`
  );
}

/** Room-scoped equivalent of {@link findLastSessionCommand}. */
export function findLastRoomCommand<V>(
  ext: GenExtension<RoomCmd, V>
): { container: Data.CommandContainer; value: V; cmdId: number; roomId: number } {
  const containers = captureAllOutbound();
  for (let i = containers.length - 1; i >= 0; i--) {
    const container = containers[i];
    for (const roomCmd of container.roomCommand ?? []) {
      if (hasExtension(roomCmd, ext)) {
        return {
          container,
          value: getExtension(roomCmd, ext),
          cmdId: Number(container.cmdId),
          roomId: container.roomId ?? 0,
        };
      }
    }
  }
  throw new Error(
    `No outbound room command with extension ${ext.typeName} has been sent.`
  );
}

/** Game-scoped equivalent of {@link findLastSessionCommand}. */
export function findLastGameCommand<V>(
  ext: GenExtension<GameCmd, V>
): { container: Data.CommandContainer; value: V; cmdId: number; gameId: number } {
  const containers = captureAllOutbound();
  for (let i = containers.length - 1; i >= 0; i--) {
    const container = containers[i];
    for (const gameCmd of container.gameCommand ?? []) {
      if (hasExtension(gameCmd, ext)) {
        return {
          container,
          value: getExtension(gameCmd, ext),
          cmdId: Number(container.cmdId),
          gameId: container.gameId ?? 0,
        };
      }
    }
  }
  throw new Error(
    `No outbound game command with extension ${ext.typeName} has been sent.`
  );
}

/** Admin-scoped equivalent of {@link findLastSessionCommand}. */
export function findLastAdminCommand<V>(
  ext: GenExtension<AdminCmd, V>
): { container: Data.CommandContainer; value: V; cmdId: number } {
  const containers = captureAllOutbound();
  for (let i = containers.length - 1; i >= 0; i--) {
    const container = containers[i];
    for (const adminCmd of container.adminCommand ?? []) {
      if (hasExtension(adminCmd, ext)) {
        return {
          container,
          value: getExtension(adminCmd, ext),
          cmdId: Number(container.cmdId),
        };
      }
    }
  }
  throw new Error(
    `No outbound admin command with extension ${ext.typeName} has been sent.`
  );
}

/** Moderator-scoped equivalent of {@link findLastSessionCommand}. */
export function findLastModeratorCommand<V>(
  ext: GenExtension<ModeratorCmd, V>
): { container: Data.CommandContainer; value: V; cmdId: number } {
  const containers = captureAllOutbound();
  for (let i = containers.length - 1; i >= 0; i--) {
    const container = containers[i];
    for (const modCmd of container.moderatorCommand ?? []) {
      if (hasExtension(modCmd, ext)) {
        return {
          container,
          value: getExtension(modCmd, ext),
          cmdId: Number(container.cmdId),
        };
      }
    }
  }
  throw new Error(
    `No outbound moderator command with extension ${ext.typeName} has been sent.`
  );
}
