import { ServerService } from 'common/services/data';
import { guid } from 'common/services/util';
import { StatusEnum } from 'common/types';

import WebClient from '../WebClient';

export const ServerIdentification = {
  id: '.Event_ServerIdentification.ext',
  action: ({ protocolVersion }) => {
    const webClient = WebClient.getInstance();

    if (protocolVersion !== webClient.protocolVersion) {
      webClient.disconnect(); // @TODO: dispatch.close
      ServerService.updateStatus(StatusEnum.DISCONNECTED, 'Protocol version mismatch: ' + protocolVersion);
      return;
    }

    ServerService.updateStatus(StatusEnum.CONNECTED, 'Logging in...');
    webClient.resetConnectionvars();

    const loginConfig = {
      "userName" : webClient.options.user,
      "password" : webClient.options.pass,
      "clientid" : guid(),
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
    
    const CmdLogin = webClient.pb.Command_Login.create(loginConfig);

    const command = webClient.pb.SessionCommand.create({
      ".Command_Login.ext" : CmdLogin
    });

    webClient.sendSessionCommand(command, handleCommandResponse);

    function handleCommandResponse(raw) {
      const resp = raw[".Response_Login.ext"];

      webClient.debug(() => console.log('.Response_Login.ext', resp));

      switch(raw.responseCode) {
        case webClient.pb.Response.ResponseCode.RespOk:
          ServerService.updateStatus(StatusEnum.LOGGEDIN, 'Logged in.');
          webClient.startPingLoop();
          break;
        case webClient.pb.Response.ResponseCode.RespClientUpdateRequired:
          ServerService.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: missing features');
          break;
        case webClient.pb.Response.ResponseCode.RespWrongPassword:
          ServerService.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: incorrect username or password');
          break;
        case webClient.pb.Response.ResponseCode.RespWouldOverwriteOldSession:
          ServerService.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: duplicated user session');
          break;
        case webClient.pb.Response.ResponseCode.RespUserIsBanned:
          ServerService.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: banned user');
          break;
        case webClient.pb.Response.ResponseCode.RespUsernameInvalid:
          ServerService.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: invalid username');
          break;
        case webClient.pb.Response.ResponseCode.RespRegistrationRequired:
          ServerService.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: registration required');
          break;
        case webClient.pb.Response.ResponseCode.RespClientIdRequired:
          ServerService.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: missing client ID');
          break;
        case webClient.pb.Response.ResponseCode.RespContextError:
          ServerService.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: server error');
          break;
        case webClient.pb.Response.ResponseCode.RespAccountNotActivated:
          ServerService.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: account not activated');
          break;
        default:
          ServerService.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: unknown error ' + raw.responseCode);
          break;
      }
    }
  }
};
