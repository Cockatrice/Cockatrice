import {
  CommandContainer,
  Command_Ping,
  IResponse,
  IRoomEvent,
  ISessionEvent,
  ModeratorCommand,
  RoomCommand,
  ServerMessage,
  SessionCommand,
} from 'protoFiles';
import * as pf from 'protoFiles';
import { SessionPersistence } from '../persistence';
import { WebClient } from '../WebClient';

type Checked<T> = Exclude<T, null | undefined>
export type Handlers = Partial<{
  [event in keyof IResponse]: (arg0: Checked<IResponse[event]>) => void;
} & {
  [event in keyof ISessionEvent]: (arg0: Checked<ISessionEvent[event]>) => void;
} & {
  [event in keyof IRoomEvent]: (arg0: Checked<IRoomEvent[event]>) => void;
}>;

export class ProtobufService {
  static PB_FILE_DIR = `${process.env.PUBLIC_URL}/pb`;
  private handlers: Handlers;

  private cmdId = 0;
  private pendingCommands: { [cmdId: number]: Function } = {};

  private webClient: WebClient;

  constructor(webClient: WebClient) {
    this.webClient = webClient;
    this.handlers = {};
    SessionPersistence.initialized();
  }

  public AddHandlers(
    handlers: Handlers
  ) {
    this.handlers = { ...this.handlers, ...handlers };
  }

  public resetCommands() {
    this.cmdId = 0;
    this.pendingCommands = {};
  }

  public sendRoomCommand(
    roomId: number,
    roomCmd: RoomCommand,
    callback?: Function
  ) {
    const cmd = CommandContainer.create({
      roomId: roomId,
      roomCommand: [roomCmd],
    });

    this.sendCommand(cmd, callback);
  }

  public sendSessionCommand(sesCmd: SessionCommand, callback?: Function) {
    const cmd = CommandContainer.create({
      sessionCommand: [sesCmd],
    });

    this.sendCommand(cmd, callback);
  }

  public sendModeratorCommand(modCmd: ModeratorCommand, callback?: Function) {
    const cmd = CommandContainer.create({
      moderatorCommand: [modCmd],
    });

    this.sendCommand(cmd, callback);
  }

  public sendCommand(cmd: CommandContainer, callback?: Function) {
    this.cmdId++;

    cmd['cmdId'] = this.cmdId;
    if (callback) {
      this.pendingCommands[this.cmdId] = callback;
    }

    if (this.webClient.socket.checkReadyState(WebSocket.OPEN)) {
      this.webClient.socket.send(CommandContainer.encode(cmd).finish());
    }
  }

  public sendKeepAliveCommand(pingReceived: Function) {
    const command = SessionCommand.create({
      '.Command_Ping.ext': Command_Ping.create(),
    });

    this.sendSessionCommand(command, pingReceived);
  }

  public handleMessageEvent({ data }: MessageEvent): void {
    try {
      const uint8msg = new Uint8Array(data);
      const msg = ServerMessage.decode(uint8msg);

      switch (msg.messageType) {
        case ServerMessage.MessageType.RESPONSE:
          this.processServerResponse(msg.response!);
          break;
        case ServerMessage.MessageType.ROOM_EVENT:
          this.processEvent(msg.roomEvent!, msg);
          break;
        case ServerMessage.MessageType.SESSION_EVENT:
          this.processEvent(msg.sessionEvent!, msg);
          break;
        case ServerMessage.MessageType.GAME_EVENT_CONTAINER:
          // @TODO
          break;
      }
    } catch (err) {
      console.error('Processing failed:', err);
    }
  }

  private processServerResponse(response: IResponse) {
    const rawCmdId = response.cmdId;
    const cmdId = Number(rawCmdId);

    if (this.pendingCommands[cmdId]) {
      this.pendingCommands[cmdId](response);
      delete this.pendingCommands[cmdId];
    }
  }

  private processEvent(event: IRoomEvent | ISessionEvent, raw: ServerMessage) {
    for (const name in event) {
      // @ts-ignore
      this.handlers[name]?.(event[name]);
    }
  }
}
