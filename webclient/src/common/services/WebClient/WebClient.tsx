import protobuf from 'protobufjs';
import $ from 'jquery';

class WebClient {
  private static instance: WebClient;

  public static getInstance() {
    if (this.instance) {
      return this.instance;
    }

    this.instance = new WebClient();
    return this.instance;
  }

  constructor() {
    this.pb = new protobuf.Root();
    this.pb.load(WebClient.PB_FILES, { keepCase: false }, (err, root) => {
      if (err) {
        throw err;
      }
    });
  }

  private status = WebClient.StatusEnum.DISCONNECTED;
  private protocolVersion = 14;
  private socket: WebSocket;
  private keepalivecb;
  private lastPingPending = false;
  private cmdId = 0;
  private pendingCommands = {};
  private options: any = {
    host: '',
    port: '',
    user: '',
    pass: '',
    debug: false,
    autojoinrooms: false,
    keepalive: 5000
  };

  public pb;

  private guid() {
    return s4() + s4() + '-' + s4() + '-' + s4() + '-' + s4() + '-' + s4() + s4() + s4();

    function s4() {
      return Math.floor((1 + Math.random()) * 0x10000)
        .toString(16)
        .substring(1);
    }
  }

  private setStatus(status, desc) {
    this.status = status;

    if (this.options.debug) {
      console.log("Stats change:", status, desc)
    }

    if (this.options.statusCallback) {
      this.options.statusCallback(status, desc);
    }
  }

  private resetConnectionvars() {
    this.cmdId = 0;
    this.pendingCommands = {};    
  }

  private sendCommand(cmd, callback) {
    this.cmdId++;
    cmd["cmdId"] = this.cmdId;

    this.pendingCommands[this.cmdId] = callback;

    if (this.socket.readyState === WebSocket.OPEN) {
        this.socket.send(this.pb.CommandContainer.encode(cmd).finish());
        if (this.options.debug) {
          console.log("Sent: " + cmd.toString());
        }
    } else {
      if (this.options.debug) {
        console.log("Send: Not connected");
      }
    }
  }

  private sendRoomCommand(roomId, roomCmd, callback) {
    var cmd = this.pb.CommandContainer.create({
      "roomId" : roomId,
      "roomCommand" : [ roomCmd ]
    });

    this.sendCommand(cmd, callback);
  }

  private sendSessionCommand(ses, callback) {
    var cmd = this.pb.CommandContainer.create({
      "sessionCommand" : [ ses ]
    });

    this.sendCommand(cmd, callback);
  }

  private startPingLoop() {
    this.keepalivecb = setInterval(() => {
      // check if the previous ping got no reply
      if (this.lastPingPending) {
        this.socket.close();
        this.setStatus(WebClient.StatusEnum.DISCONNECTED, 'Connection timeout');
      }

      // stop the ping loop if we're disconnected
      if (this.status !== WebClient.StatusEnum.LOGGEDIN) {
        clearInterval(this.keepalivecb);
        this.keepalivecb = null;
        return;
      }

      // send a ping
      var CmdPing = this.pb.Command_Ping.create();

      var sc = this.pb.SessionCommand.create({
        ".Command_Ping.ext" : CmdPing
      });

      this.lastPingPending = true;
      this.sendSessionCommand(sc, () => {
        this.lastPingPending = false;
      });
    }, this.options.keepalive);
  }

  private doLogin() {
    var CmdLogin = this.pb.Command_Login.create({
      "userName" : this.options.user,
      "password" : this.options.pass,
      "clientid" : this.guid(),
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
    });

    var sc = this.pb.SessionCommand.create({
      ".Command_Login.ext" : CmdLogin
    });

    this.sendSessionCommand(sc, (raw) => {
      var resp = raw[".Response_Login.ext"];

      switch(raw.responseCode) {
        case this.pb.Response.ResponseCode.RespOk:
          this.setStatus(WebClient.StatusEnum.LOGGEDIN, 'Logged in.');

          if (this.options.userInfoCallback) {
            this.options.userInfoCallback(resp);
          }

          this.startPingLoop();
          this.doListRooms();
          break;
        case this.pb.Response.ResponseCode.RespClientUpdateRequired:
          this.setStatus(WebClient.StatusEnum.DISCONNECTING, 'Login failed: missing features');
          break;
        case this.pb.Response.ResponseCode.RespWrongPassword:
          this.setStatus(WebClient.StatusEnum.DISCONNECTING, 'Login failed: incorrect username or password');
          break;
        case this.pb.Response.ResponseCode.RespWouldOverwriteOldSession:
          this.setStatus(WebClient.StatusEnum.DISCONNECTING, 'Login failed: duplicated user session');
          break;
        case this.pb.Response.ResponseCode.RespUserIsBanned:
          this.setStatus(WebClient.StatusEnum.DISCONNECTING, 'Login failed: banned user');
          break;
        case this.pb.Response.ResponseCode.RespUsernameInvalid:
          this.setStatus(WebClient.StatusEnum.DISCONNECTING, 'Login failed: invalid username');
          break;
        case this.pb.Response.ResponseCode.RespRegistrationRequired:
          this.setStatus(WebClient.StatusEnum.DISCONNECTING, 'Login failed: registration required');
          break;
        case this.pb.Response.ResponseCode.RespClientIdRequired:
          this.setStatus(WebClient.StatusEnum.DISCONNECTING, 'Login failed: missing client ID');
          break;
        case this.pb.Response.ResponseCode.RespContextError:
          this.setStatus(WebClient.StatusEnum.DISCONNECTING, 'Login failed: server error');
          break;
        case this.pb.Response.ResponseCode.RespAccountNotActivated:
          this.setStatus(WebClient.StatusEnum.DISCONNECTING, 'Login failed: account not activated');
          break;
        default:
          this.setStatus(WebClient.StatusEnum.DISCONNECTING, 'Login failed: unknown error ' + raw.responseCode);
          break;
      }
    });
  }

