import { create, fromBinary, hasExtension, getExtension, toBinary } from '@bufbuild/protobuf';
import type { GenExtension } from '@bufbuild/protobuf/codegenv2';

import type { Response } from 'generated/proto/response_pb';
import type { RoomEvent } from 'generated/proto/room_event_pb';
import type { SessionEvent } from 'generated/proto/session_event_pb';
import type { GameEventContainer } from 'generated/proto/game_event_container_pb';
import type { GameEvent } from 'generated/proto/game_event_pb';

import { GameEvents, RoomEvents, SessionEvents } from '../events';
import { WebClient } from '../WebClient';
import { SessionCommands } from 'websocket';
import { GameEventMeta } from 'types';

import { CommandContainerSchema, type CommandContainer } from 'generated/proto/commands_pb';
import { ServerMessageSchema, ServerMessage_MessageType, type ServerMessage } from 'generated/proto/server_message_pb';
import type { SessionCommand } from 'generated/proto/session_commands_pb';
import type { GameCommand } from 'generated/proto/game_commands_pb';
import type { RoomCommand } from 'generated/proto/room_commands_pb';
import type { ModeratorCommand } from 'generated/proto/moderator_commands_pb';
import type { AdminCommand } from 'generated/proto/admin_commands_pb';

// Per-family registry entry types. Each family hardcodes its parent message type
// and the handler's exact secondary-argument signature, eliminating the previous
// `...args: unknown[]` erasure.

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

/**
 * Type-safe factory functions. The compiler verifies at the call site that the
 * handler's parameter types match the extension's value type and the family's
 * secondary argument type.
 */
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

export class ProtobufService {
  private cmdId = 0;
  private pendingCommands: { [cmdId: string]: (response: Response) => void } = {};

  private webClient: WebClient;

  constructor(webClient: WebClient) {
    this.webClient = webClient;
  }

  public resetCommands() {
    this.cmdId = 0;
    this.pendingCommands = {};
  }

  public sendGameCommand(gameId: number, gameCmd: GameCommand, callback?: (raw: Response) => void) {
    const cmd = create(CommandContainerSchema, {
      gameId,
      gameCommand: [gameCmd],
    });
    this.sendCommand(cmd, (raw: Response) => callback?.(raw));
  }

  public sendRoomCommand(roomId: number, roomCmd: RoomCommand, callback?: (raw: Response) => void) {
    const cmd = create(CommandContainerSchema, {
      roomId,
      roomCommand: [roomCmd],
    });
    this.sendCommand(cmd, raw => callback?.(raw));
  }

  public sendSessionCommand(sesCmd: SessionCommand, callback?: (raw: Response) => void) {
    const cmd = create(CommandContainerSchema, {
      sessionCommand: [sesCmd],
    });
    this.sendCommand(cmd, (raw) => callback?.(raw));
  }

  public sendModeratorCommand(modCmd: ModeratorCommand, callback?: (raw: Response) => void) {
    const cmd = create(CommandContainerSchema, {
      moderatorCommand: [modCmd],
    });
    this.sendCommand(cmd, (raw) => callback?.(raw));
  }

  public sendAdminCommand(adminCmd: AdminCommand, callback?: (raw: Response) => void) {
    const cmd = create(CommandContainerSchema, {
      adminCommand: [adminCmd],
    });
    this.sendCommand(cmd, (raw) => callback?.(raw));
  }

  public sendCommand(cmd: CommandContainer, callback: (raw: Response) => void) {
    this.cmdId++;

    cmd.cmdId = BigInt(this.cmdId);
    this.pendingCommands[this.cmdId] = callback;

    if (this.webClient.socket.checkReadyState(WebSocket.OPEN)) {
      this.webClient.socket.send(toBinary(CommandContainerSchema, cmd));
    }
  }

  public sendKeepAliveCommand(pingReceived: () => void) {
    SessionCommands.ping(pingReceived);
  }

  public handleMessageEvent({ data }: MessageEvent): void {
    try {
      const uint8msg = new Uint8Array(data);
      const msg: ServerMessage = fromBinary(ServerMessageSchema, uint8msg);

      if (msg) {
        switch (msg.messageType) {
          case ServerMessage_MessageType.RESPONSE:
            this.processServerResponse(msg.response);
            break;
          case ServerMessage_MessageType.ROOM_EVENT:
            this.processRoomEvent(msg.roomEvent);
            break;
          case ServerMessage_MessageType.SESSION_EVENT:
            this.processSessionEvent(msg.sessionEvent);
            break;
          case ServerMessage_MessageType.GAME_EVENT_CONTAINER:
            this.processGameEvent(msg.gameEventContainer);
            break;
          default:
            console.log(msg);
            break;
        }
      }
    } catch (err) {
      console.error('Processing failed:', err);
    }
  }

  private processServerResponse(response: Response | undefined) {
    if (!response) {
      return;
    }
    const cmdId = Number(response.cmdId);

    if (this.pendingCommands[cmdId]) {
      this.pendingCommands[cmdId](response);
      delete this.pendingCommands[cmdId];
    }
  }

  private processRoomEvent(event: RoomEvent | undefined) {
    if (!event) {
      return;
    }
    for (const [ext, handler] of RoomEvents) {
      if (hasExtension(event, ext)) {
        handler(getExtension(event, ext), event);
        return;
      }
    }
  }

  private processSessionEvent(event: SessionEvent | undefined) {
    if (!event) {
      return;
    }
    for (const [ext, handler] of SessionEvents) {
      if (hasExtension(event, ext)) {
        handler(getExtension(event, ext));
        return;
      }
    }
  }

  private processGameEvent(container: GameEventContainer | undefined): void {
    if (!container?.eventList?.length) {
      return;
    }

    const { gameId, context, secondsElapsed, forcedByJudge } = container;

    for (const event of container.eventList) {
      const meta: GameEventMeta = {
        gameId: gameId ?? -1,
        playerId: event.playerId ?? -1,
        context,
        secondsElapsed: secondsElapsed ?? 0,
        forcedByJudge: forcedByJudge ?? 0,
      };

      for (const [ext, handler] of GameEvents) {
        if (hasExtension(event, ext)) {
          handler(getExtension(event, ext), meta);
          break;
        }
      }
    }
  }

}

