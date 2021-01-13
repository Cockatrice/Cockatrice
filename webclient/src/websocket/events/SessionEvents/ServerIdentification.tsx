import { StatusEnum } from "types";

export const ServerIdentification = {
  id: ".Event_ServerIdentification.ext",
  action: (info, webClient, _raw) => {
    const { serverName, serverVersion, protocolVersion } = info;

    if (protocolVersion !== webClient.protocolVersion) {
      webClient.disconnect();
      webClient.updateStatus(StatusEnum.DISCONNECTED, "Protocol version mismatch: " + protocolVersion);
      return;
    }

    webClient.resetConnectionvars();
    webClient.updateStatus(StatusEnum.LOGGINGIN, "Logging in...");
    webClient.services.session.updateInfo(serverName, serverVersion);
    webClient.commands.session.login();
  }
};