  private doListRooms() {
    var CmdListRooms = this.pb.Command_ListRooms.create();

    var sc = this.pb.SessionCommand.create({
      ".Command_ListRooms.ext" : CmdListRooms
    });

    this.sendSessionCommand(sc, (raw) => {
      // Command_ListRooms 's response will be received inside a sessionEvent
    });
  }

  private processSessionEvent(raw) {
    if (raw[".Event_ConnectionClosed.ext"]) {
      var message = '';

      switch(raw[".Event_ConnectionClosed.ext"]["reason"]) {
        case this.pb.Event_ConnectionClosed.CloseReason.USER_LIMIT_REACHED:
          message = 'The server has reached its maximum user capacity';
          break;
        case this.pb.Event_ConnectionClosed.CloseReason.TOO_MANY_CONNECTIONS:
          message = 'There are too many concurrent connections from your address';
          break;
        case this.pb.Event_ConnectionClosed.CloseReason.BANNED:
          message = 'You are banned';
          break;
        case this.pb.Event_ConnectionClosed.CloseReason.SERVER_SHUTDOWN:
          message = 'Scheduled server shutdown';
          break;
        case this.pb.Event_ConnectionClosed.CloseReason.USERNAMEINVALID:
          message = 'Invalid username';
          break;
        case this.pb.Event_ConnectionClosed.CloseReason.LOGGEDINELSEWERE:
          message = 'You have been logged out due to logging in at another location';
          break;
        case this.pb.Event_ConnectionClosed.CloseReason.OTHER:
        default:
          message = 'Unknown reason';
          break;
      }

      if (this.options.connectionClosedCallback) {
        this.options.connectionClosedCallback(raw[".Event_ConnectionClosed.ext"]["reason"], message);
      }

      return;
    }

    if (raw[".Event_ServerMessage.ext"]) {
      if (this.options.serverMessageCallback) {
        this.options.serverMessageCallback(raw[".Event_ServerMessage.ext"]["message"]);
      }

      return;
    }

    if (raw[".Event_ListRooms.ext"]) {
      var roomsList = raw[".Event_ListRooms.ext"]["roomList"];
      
      if (this.options.listRoomsCallback) {
        this.options.listRoomsCallback(roomsList);
      }

      if (this.options.autojoinrooms) {
        $.each(roomsList, (index, room) => {
          if (room.autoJoin) {
            var CmdJoinRoom = this.pb.Command_JoinRoom.create({
              "roomId" : room.roomId
            });

            var sc = this.pb.SessionCommand.create({
              ".Command_JoinRoom.ext" : CmdJoinRoom
            });

            this.sendSessionCommand(sc, this.processJoinRoom);
          }
        });
      }

      return;
    }

    if (raw[".Event_ServerIdentification.ext"]) {
      if (this.options.serverIdentificationCallback) {
        this.options.serverIdentificationCallback(
          raw[".Event_ServerIdentification.ext"]
        );
      }

      if (raw[".Event_ServerIdentification.ext"].protocolVersion !== this.protocolVersion) {
        const protocolVersion = raw[".Event_ServerIdentification.ext"].protocolVersion;
        
        this.socket.close();
        this.setStatus(WebClient.StatusEnum.DISCONNECTED, 'Protocol version mismatch: ' + protocolVersion);
        return;
      }

      this.setStatus(WebClient.StatusEnum.CONNECTED, 'Logging in...');
      this.resetConnectionvars();
      this.doLogin();
      return;
    }

    if (raw[".Event_ServerShutdown.ext"]) {
      if (this.options.serverShutdownCallback) {
        this.options.serverShutdownCallback(raw[".Event_ServerShutdown.ext"]);
      }

      return;
    }

    if (raw[".Event_NotifyUser.ext"]) {
      if (this.options.notifyUserCallback) {
        this.options.notifyUserCallback(raw[".Event_NotifyUser.ext"]);
      }

      return;
    }
  }

