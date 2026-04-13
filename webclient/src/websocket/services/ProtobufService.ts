import { create, fromBinary, hasExtension, getExtension, toBinary } from '@bufbuild/protobuf';
import type { GenExtension, Message } from '@bufbuild/protobuf';

import type { Response } from 'generated/proto/response_pb';
import type { RoomEvent } from 'generated/proto/room_event_pb';
import type { SessionEvent } from 'generated/proto/session_event_pb';
import type { GameEventContainer } from 'generated/proto/game_event_container_pb';

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

export type ExtensionRegistry = Array<[GenExtension<any, any>, (...args: unknown[]) => void]>;

export class ProtobufService {
  private cmdId = 0;
  private pendingCommands: { [cmdId: string]: Function } = {};

  private webClient: WebClient;

  constructor(webClient: WebClient) {
    this.webClient = webClient;
  }

  public resetCommands() {
    this.cmdId = 0;
    this.pendingCommands = {};
  }

  public sendGameCommand(gameId: number, gameCmd: GameCommand, callback?: Function) {
    const cmd = create(CommandContainerSchema, {
      gameId,
      gameCommand: [gameCmd],
    });
    this.sendCommand(cmd, (raw: Response) => callback && callback(raw));
  }

  public sendRoomCommand(roomId: number, roomCmd: RoomCommand, callback?: Function) {
    const cmd = create(CommandContainerSchema, {
      roomId,
      roomCommand: [roomCmd],
    });
    this.sendCommand(cmd, raw => callback && callback(raw));
  }

  public sendSessionCommand(sesCmd: SessionCommand, callback?: Function) {
    const cmd = create(CommandContainerSchema, {
      sessionCommand: [sesCmd],
    });
    this.sendCommand(cmd, (raw) => callback && callback(raw));
  }

  public sendModeratorCommand(modCmd: ModeratorCommand, callback?: Function) {
    const cmd = create(CommandContainerSchema, {
      moderatorCommand: [modCmd],
    });
    this.sendCommand(cmd, (raw) => callback && callback(raw));
  }

  public sendAdminCommand(adminCmd: AdminCommand, callback?: Function) {
    const cmd = create(CommandContainerSchema, {
      adminCommand: [adminCmd],
    });
    this.sendCommand(cmd, (raw) => callback && callback(raw));
  }

  public sendCommand(cmd: CommandContainer, callback: Function) {
    this.cmdId++;

    cmd.cmdId = BigInt(this.cmdId);
    this.pendingCommands[this.cmdId] = callback;

    if (this.webClient.socket.checkReadyState(WebSocket.OPEN)) {
      this.webClient.socket.send(toBinary(CommandContainerSchema, cmd));
    }
  }

  public sendKeepAliveCommand(pingReceived: Function) {
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
    this.processEvent(event, RoomEvents, event);
  }

  private processSessionEvent(event: SessionEvent | undefined) {
    if (!event) {
      return;
    }
    this.processEvent(event, SessionEvents);
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
          (handler as Function)(getExtension(event, ext), meta);
          break;
        }
      }
    }
  }

  private processEvent(response: Message<string>, registry: ExtensionRegistry, raw?: Message) {
    for (const [ext, handler] of registry) {
      if (hasExtension(response, ext)) {
        (handler as Function)(getExtension(response, ext), raw);
        return;
      }
    }
  }
}

