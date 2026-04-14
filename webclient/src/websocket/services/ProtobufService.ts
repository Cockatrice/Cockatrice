import { create, fromBinary, hasExtension, getExtension, setExtension, toBinary } from '@bufbuild/protobuf';
import type { GenExtension } from '@bufbuild/protobuf/codegenv2';

import type { Response } from 'generated/proto/response_pb';
import type { RoomEvent } from 'generated/proto/room_event_pb';
import type { SessionEvent } from 'generated/proto/session_event_pb';
import type { GameEventContainer } from 'generated/proto/game_event_container_pb';

import { GameEvents, RoomEvents, SessionEvents } from '../events';
import { GameEventMeta } from 'types';

import { CommandContainerSchema, type CommandContainer } from 'generated/proto/commands_pb';
import { ServerMessageSchema, ServerMessage_MessageType, type ServerMessage } from 'generated/proto/server_message_pb';
import { SessionCommandSchema, type SessionCommand } from 'generated/proto/session_commands_pb';
import { GameCommandSchema, type GameCommand } from 'generated/proto/game_commands_pb';
import { RoomCommandSchema, type RoomCommand } from 'generated/proto/room_commands_pb';
import { ModeratorCommandSchema, type ModeratorCommand } from 'generated/proto/moderator_commands_pb';
import { AdminCommandSchema, type AdminCommand } from 'generated/proto/admin_commands_pb';

import { type CommandOptions, handleResponse } from './command-options';

export interface SocketTransport {
  send(data: Uint8Array): void;
  isOpen(): boolean;
}

export class ProtobufService {
  private cmdId = 0;
  private pendingCommands = new Map<number, (response: Response) => void>();

  private transport: SocketTransport;

  constructor(transport: SocketTransport) {
    this.transport = transport;
  }

  public resetCommands() {
    this.cmdId = 0;
    this.pendingCommands.clear();
  }

  public sendGameCommand<V, R = unknown>(
    gameId: number,
    ext: GenExtension<GameCommand, V>,
    value: V,
    options?: CommandOptions<R>
  ): void {
    const gameCmd = create(GameCommandSchema);
    setExtension(gameCmd, ext, value);
    const cmd = create(CommandContainerSchema, { gameId, gameCommand: [gameCmd] });
    this.sendCommand(cmd, raw => {
      if (options) {
        handleResponse(ext.typeName, raw, options);
      }
    });
  }

  public sendRoomCommand<V, R = unknown>(
    roomId: number,
    ext: GenExtension<RoomCommand, V>,
    value: V,
    options?: CommandOptions<R>
  ): void {
    const roomCmd = create(RoomCommandSchema);
    setExtension(roomCmd, ext, value);
    const cmd = create(CommandContainerSchema, { roomId, roomCommand: [roomCmd] });
    this.sendCommand(cmd, raw => {
      if (options) {
        handleResponse(ext.typeName, raw, options);
      }
    });
  }

  public sendSessionCommand<V, R = unknown>(
    ext: GenExtension<SessionCommand, V>,
    value: V,
    options?: CommandOptions<R>
  ): void {
    const sesCmd = create(SessionCommandSchema);
    setExtension(sesCmd, ext, value);
    const cmd = create(CommandContainerSchema, { sessionCommand: [sesCmd] });
    this.sendCommand(cmd, raw => {
      if (options) {
        handleResponse(ext.typeName, raw, options);
      }
    });
  }

  public sendModeratorCommand<V, R = unknown>(
    ext: GenExtension<ModeratorCommand, V>,
    value: V,
    options?: CommandOptions<R>
  ): void {
    const modCmd = create(ModeratorCommandSchema);
    setExtension(modCmd, ext, value);
    const cmd = create(CommandContainerSchema, { moderatorCommand: [modCmd] });
    this.sendCommand(cmd, raw => {
      if (options) {
        handleResponse(ext.typeName, raw, options);
      }
    });
  }

  public sendAdminCommand<V, R = unknown>(
    ext: GenExtension<AdminCommand, V>,
    value: V,
    options?: CommandOptions<R>
  ): void {
    const adminCmd = create(AdminCommandSchema);
    setExtension(adminCmd, ext, value);
    const cmd = create(CommandContainerSchema, { adminCommand: [adminCmd] });
    this.sendCommand(cmd, raw => {
      if (options) {
        handleResponse(ext.typeName, raw, options);
      }
    });
  }

  public sendCommand(cmd: CommandContainer, callback: (raw: Response) => void) {
    this.cmdId++;

    cmd.cmdId = BigInt(this.cmdId);
    this.pendingCommands.set(this.cmdId, callback);

    if (this.transport.isOpen()) {
      this.transport.send(toBinary(CommandContainerSchema, cmd));
    }
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

    if (this.pendingCommands.has(cmdId)) {
      this.pendingCommands.get(cmdId)!(response);
      this.pendingCommands.delete(cmdId);
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

