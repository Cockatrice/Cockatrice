import { create, fromBinary, hasExtension, getExtension, setExtension, toBinary } from '@bufbuild/protobuf';
import type { GenExtension } from '@bufbuild/protobuf/codegenv2';


import { GameEvents, RoomEvents, SessionEvents } from '../events';
import { Data, Enriched } from '@app/types';


import { type CommandOptions, handleResponse } from './command-options';

export interface SocketTransport {
  send(data: Uint8Array): void;
  isOpen(): boolean;
}

export class ProtobufService {
  private cmdId = 0;
  private pendingCommands = new Map<number, (response: Data.Response) => void>();

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
    ext: GenExtension<Data.GameCommand, V>,
    value: V,
    options?: CommandOptions<R>
  ): void {
    const gameCmd = create(Data.GameCommandSchema);
    setExtension(gameCmd, ext, value);
    const cmd = create(Data.CommandContainerSchema, { gameId, gameCommand: [gameCmd] });
    this.sendCommand(cmd, raw => {
      if (options) {
        handleResponse(ext.typeName, raw, options);
      }
    });
  }

  public sendRoomCommand<V, R = unknown>(
    roomId: number,
    ext: GenExtension<Data.RoomCommand, V>,
    value: V,
    options?: CommandOptions<R>
  ): void {
    const roomCmd = create(Data.RoomCommandSchema);
    setExtension(roomCmd, ext, value);
    const cmd = create(Data.CommandContainerSchema, { roomId, roomCommand: [roomCmd] });
    this.sendCommand(cmd, raw => {
      if (options) {
        handleResponse(ext.typeName, raw, options);
      }
    });
  }

  public sendSessionCommand<V, R = unknown>(
    ext: GenExtension<Data.SessionCommand, V>,
    value: V,
    options?: CommandOptions<R>
  ): void {
    const sesCmd = create(Data.SessionCommandSchema);
    setExtension(sesCmd, ext, value);
    const cmd = create(Data.CommandContainerSchema, { sessionCommand: [sesCmd] });
    this.sendCommand(cmd, raw => {
      if (options) {
        handleResponse(ext.typeName, raw, options);
      }
    });
  }

  public sendModeratorCommand<V, R = unknown>(
    ext: GenExtension<Data.ModeratorCommand, V>,
    value: V,
    options?: CommandOptions<R>
  ): void {
    const modCmd = create(Data.ModeratorCommandSchema);
    setExtension(modCmd, ext, value);
    const cmd = create(Data.CommandContainerSchema, { moderatorCommand: [modCmd] });
    this.sendCommand(cmd, raw => {
      if (options) {
        handleResponse(ext.typeName, raw, options);
      }
    });
  }

  public sendAdminCommand<V, R = unknown>(
    ext: GenExtension<Data.AdminCommand, V>,
    value: V,
    options?: CommandOptions<R>
  ): void {
    const adminCmd = create(Data.AdminCommandSchema);
    setExtension(adminCmd, ext, value);
    const cmd = create(Data.CommandContainerSchema, { adminCommand: [adminCmd] });
    this.sendCommand(cmd, raw => {
      if (options) {
        handleResponse(ext.typeName, raw, options);
      }
    });
  }

  public sendCommand(cmd: Data.CommandContainer, callback: (raw: Data.Response) => void) {
    if (!this.transport.isOpen()) {
      return;
    }

    this.cmdId++;
    cmd.cmdId = BigInt(this.cmdId);
    this.pendingCommands.set(this.cmdId, callback);
    this.transport.send(toBinary(Data.CommandContainerSchema, cmd));
  }

  public handleMessageEvent({ data }: MessageEvent): void {
    try {
      const uint8msg = new Uint8Array(data);
      const msg: Data.ServerMessage = fromBinary(Data.ServerMessageSchema, uint8msg);

      if (msg) {
        switch (msg.messageType) {
          case Data.ServerMessage_MessageType.RESPONSE:
            this.processServerResponse(msg.response);
            break;
          case Data.ServerMessage_MessageType.ROOM_EVENT:
            this.processRoomEvent(msg.roomEvent);
            break;
          case Data.ServerMessage_MessageType.SESSION_EVENT:
            this.processSessionEvent(msg.sessionEvent);
            break;
          case Data.ServerMessage_MessageType.GAME_EVENT_CONTAINER:
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

  private processServerResponse(response: Data.Response | undefined) {
    if (!response) {
      return;
    }
    const cmdId = Number(response.cmdId);

    if (this.pendingCommands.has(cmdId)) {
      this.pendingCommands.get(cmdId)!(response);
      this.pendingCommands.delete(cmdId);
    }
  }

  private processRoomEvent(event: Data.RoomEvent | undefined) {
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

  private processSessionEvent(event: Data.SessionEvent | undefined) {
    if (!event) {
      return;
    }
    for (const [ext, handler] of SessionEvents) {
      if (hasExtension(event, ext)) {
        handler(getExtension(event, ext), undefined);
        return;
      }
    }
  }

  private processGameEvent(container: Data.GameEventContainer | undefined): void {
    if (!container?.eventList?.length) {
      return;
    }

    const { gameId, context, secondsElapsed, forcedByJudge } = container;

    for (const event of container.eventList) {
      const meta: Enriched.GameEventMeta = {
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