  private processRoomEvent(raw) {
    if (raw[".Event_ListGames.ext"]) {
      if (this.options.roomListGamesCallback) {
        this.options.roomListGamesCallback(raw["roomId"], raw[".Event_ListGames.ext"]);
      }

      return;
    }

    if (raw[".Event_JoinRoom.ext"]) {
      if (this.options.roomJoinCallback) {
        this.options.roomJoinCallback(raw["roomId"], raw[".Event_JoinRoom.ext"]);
      }

      return;
    }

    if (raw[".Event_LeaveRoom.ext"]) {
      if (this.options.roomLeaveCallback) {
        this.options.roomLeaveCallback(raw["roomId"], raw[".Event_LeaveRoom.ext"]);
      }

      return;
    }

    if (raw[".Event_RoomSay.ext"]) {
      if (this.options.roomMessageCallback) {
        this.options.roomMessageCallback(raw["roomId"], raw[".Event_RoomSay.ext"]);
      }

      return;
    }
  }

  private processJoinRoom(raw) {
    let errorMsg;

    switch (raw["responseCode"]) {
      case this.pb.Response.ResponseCode.RespOk:
        if (this.options.joinRoomCallback) {
          var roomInfo = raw[".Response_JoinRoom.ext"]["roomInfo"];
          this.options.joinRoomCallback(roomInfo);
        }

        break;
      case this.pb.Response.ResponseCode.RespNameNotFound:
        errorMsg = "Failed to join the room: it doesn't exists on the server.";
        // eslint-disable-next-line
      case this.pb.Response.ResponseCode.RespContextError:
        errorMsg = "The server thinks you are in the room but Cockatrice is unable to display it. Try restarting Cockatrice.";
        // eslint-disable-next-line
      case this.pb.Response.ResponseCode.RespUserLevelTooLow:
        errorMsg = "You do not have the required permission to join this room.";
        // eslint-disable-next-line
      default:
        if (this.options.errorCallback) {
          if (!errorMsg) {
            errorMsg = "Failed to join the room due to an unknown error: " + raw["responseCode"];
          }

          this.options.errorCallback(raw["responseCode"], errorMsg);
        }

        return;
    }
  }

  public connect(options) {
    $.extend(this.options, options || {});

    this.socket = new WebSocket('ws://' + this.options.host + ':' + this.options.port);
    this.socket.binaryType = "arraybuffer"; // We are talking binary
    this.setStatus(WebClient.StatusEnum.CONNECTING, 'Connecting...');

    this.socket.onclose = () => {
      this.setStatus(WebClient.StatusEnum.DISCONNECTED, 'Connection closed');
    } 

    this.socket.onerror = () => {
      this.setStatus(WebClient.StatusEnum.DISCONNECTED, 'Connection failed');
    }

    this.socket.onopen = () => {
      this.setStatus(WebClient.StatusEnum.CONNECTED, 'Connected');
    } 

    this.socket.onmessage = (event) => {
      //console.log("Received " + event.data.byteLength + " bytes");

      var uint8msg = new Uint8Array(event.data);

      try {
        var msg = this.pb.ServerMessage.decode(uint8msg);

        if (this.options.debug) {
          console.log(msg);
        }
      } catch (err) {
        console.log("Processing failed:", err);
        if (this.options.debug) {
          var str = "";
          for (var i = 0; i < uint8msg.length; i++) {
            str += String.fromCharCode(uint8msg[i]);
          }
          console.log(str);          
        }
        return;
      }

      switch (msg.messageType) {
        case this.pb.ServerMessage.MessageType.RESPONSE:
          var response = msg.response;
          var cmdId = response.cmdId;

          if (!this.pendingCommands.hasOwnProperty(cmdId)) {
            return;
          }

          this.pendingCommands[cmdId](response);
          delete this.pendingCommands[cmdId];
          break;
        case this.pb.ServerMessage.MessageType.SESSION_EVENT:
          this.processSessionEvent(msg.sessionEvent);
          break;
        case this.pb.ServerMessage.MessageType.GAME_EVENT_CONTAINER:
          // TODO
          break;
        case this.pb.ServerMessage.MessageType.ROOM_EVENT:
          this.processRoomEvent(msg.roomEvent);
          break;
      }
    } 
  }

  public disconnect() {
    this.socket.close();
  }

  public roomSay(roomId, msg) {
    var CmdRoomSay = this.pb.Command_RoomSay.create({
      "message" : msg
    });

    var sc = this.pb.RoomCommand.create({
      ".Command_RoomSay.ext" : CmdRoomSay
    });

    this.sendRoomCommand(roomId, sc, (raw) => {
      switch (raw["responseCode"]) {
        case this.pb.Response.ResponseCode.RespChatFlood:
          console.log("room flood " + roomId);
          break;
        default:
          break;
      }
    });
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

  static StatusEnum = {
    DISCONNECTED  :  0,
    CONNECTING    :  1,
    CONNECTED     :  2,
    LOGGINGIN     :  3,
    LOGGEDIN      :  4,
    DISCONNECTING :  99
  };

  private static StatusEnumLabel = {
    0: 'Disconnected',
    1: 'Connecting',
    2: 'Connected',
    3: 'Loggingin',
    4: 'Loggedin',
    99: 'Disconnecting'
  };

  public static getStatusEnumLabel(statusEnum: number) {
    if (WebClient.StatusEnumLabel[statusEnum] !== undefined) {
      return WebClient.StatusEnumLabel[statusEnum];
    }

    return 'Unknown';
  }
}


export default WebClient;