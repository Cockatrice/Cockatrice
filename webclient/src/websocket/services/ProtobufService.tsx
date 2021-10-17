import protobuf from "protobufjs";

import ProtoFiles from "../ProtoFiles";
import { WebClient } from "../WebClient";

import { RoomEvents, SessionEvents } from '../events';

export class ProtobufService {
  static PB_FILE_DIR = `${process.env.PUBLIC_URL}/pb`;

  public controller;
  private cmdId = 0;
  private pendingCommands = {};

  private webClient: WebClient;

  constructor(webClient: WebClient) {
    this.webClient = webClient;

    this.loadProtobufFiles();
  }

  public resetCommands() {
    this.cmdId = 0;
    this.pendingCommands = {};
  }

  public sendRoomCommand(roomId, roomCmd, callback?) {
    const cmd = this.controller.CommandContainer.create({
      "roomId" : roomId,
      "roomCommand" : [ roomCmd ]
    });

    this.sendCommand(cmd, raw => callback && callback(raw));
  }

  public sendSessionCommand(sesCmd, callback?) {
    const cmd = this.controller.CommandContainer.create({
      "sessionCommand" : [ sesCmd ]
    });

    this.sendCommand(cmd, (raw) => callback && callback(raw));
  }

  public sendModeratorCommand(modCmd, callback?) {
    const cmd = this.controller.CommandContainer.create({
      "moderatorCommand" : [ modCmd ]
    });

    this.sendCommand(cmd, (raw) => callback && callback(raw));
  }

  public sendCommand(cmd, callback) {
    this.cmdId++;
    cmd["cmdId"] = this.cmdId;

    this.pendingCommands[this.cmdId] = callback;

    if (this.webClient.socket.checkReadyState(WebSocket.OPEN)) {
      this.webClient.socket.send(this.controller.CommandContainer.encode(cmd).finish());
    }
  }

  public handleMessageEvent({ data }: MessageEvent): void {
    try {
      const uint8msg = new Uint8Array(data);
      const msg = this.controller.ServerMessage.decode(uint8msg);

      if (msg) {
        switch (msg.messageType) {
          case this.controller.ServerMessage.MessageType.RESPONSE:
            this.processServerResponse(msg.response);
            break;
          case this.controller.ServerMessage.MessageType.ROOM_EVENT:
            this.processRoomEvent(msg.roomEvent, msg);
            break;
          case this.controller.ServerMessage.MessageType.SESSION_EVENT:
            this.processSessionEvent(msg.sessionEvent, msg);
            break;
          case this.controller.ServerMessage.MessageType.GAME_EVENT_CONTAINER:
            // @TODO
            break;
        }
      }
    } catch (err) {
      console.error("Processing failed:", err);
    }
  }

  private processServerResponse(response) {
    const cmdId = response.cmdId;

    if (!this.pendingCommands.hasOwnProperty(cmdId)) {
      return;
    }

    this.pendingCommands[cmdId](response);
    delete this.pendingCommands[cmdId];
  }

  private processRoomEvent(response, raw) {
    this.processEvent(response, RoomEvents, raw);
  }

  private processSessionEvent(response, raw) {
    this.processEvent(response, SessionEvents, raw);
  }

  private processEvent(response, events, raw) {
    for (const key in events) {
      const payload = response[key];

      if (payload) {
        events[key](payload, raw);
        return;
      }
    }
  }

  private loadProtobufFiles() {
    const files = ProtoFiles.map(file => `${ProtobufService.PB_FILE_DIR}/${file}`);
    
    this.controller = new protobuf.Root();
    this.controller.load(files, { keepCase: false }, (err, root) => {
      if (err) {
        throw err;
      }
    });
  }
}
