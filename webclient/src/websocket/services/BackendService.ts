import webClient from '../WebClient';
import { ProtoController } from './ProtoController';

export interface CommandOptions {
  responseName?: string;
  onSuccess?: (response: any, raw: any) => void;
  onError?: (responseCode: number, raw: any) => void;
  onResponseCode?: { [code: number]: (raw: any) => void };
  onResponse?: (raw: any) => void;
}

export class BackendService {
  static sendGameCommand(gameId: number, commandName: string, params: any, options: CommandOptions = {}): void {
    const command = ProtoController.root[commandName].create(params || {});
    const gc = ProtoController.root.GameCommand.create({
      [`.${commandName}.ext`]: command,
    });
    webClient.protobuf.sendGameCommand(gameId, gc, (raw: any) => {
      BackendService.handleResponse(commandName, raw, options);
    });
  }

  static sendSessionCommand(commandName: string, params: any, options: CommandOptions): void {
    const command = ProtoController.root[commandName].create(params || {});
    const sc = ProtoController.root.SessionCommand.create({
      [`.${commandName}.ext`]: command,
    });
    webClient.protobuf.sendSessionCommand(sc, raw => {
      BackendService.handleResponse(commandName, raw, options);
    });
  }

  static sendRoomCommand(roomId: number, commandName: string, params: any, options: CommandOptions): void {
    const command = ProtoController.root[commandName].create(params || {});
    const rc = ProtoController.root.RoomCommand.create({
      [`.${commandName}.ext`]: command,
    });
    webClient.protobuf.sendRoomCommand(roomId, rc, raw => {
      BackendService.handleResponse(commandName, raw, options);
    });
  }

  static sendModeratorCommand(commandName: string, params: any, options: CommandOptions): void {
    const command = ProtoController.root[commandName].create(params || {});
    const mc = ProtoController.root.ModeratorCommand.create({
      [`.${commandName}.ext`]: command,
    });
    webClient.protobuf.sendModeratorCommand(mc, raw => {
      BackendService.handleResponse(commandName, raw, options);
    });
  }

  static sendAdminCommand(commandName: string, params: any, options: CommandOptions): void {
    const command = ProtoController.root[commandName].create(params || {});
    const ac = ProtoController.root.AdminCommand.create({
      [`.${commandName}.ext`]: command,
    });
    webClient.protobuf.sendAdminCommand(ac, raw => {
      BackendService.handleResponse(commandName, raw, options);
    });
  }

  private static handleResponse(commandName: string, raw: any, options: CommandOptions): void {
    if (options.onResponse) {
      options.onResponse(raw);
      return;
    }

    const { responseCode } = raw;

    if (responseCode === ProtoController.root.Response.ResponseCode.RespOk) {
      if (options.onSuccess) {
        const response = options.responseName
          ? raw[`.${options.responseName}.ext`]
          : raw;
        options.onSuccess(response, raw);
      }
      return;
    }

    if (options.onResponseCode?.[responseCode]) {
      options.onResponseCode[responseCode](raw);
      return;
    }

    if (options.onError) {
      options.onError(responseCode, raw);
    } else {
      console.error(`${commandName} failed with response code: ${responseCode}`);
    }
  }
}
