import { GameEvents, RoomEvents, SessionEvents } from '../events';
import { WebClient } from '../WebClient';
import { SessionCommands } from 'websocket';
import { ProtoController } from './ProtoController';
import { GameEventMeta } from 'types';

export interface ProtobufEvents {
  [event: string]: Function;
}

export class ProtobufService {
  private cmdId = 0;
  private pendingCommands: { [cmdId: string]: Function } = {};

  private webClient: WebClient;

  constructor(webClient: WebClient) {
    this.webClient = webClient;
    ProtoController.load();
  }

  public resetCommands() {
    this.cmdId = 0;
    this.pendingCommands = {};
  }

  public sendGameCommand(gameId: number, gameCmd: any, callback?: Function) {
    const cmd = ProtoController.root.CommandContainer.create({
      gameId,
      gameCommand: [gameCmd],
    });

    this.sendCommand(cmd, (raw: any) => callback && callback(raw));
  }

  public sendRoomCommand(roomId: number, roomCmd: any, callback?: Function) {
    const cmd = ProtoController.root.CommandContainer.create({
      'roomId': roomId,
      'roomCommand': [roomCmd]
    });

    this.sendCommand(cmd, raw => callback && callback(raw));
  }

  public sendSessionCommand(sesCmd: any, callback?: Function) {
    const cmd = ProtoController.root.CommandContainer.create({
      'sessionCommand': [sesCmd]
    });

    this.sendCommand(cmd, (raw) => callback && callback(raw));
  }

  public sendModeratorCommand(modCmd: any, callback?: Function) {
    const cmd = ProtoController.root.CommandContainer.create({
      'moderatorCommand': [modCmd]
    });

    this.sendCommand(cmd, (raw) => callback && callback(raw));
  }

  public sendAdminCommand(adminCmd: any, callback?: Function) {
    const cmd = ProtoController.root.CommandContainer.create({
      'adminCommand': [adminCmd]
    });

    this.sendCommand(cmd, (raw) => callback && callback(raw));
  }

  public sendCommand(cmd: any, callback: Function) {
    this.cmdId++;

    cmd['cmdId'] = this.cmdId;
    this.pendingCommands[this.cmdId] = callback;

    if (this.webClient.socket.checkReadyState(WebSocket.OPEN)) {
      this.webClient.socket.send(ProtoController.root.CommandContainer.encode(cmd).finish());
    }
  }

  public sendKeepAliveCommand(pingReceived: Function) {
    SessionCommands.ping(pingReceived);
  }

  public handleMessageEvent({ data }: MessageEvent): void {
    try {
      const uint8msg = new Uint8Array(data);
      const msg = ProtoController.root.ServerMessage.decode(uint8msg);

      if (msg) {
        switch (msg.messageType) {
          case ProtoController.root.ServerMessage.MessageType.RESPONSE:
            this.processServerResponse(msg.response);
            break;
          case ProtoController.root.ServerMessage.MessageType.ROOM_EVENT:
            this.processRoomEvent(msg.roomEvent, msg);
            break;
          case ProtoController.root.ServerMessage.MessageType.SESSION_EVENT:
            this.processSessionEvent(msg.sessionEvent, msg);
            break;
          case ProtoController.root.ServerMessage.MessageType.GAME_EVENT_CONTAINER:
            this.processGameEvent(msg.gameEventContainer, msg);
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

  private processServerResponse(response: any) {
    const { cmdId } = response;

    if (this.pendingCommands[cmdId]) {
      this.pendingCommands[cmdId](response);
      delete this.pendingCommands[cmdId];
    }
  }

  private processRoomEvent(response: any, raw: any) {
    this.processEvent(response, RoomEvents, raw);
  }

  private processSessionEvent(response: any, raw: any) {
    this.processEvent(response, SessionEvents, raw);
  }

  private processGameEvent(container: any, raw: any): void {
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

      for (const key of Object.keys(GameEvents)) {
        const payload = event[key];
        if (payload !== undefined && payload !== null) {
          (GameEvents[key] as Function)(payload, meta);
          break;
        }
      }
    }
  }

  private processEvent(response: any, events: ProtobufEvents, raw: any) {
    for (const event in events) {
      const payload = response[event];

      if (payload !== undefined && payload !== null) {
        events[event](payload, raw);
        return;
      }
    }
  }
}
