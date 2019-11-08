import protobuf from 'protobufjs';
import $ from 'jquery';

import { StatusEnum } from 'types';

import * as roomEvents from './roomEvents';
import * as sessionEvents from './sessionEvents';
  
import { ServerService } from './services';
import { SessionCommands } from './commands';

const roomEventKeys = Object.keys(roomEvents);
const sessionEventKeys = Object.keys(sessionEvents);

interface ApplicationCommands {
  session: SessionCommands;
}

interface ApplicationServices {
  server: ServerService;
}

export class WebClient {
  private socket: WebSocket;
  private status: StatusEnum = StatusEnum.DISCONNECTED;
  private keepalivecb;
  private lastPingPending = false;
  private cmdId = 0;
  private pendingCommands = {};

  public commands: ApplicationCommands;
  public services: ApplicationServices;

  public protocolVersion = 14;
  public pb;

  public options: any = {
    host: '',
    port: '',
    user: '',
    pass: '',
    debug: false,
    autojoinrooms: true,
    keepalive: 5000
  };

  constructor() {
    this.pb = new protobuf.Root();
    this.pb.load(WebClient.PB_FILES, { keepCase: false }, (err, root) => {
      if (err) {
        throw err;
      }
    });

    this.commands = {
      session: new SessionCommands(this),
    };

    this.services = {
      server: new ServerService(this),
    };
  }

  public updateStatus(status, description) {
    this.status = status;
    this.services.server.updateStatus(status, description);
  }

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

  public sendRoomCommand(roomId, roomCmd, callback?) {
    const cmd = this.pb.CommandContainer.create({
      "roomId" : roomId,
      "roomCommand" : [ roomCmd ]
    });

    this.sendCommand(cmd, raw => {
      this.debug(() => console.log(raw));
      
      if (callback) {
        callback(raw);
      }
    });
  }

  public sendSessionCommand(ses, callback?) {
    const cmd = this.pb.CommandContainer.create({
      "sessionCommand" : [ ses ]
    });

    this.sendCommand(cmd, raw => {
      this.debug(() => console.log(raw));
      
      if (callback) {
        callback(raw);
      }
    });
  }

  public startPingLoop() {
    this.keepalivecb = setInterval(() => {
      // check if the previous ping got no reply
      if (this.lastPingPending) {
        this.disconnect();

        this.updateStatus(StatusEnum.DISCONNECTED, 'Connection timeout');
      }

      // stop the ping loop if we're disconnected
      if (this.status !== StatusEnum.LOGGEDIN) {
        clearInterval(this.keepalivecb);
        this.keepalivecb = null;
        return;
      }

      // send a ping
      this.lastPingPending = true;

      const ping = this.pb.Command_Ping.create();
      const command = this.pb.SessionCommand.create({
        ".Command_Ping.ext" : ping
      });

      this.sendSessionCommand(command, () => this.lastPingPending = false);
    }, this.options.keepalive);
  }

  public connect(options) {
    $.extend(this.options, options || {});

    this.socket = new WebSocket('ws://' + this.options.host + ':' + this.options.port);
    this.socket.binaryType = "arraybuffer"; // We are talking binary

    this.socket.onopen = () => {
      this.updateStatus(StatusEnum.CONNECTED, 'Connected');
    };

    this.socket.onclose = () => {
      // @TODO determine if these connectionClosed hooks are desired
      // this.services.server.connectionClosed('Connection Closed');
      this.updateStatus(StatusEnum.DISCONNECTED, 'Connection Closed');
    };

    this.socket.onerror = () => {
      // @TODO determine if these connectionClosed hooks are desired
      // this.services.server.connectionClosed('Connection Failed');
      this.updateStatus(StatusEnum.DISCONNECTED, 'Connection Failed');
    };


    this.socket.onmessage = (event) => {
      const msg = this.decodeServerMessage(event);

      if (msg) {
        switch (msg.messageType) {
          case this.pb.ServerMessage.MessageType.RESPONSE:
            this.processServerResponse(msg.response);
            break;
          case this.pb.ServerMessage.MessageType.ROOM_EVENT:
            this.processRoomEvent(msg.roomEvent, msg);
            break;
          case this.pb.ServerMessage.MessageType.SESSION_EVENT:
            this.processSessionEvent(msg.sessionEvent, msg);
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
        let str = "";

        for (let i = 0; i < uint8msg.length; i++) {
          str += String.fromCharCode(uint8msg[i]);
        }

        console.log(str);          
      });

      return;
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
    this.processEvent(response, roomEvents, roomEventKeys, raw);
  }

  private processSessionEvent(response, raw) {
    this.processEvent(response, sessionEvents, sessionEventKeys, raw);
  }

  private processEvent(response, events, keys, raw) {
    for (let i = 0; i < keys.length; i++) {
      const key = keys[i];
      const event = events[key];
      const payload = response[event.id];

      if (payload) {
        events[key].action(payload, this, raw);
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

const webClient = new WebClient();
export default webClient;
