import protobuf from "protobufjs";

import { StatusEnum } from "types";

import * as roomEvents from "./events/RoomEvents";
import * as sessionEvents from "./events/SessionEvents";
  
import { RoomService, SessionService } from "./services";
import { RoomCommands, SessionCommands } from "./commands";

import ProtoFiles from "./ProtoFiles";

const roomEventKeys = Object.keys(roomEvents);
const sessionEventKeys = Object.keys(sessionEvents);

interface ApplicationCommands {
  room: RoomCommands;
  session: SessionCommands;
}

interface ApplicationServices {
  room: RoomService;
  session: SessionService;
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

  public clientConfig = {
    "clientver" : "webclient-1.0 (2019-10-31)",
    "clientfeatures" : [
      "client_id",
      "client_ver",
      "feature_set",
      "room_chat_history",
      "client_warnings",
      /* unimplemented features */
      "forgot_password",
      "idle_client",
      "mod_log_lookup",
      "user_ban_history",
      // satisfy server reqs for POC
      "websocket",
      "2.6.1_min_version",
      "2.7.0_min_version",
    ]
  };

  public options: any = {
    host: "",
    port: "",
    user: "",
    pass: "",
    debug: false,
    autojoinrooms: true,
    keepalive: 5000
  };

  constructor() {
    const files = ProtoFiles.map(file => `${WebClient.PB_FILE_DIR}/${file}`);
    
    this.pb = new protobuf.Root();
    this.pb.load(files, { keepCase: false }, (err, root) => {
      if (err) {
        throw err;
      }
    });

    // This sucks. I can"t seem to get out of this
    // circular dependency trap, so this is my current best.
    this.commands = {
      room: new RoomCommands(this),
      session: new SessionCommands(this),
    };

    this.services = {
      room: new RoomService(this),
      session: new SessionService(this),
    };

    console.log(this);
  }

  private clearStores() {
    this.services.room.clearStore();
    this.services.session.clearStore();
  }

  public updateStatus(status, description) {
    console.log(`Status: [${status}]: ${description}`);
    this.status = status;
    this.services.session.updateStatus(status, description);

    if (status === StatusEnum.DISCONNECTED) {
      this.clearStores();
      this.endPingLoop();
      this.resetConnectionvars();
    }
  }

  public resetConnectionvars() {
    this.cmdId = 0;
    this.pendingCommands = {};
    this.lastPingPending = false;    
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

  public sendSessionCommand(sesCmd, callback?) {
    const cmd = this.pb.CommandContainer.create({
      "sessionCommand" : [ sesCmd ]
    });

    this.sendCommand(cmd, (raw) => {
      this.debug(() => console.log(raw));
      
      if (callback) {
        callback(raw);
      }
    });
  }

  public sendModeratorCommand(modCmd, callback?) {
    const cmd = this.pb.CommandContainer.create({
      "moderatorCommand" : [ modCmd ]
    });

    this.sendCommand(cmd, (raw) => {
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

        this.updateStatus(StatusEnum.DISCONNECTED, "Connection timeout");
      }

      // stop the ping loop if we"re disconnected
      if (this.status !== StatusEnum.LOGGEDIN) {
        this.endPingLoop();
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

  private endPingLoop() {
    clearInterval(this.keepalivecb);
    this.keepalivecb = null;
  }

  public connect(options) {
    this.options = { ...this.options, ...options };

    const { host, port } = this.options;
    const protocol = port === '443' ? 'wss' : 'ws';

    this.socket = new WebSocket(protocol + "://" + host + ":" + port);
    this.socket.binaryType = "arraybuffer"; // We are talking binary

    this.socket.onopen = () => {
      this.updateStatus(StatusEnum.CONNECTED, "Connected");
    };

    this.socket.onclose = () => {
      // dont overwrite failure messages
      if (this.status !== StatusEnum.DISCONNECTED) {
        this.updateStatus(StatusEnum.DISCONNECTED, "Connection Closed");
      }
    };

    this.socket.onerror = () => {
      this.updateStatus(StatusEnum.DISCONNECTED, "Connection Failed");
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
            // @TODO
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
      console.error("Processing failed:", err);

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
}

const webClient = new WebClient();
export default webClient;
