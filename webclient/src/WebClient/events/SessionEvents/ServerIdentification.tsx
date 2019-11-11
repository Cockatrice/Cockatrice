import { StatusEnum } from 'types';
import { guid } from '../../util';

const defaultLoginConfig = {
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
}

export const ServerIdentification = {
  id: '.Event_ServerIdentification.ext',
  action: (info, webClient, _raw) => {
    const { serverName, serverVersion, protocolVersion } = info;

    if (protocolVersion !== webClient.protocolVersion) {
      webClient.disconnect();
      webClient.updateStatus(StatusEnum.DISCONNECTED, 'Protocol version mismatch: ' + protocolVersion);
      return;
    }

    webClient.resetConnectionvars();
    webClient.updateStatus(StatusEnum.CONNECTED, 'Logging in...');
    webClient.services.session.updateInfo(serverName, serverVersion);

    const loginConfig = {
      ...defaultLoginConfig,
      "userName" : webClient.options.user,
      "password" : webClient.options.pass,
      "clientid" : guid()
    };

    const CmdLogin = webClient.pb.Command_Login.create(loginConfig);

    const command = webClient.pb.SessionCommand.create({
      ".Command_Login.ext" : CmdLogin
    });

    webClient.sendSessionCommand(command, handleCommandResponse);

    function handleCommandResponse(raw) {
      const resp = raw[".Response_Login.ext"];

      webClient.debug(() =>  console.log('.Response_Login.ext', resp));

      switch(raw.responseCode) {
        case webClient.pb.Response.ResponseCode.RespOk:
          const { userInfo } = resp;
          webClient.services.session.updateUser(userInfo);
          webClient.commands.session.listUsers();
          webClient.commands.session.listRooms();
          webClient.updateStatus(StatusEnum.LOGGEDIN, 'Logged in.');
          webClient.startPingLoop();
          break;

        case webClient.pb.Response.ResponseCode.RespClientUpdateRequired:
          webClient.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: missing features');
          break;

        case webClient.pb.Response.ResponseCode.RespWrongPassword:
        case webClient.pb.Response.ResponseCode.RespUsernameInvalid:
          webClient.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: incorrect username or password');
          break;

        case webClient.pb.Response.ResponseCode.RespWouldOverwriteOldSession:
          webClient.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: duplicated user session');
          break;

        case webClient.pb.Response.ResponseCode.RespUserIsBanned:
          webClient.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: banned user');
          break;

        case webClient.pb.Response.ResponseCode.RespRegistrationRequired:
          webClient.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: registration required');
          break;

        case webClient.pb.Response.ResponseCode.RespClientIdRequired:
          webClient.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: missing client ID');
          break;

        case webClient.pb.Response.ResponseCode.RespContextError:
          webClient.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: server error');
          break;

        case webClient.pb.Response.ResponseCode.RespAccountNotActivated:
          webClient.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: account not activated');
          break;

        default:
          webClient.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: unknown error ' + raw.responseCode);
      }
    }
  }
};
