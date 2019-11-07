import protobuf from 'protobufjs';
import $ from 'jquery';

import { StatusEnum } from 'common/types';
import { ServerService } from 'common/services/data';
  
import * as sessionEvents from './sessionEvents';
import * as roomEvents from './roomEvents';

class WebClient {
  private static instance: WebClient;

  public static getInstance() {
    if (this.instance) {
      return this.instance;
    }

    this.instance = new WebClient();
    return this.instance;
  }

  public pb;

  private roomEventKeys;
  private sessionEventKeys;

  constructor() {
    this.pb = new protobuf.Root();
    this.pb.load(WebClient.PB_FILES, { keepCase: false }, (err, root) => {
      if (err) {
        throw err;
      }
    });

    this.roomEventKeys = Object.keys(roomEvents);
    this.sessionEventKeys = Object.keys(sessionEvents);
  }

  public protocolVersion = 14;
  public options: any = {
    host: '',
    port: '',
    user: '',
    pass: '',
    debug: false,
    autojoinrooms: false,
    keepalive: 5000
  };
  private status = StatusEnum.DISCONNECTED;
  private socket: WebSocket;
  private keepalivecb;
  private lastPingPending = false;
  private cmdId = 0;
  private pendingCommands = {};

  public resetConnectionvars() {
    this.cmdId = 0;
    this.pendingCommands = {};    
  }

  public sendCommand(cmd, callback) {
    this.cmdId++;
    cmd["cmdId"] = this.cmdId;

    this.pendingCommands[this.cmdId] = callback;

    if (this.socket.readyState === WebSocket.OPEN) {
        this.socket.send(this.pb.CommandContainer.encode(cmd).finish());
        this.debug(() => console.log("Sent: " + cmd.toString()));
    } else {
      this.debug(() => console.log("Send: Not connected"));
    }
  }

  public sendRoomCommand(roomId, roomCmd, callback) {
    var cmd = this.pb.CommandContainer.create({
      "roomId" : roomId,
      "roomCommand" : [ roomCmd ]
    });

    this.sendCommand(cmd, callback);
  }

  public sendSessionCommand(ses, callback) {
    var cmd = this.pb.CommandContainer.create({
      "sessionCommand" : [ ses ]
    });

    this.sendCommand(cmd, callback);
  }

  public startPingLoop() {
    this.keepalivecb = setInterval(() => {
      // check if the previous ping got no reply
      if (this.lastPingPending) {
        this.disconnect();

        ServerService.updateStatus(StatusEnum.DISCONNECTED, 'Connection timeout');
      }

      // stop the ping loop if we're disconnected
      if (ServerService.getStatus().state !== StatusEnum.LOGGEDIN) {
        clearInterval(this.keepalivecb);
        this.keepalivecb = null;
        return;
      }

      // send a ping
      this.lastPingPending = true;

      var ping = this.pb.Command_Ping.create();
      var command = this.pb.SessionCommand.create({
        ".Command_Ping.ext" : ping
      });

      this.sendSessionCommand(command, () => this.lastPingPending = false);
    }, this.options.keepalive);
  }

  public connect(options, { onopen, onclose, onerror }) {
    $.extend(this.options, options || {});

    this.socket = new WebSocket('ws://' + this.options.host + ':' + this.options.port);
    this.socket.binaryType = "arraybuffer"; // We are talking binary

    this.socket.onopen = onopen;
    this.socket.onclose = onclose;
    this.socket.onerror = onerror;

    this.socket.onmessage = (event) => {
      const msg = this.decodeServerMessage(event);

      if (msg) {
        switch (msg.messageType) {
          case this.pb.ServerMessage.MessageType.RESPONSE:
            this.processServerResponse(msg.response);
            break;
          case this.pb.ServerMessage.MessageType.ROOM_EVENT:
            this.processRoomEvent(msg.roomEvent);
            break;
          case this.pb.ServerMessage.MessageType.SESSION_EVENT:
            this.processSessionEvent(msg.sessionEvent);
            break;
          case this.pb.ServerMessage.MessageType.GAME_EVENT_CONTAINER:
            // TODO
            break;
        }
      }
    } 
  }

  public disconnect() {
    if (this.socket) {
      this.socket.close();
    }
  }

  public debug(debug) {
    if (this.options.debug) {
      debug();
    }
  }

  private decodeServerMessage(event) {
    const uint8msg = new Uint8Array(event.data);
    let msg;

    try {
      msg = this.pb.ServerMessage.decode(uint8msg);

      this.debug(() => console.log(msg));

      return msg;
    } catch (err) {
      console.log("Processing failed:", err);

      this.debug(() => {
        var str = "";

        for (var i = 0; i < uint8msg.length; i++) {
          str += String.fromCharCode(uint8msg[i]);
        }

        console.log(str);          
      });

      return;
    }
  }

  private processServerResponse(response) {
    var cmdId = response.cmdId;

    if (!this.pendingCommands.hasOwnProperty(cmdId)) {
      return;
    }

    this.pendingCommands[cmdId](response);
    delete this.pendingCommands[cmdId];
  }

  private processRoomEvent(raw) {
    this.processEvent(raw, roomEvents, this.roomEventKeys);
  }

  private processSessionEvent(raw) {
    this.processEvent(raw, sessionEvents, this.sessionEventKeys);
  }

  private processEvent(raw, events, keys) {
    for (let i = 0; i < keys.length; i++) {
      const key = keys[i];
      const event = events[key];
      const payload = raw[event.id];

      if (payload) {
        events[key].action(payload);
        return;
      }
    }
  }

  static PB_FILE_DIR = `${process.env.PUBLIC_URL}/pb`;

  static PB_FILES = [
    // commands
    `${WebClient.PB_FILE_DIR}/commands.proto`,
    `${WebClient.PB_FILE_DIR}/session_commands.proto`,
    `${WebClient.PB_FILE_DIR}/room_commands.proto`,
    // replies
    `${WebClient.PB_FILE_DIR}/server_message.proto`,
    `${WebClient.PB_FILE_DIR}/response.proto`,
    `${WebClient.PB_FILE_DIR}/response_login.proto`,
    `${WebClient.PB_FILE_DIR}/session_event.proto`,
    `${WebClient.PB_FILE_DIR}/event_server_message.proto`,
    `${WebClient.PB_FILE_DIR}/event_server_identification.proto`,
    `${WebClient.PB_FILE_DIR}/event_server_shutdown.proto`,
    `${WebClient.PB_FILE_DIR}/event_notify_user.proto`,
    `${WebClient.PB_FILE_DIR}/event_connection_closed.proto`,
    `${WebClient.PB_FILE_DIR}/event_list_rooms.proto`,
    `${WebClient.PB_FILE_DIR}/response_join_room.proto`,
    `${WebClient.PB_FILE_DIR}/room_event.proto`,
    `${WebClient.PB_FILE_DIR}/event_room_say.proto`,
    `${WebClient.PB_FILE_DIR}/event_join_room.proto`,
    `${WebClient.PB_FILE_DIR}/event_leave_room.proto`,
    `${WebClient.PB_FILE_DIR}/event_list_games.proto`,
    `${WebClient.PB_FILE_DIR}/serverinfo_user.proto`,
    `${WebClient.PB_FILE_DIR}/serverinfo_game.proto`
  ];
}

export default WebClient;