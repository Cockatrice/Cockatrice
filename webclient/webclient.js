var StatusEnum = {
    DISCONNECTED  :  0,
    CONNECTING    :  1,
    CONNECTED     :  2,
    LOGGINGIN     :  3,
    LOGGEDIN      :  4,
    DISCONNECTING :  99
};

var WebClient = {
  status : StatusEnum.DISCONNECTED,
  protocolVersion : 14,
  socket : 0,
  keepalivecb: null,
  lastPingPending: false,
  cmdId : 0,
  initialized: false,
  pendingCommands : {},
  options : {
    host: "",
    port: "",
    user: "",
    pass: "",
    debug: false,
    autojoinrooms: false,
    keepalive: 5000
  },

  pb : null,
  pbfiles : [
    // commands
    "pb/commands.proto",
    "pb/session_commands.proto",
    "pb/room_commands.proto",
    // replies
    "pb/server_message.proto",
    "pb/response.proto",
    "pb/response_login.proto",
    "pb/session_event.proto",
    "pb/event_server_message.proto",
    "pb/event_server_identification.proto",
    "pb/event_server_shutdown.proto",
    "pb/event_notify_user.proto",
    "pb/event_connection_closed.proto",
    "pb/event_list_rooms.proto",
    "pb/response_join_room.proto",
    "pb/room_event.proto",
    "pb/event_room_say.proto",
    "pb/event_join_room.proto",
    "pb/event_leave_room.proto",
    "pb/event_list_games.proto",
    "pb/serverinfo_user.proto",
    "pb/serverinfo_game.proto"
  ],

  initialize : function()
  {
    this.pb = new protobuf.Root();

    this.pb.load(this.pbfiles, { keepCase: false }, function(err, root) {
    if (err)
        throw err;
    });

    this.initialized=true;
  },

  guid : function(options)
  {
    function s4() {
      return Math.floor((1 + Math.random()) * 0x10000)
        .toString(16)
        .substring(1);
    }
    return s4() + s4() + '-' + s4() + '-' + s4() + '-' + s4() + '-' + s4() + s4() + s4();
  },

  setStatus : function(status, desc)
  {
    this.status = status;
    if(this.options.debug)
      console.log("Stats change:", status, desc)
    if(this.options.statusCallback)
      this.options.statusCallback(status, desc);
  },

  resetConnectionvars : function () {
    this.cmdId = 0;
    this.pendingCommands = {};
  },

  sendCommand : function (cmd, callback)
  {
    this.cmdId++;
    cmd["cmdId"] = this.cmdId;
    this.pendingCommands[this.cmdId] = callback;

    if (this.socket.readyState == WebSocket.OPEN) {
        this.socket.send(WebClient.pb.CommandContainer.encode(cmd).finish());
        if(this.options.debug)
          console.log("Sent: " + cmd.toString());
    } else {
      if(this.options.debug)
        console.log("Send: Not connected");
    }
  },

  sendRoomCommand : function(roomId, roomCmd, callback)
  {
    var cmd = WebClient.pb.CommandContainer.create({
      "roomId" : roomId,
      "roomCommand" : [ roomCmd ]
    });
    WebClient.sendCommand(cmd, callback);
  },

  sendSessionCommand : function(ses, callback)
  {
    var cmd = WebClient.pb.CommandContainer.create({
      "sessionCommand" : [ ses ]
    });
    WebClient.sendCommand(cmd, callback);
  },

  startPingLoop : function()
  {
    keepalivecb = setInterval(function() {
      // check if the previous ping got no reply
      if(WebClient.lastPingPending)
      {
        WebClient.socket.close();
        WebClient.setStatus(StatusEnum.DISCONNECTED, 'Connection timeout');
      }

      // stop the ping loop if we're disconnected
      if(WebClient.status != StatusEnum.LOGGEDIN)
      {
        clearInterval(keepalivecb);
        keepalivecb = null;
        return;
      }

      // send a ping
      var CmdPing = WebClient.pb.Command_Ping.create();

      var sc = WebClient.pb.SessionCommand.create({
      ".Command_Ping.ext" : CmdPing
      });

      WebClient.lastPingPending = true;
      WebClient.sendSessionCommand(sc, function() {
        WebClient.lastPingPending = false;
      });

    }, WebClient.options.keepalive);
  },

  doLogin : function()
  {
    var CmdLogin = WebClient.pb.Command_Login.create({
      "userName" : this.options.user,
      "password" : this.options.pass,
      "clientid" : this.guid(),
      "clientver" : "webclient-0.3 (2017-05-26)",
      "clientfeatures" : [
        "client_id",
        "client_ver",
        "feature_set",
        "room_chat_history",
        "client_warnings",
        "websocket",
        "2.6.1_min_version",
        "2.7.0_min_version",
        /* unimplemented features */
        "forgot_password",
        "idle_client",
        "mod_log_lookup",
        "user_ban_history"
      ]
    });

    var sc = WebClient.pb.SessionCommand.create({
      ".Command_Login.ext" : CmdLogin
    });

    this.sendSessionCommand(sc, function(raw) {
      var resp = raw[".Response_Login.ext"];
      switch(raw.responseCode)
      {
          case WebClient.pb.Response.ResponseCode.RespOk:
            WebClient.setStatus(StatusEnum.LOGGEDIN, 'Logged in.');

            if(WebClient.options.userInfoCallback)
              WebClient.options.userInfoCallback(resp);

            WebClient.startPingLoop();
            WebClient.doListRooms();
            break;
          case WebClient.pb.Response.ResponseCode.RespClientUpdateRequired:
            WebClient.setStatus(StatusEnum.DISCONNECTING, 'Login failed: missing features');
            break;
          case WebClient.pb.Response.ResponseCode.RespWrongPassword:
            WebClient.setStatus(StatusEnum.DISCONNECTING, 'Login failed: incorrect username or password');
            break;
          case WebClient.pb.Response.ResponseCode.RespWouldOverwriteOldSession:
            WebClient.setStatus(StatusEnum.DISCONNECTING, 'Login failed: duplicated user session');
            break;
          case WebClient.pb.Response.ResponseCode.RespUserIsBanned:
            WebClient.setStatus(StatusEnum.DISCONNECTING, 'Login failed: banned user');
            break;
          case WebClient.pb.Response.ResponseCode.RespUsernameInvalid:
            WebClient.setStatus(StatusEnum.DISCONNECTING, 'Login failed: invalid username');
            break;
          case WebClient.pb.Response.ResponseCode.RespRegistrationRequired:
            WebClient.setStatus(StatusEnum.DISCONNECTING, 'Login failed: registration required');
            break;
          case WebClient.pb.Response.ResponseCode.RespClientIdRequired:
            WebClient.setStatus(StatusEnum.DISCONNECTING, 'Login failed: missing client ID');
            break;
          case WebClient.pb.Response.ResponseCode.RespContextError:
            WebClient.setStatus(StatusEnum.DISCONNECTING, 'Login failed: server error');
            break;
          case WebClient.pb.Response.ResponseCode.RespAccountNotActivated:
            WebClient.setStatus(StatusEnum.DISCONNECTING, 'Login failed: account not activated');
            break;
          case WebClient.pb.Response.ResponseCode.RespClientUpdateRequired:
            WebClient.setStatus(StatusEnum.DISCONNECTING, 'Login failed: missing features');
            break;
          default:
            WebClient.setStatus(StatusEnum.DISCONNECTING, 'Login failed: unknown error ' + raw.responseCode);
            break;
      }
    });
  },

  doListRooms : function()
  {
    var CmdListRooms = WebClient.pb.Command_ListRooms.create();

    var sc = WebClient.pb.SessionCommand.create({
      ".Command_ListRooms.ext" : CmdListRooms
    });

    this.sendSessionCommand(sc, function(raw) {
      // Command_ListRooms 's response will be received inside a sessionEvent
    });
  },

  processSessionEvent : function (raw)
  {
    if(raw[".Event_ConnectionClosed.ext"]) {
      var message = '';
      switch(raw[".Event_ConnectionClosed.ext"]["reason"])
      {
          case WebClient.pb.Event_ConnectionClosed.CloseReason.USER_LIMIT_REACHED:
            message = 'The server has reached its maximum user capacity';
            break;
          case WebClient.pb.Event_ConnectionClosed.CloseReason.TOO_MANY_CONNECTIONS:
            message = 'There are too many concurrent connections from your address';
            break;
          case WebClient.pb.Event_ConnectionClosed.CloseReason.BANNED:
            message = 'You are banned';
            break;
          case WebClient.pb.Event_ConnectionClosed.CloseReason.SERVER_SHUTDOWN:
            message = 'Scheduled server shutdown';
            break;
          case WebClient.pb.Event_ConnectionClosed.CloseReason.USERNAMEINVALID:
            message = 'Invalid username';
            break;
          case WebClient.pb.Event_ConnectionClosed.CloseReason.LOGGEDINELSEWERE:
            message = 'You have been logged out due to logging in at another location';
            break;
          case WebClient.pb.Event_ConnectionClosed.CloseReason.OTHER:
          default:
            message = 'Unknown reason';
            break;
      }

      if(this.options.connectionClosedCallback)
        this.options.connectionClosedCallback(raw[".Event_ConnectionClosed.ext"]["reason"], message);

      return;
    }

    if(raw[".Event_ServerMessage.ext"]) {
      if(this.options.serverMessageCallback)
        this.options.serverMessageCallback(raw[".Event_ServerMessage.ext"]["message"]);
      return;
    }

    if(raw[".Event_ListRooms.ext"]) {
      var roomsList = raw[".Event_ListRooms.ext"]["roomList"];
      if(this.options.listRoomsCallback)
        this.options.listRoomsCallback(roomsList);
      if(this.options.autojoinrooms)
      {
        $.each(roomsList, function(index, room) {
          if(room.autoJoin)
          {
            var CmdJoinRoom = WebClient.pb.Command_JoinRoom.create({
              "roomId" : room.roomId
            });

            var sc = WebClient.pb.SessionCommand.create({
              ".Command_JoinRoom.ext" : CmdJoinRoom
            });

            WebClient.sendSessionCommand(sc, WebClient.processJoinRoom);
          }
        });
      }
      return;
    }

    if(raw[".Event_ServerIdentification.ext"]) {
      if(this.options.serverIdentificationCallback)
        this.options.serverIdentificationCallback(
          raw[".Event_ServerIdentification.ext"]
        );

      if(raw[".Event_ServerIdentification.ext"].protocolVersion != WebClient.protocolVersion)
      {
        WebClient.socket.close();
        WebClient.setStatus(StatusEnum.DISCONNECTED, 'Protocol version mismatch: ' + raw[".Event_ServerIdentification.ext"].protocolVersion);
        return;
      }

      WebClient.setStatus(StatusEnum.CONNECTED, 'Logging in...');
      WebClient.resetConnectionvars();
      WebClient.doLogin();
      return;
    }

    if(raw[".Event_ServerShutdown.ext"]) {
      if(this.options.serverShutdownCallback)
        this.options.serverShutdownCallback(raw[".Event_ServerShutdown.ext"]);
      return;
    }

    if(raw[".Event_NotifyUser.ext"]) {
      if(this.options.notifyUserCallback)
        this.options.notifyUserCallback(raw[".Event_NotifyUser.ext"]);
      return;
    }
  },

  processRoomEvent : function (raw)
  {
    if(raw[".Event_ListGames.ext"]) {
      if(this.options.roomListGamesCallback)
        this.options.roomListGamesCallback(raw["roomId"], raw[".Event_ListGames.ext"]);
      return;
    }

    if(raw[".Event_JoinRoom.ext"]) {
      if(this.options.roomJoinCallback)
        this.options.roomJoinCallback(raw["roomId"], raw[".Event_JoinRoom.ext"]);
      return;
    }

    if(raw[".Event_LeaveRoom.ext"]) {
      if(this.options.roomLeaveCallback)
        this.options.roomLeaveCallback(raw["roomId"], raw[".Event_LeaveRoom.ext"]);
      return;
    }

    if(raw[".Event_RoomSay.ext"]) {
      if(this.options.roomMessageCallback)
        this.options.roomMessageCallback(raw["roomId"], raw[".Event_RoomSay.ext"]);
      return;
    }
  },

  processJoinRoom : function(raw)
  {
    switch(raw["responseCode"])
    {
      case WebClient.pb.Response.ResponseCode.RespOk:
        var roomInfo = raw[".Response_JoinRoom.ext"]["roomInfo"];
        if(WebClient.options.joinRoomCallback)
          WebClient.options.joinRoomCallback(roomInfo);
        break;
      case WebClient.pb.Response.ResponseCode.RespNameNotFound:
          if(WebClient.options.errorCallback) WebClient.options.errorCallback(raw["responseCode"], "Failed to join the room: it doesn't exists on the server.");
          return;
      case WebClient.pb.Response.ResponseCode.RespContextError:
          if(WebClient.options.errorCallback) WebClient.options.errorCallback(raw["responseCode"], "The server thinks you are in the room but Cockatrice is unable to display it. Try restarting Cockatrice.");
          return;
      case WebClient.pb.Response.ResponseCode.RespUserLevelTooLow:
          if(WebClient.options.errorCallback) WebClient.options.errorCallback(raw["responseCode"], "You do not have the required permission to join this room.");
          return;
      default:
          if(WebClient.options.errorCallback) WebClient.options.errorCallback(raw["responseCode"], "Failed to join the room due to an unknown error: " + raw["responseCode"]);
          return;
    }
  },

  connect : function(options) {
    jQuery.extend(this.options, options || {});

    if(!this.initialized)
      this.initialize();

    this.socket = new WebSocket('ws://' + this.options.host + ':' + this.options.port);
    this.socket.binaryType = "arraybuffer"; // We are talking binary
    this.setStatus(StatusEnum.CONNECTING, 'Connecting...');

    this.socket.onclose = function() {
      WebClient.setStatus(StatusEnum.DISCONNECTED, 'Connection closed');
    }

    this.socket.onerror = function() {
      WebClient.setStatus(StatusEnum.DISCONNECTED, 'Connection failed');
    }

    this.socket.onopen = function(){
      WebClient.setStatus(StatusEnum.CONNECTED, 'Connected');
    }

    this.socket.onmessage = function(event) {
      //console.log("Received " + event.data.byteLength + " bytes");

      var uint8msg = new Uint8Array(event.data);
      try {
        var msg = WebClient.pb.ServerMessage.decode(uint8msg);
        if(WebClient.options.debug)
          console.log(msg);
      } catch (err) {
        console.log("Processing failed:", err);
        if(WebClient.options.debug)
        {
          var str = "";
          for(var i = 0; i < uint8msg.length; i++)
          {
            str += String.fromCharCode(uint8msg[i]);
          }
          console.log(str);
        }
        return;
      }

      switch (msg.messageType) {
        case WebClient.pb.ServerMessage.MessageType.RESPONSE:
          var response = msg.response;
          var cmdId = response.cmdId;

          if(!WebClient.pendingCommands.hasOwnProperty(cmdId))
            return;
          WebClient.pendingCommands[cmdId](response);
          delete WebClient.pendingCommands[cmdId];
          break;
        case WebClient.pb.ServerMessage.MessageType.SESSION_EVENT:
          WebClient.processSessionEvent(msg.sessionEvent);
          break;
        case WebClient.pb.ServerMessage.MessageType.GAME_EVENT_CONTAINER:
          // TODO
          break;
        case WebClient.pb.ServerMessage.MessageType.ROOM_EVENT:
          WebClient.processRoomEvent(msg.roomEvent);
          break;
      }
    }
  },

  disconnect : function() {
    this.socket.close();
  },

  roomSay : function(roomId, msg) {
    var CmdRoomSay = WebClient.pb.Command_RoomSay.create({
      "message" : msg
    });

    var sc = WebClient.pb.RoomCommand.create({
      ".Command_RoomSay.ext" : CmdRoomSay
    });

    WebClient.sendRoomCommand(roomId, sc, function(raw) {
      switch(raw["responseCode"])
      {
        case WebClient.pb.Response.ResponseCode.RespChatFlood:
          console.log("room flood " + roomId);
          break;
        default:
          break;
      }
    });
  }
}
