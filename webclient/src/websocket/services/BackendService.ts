import { create, getExtension, setExtension } from '@bufbuild/protobuf';
import type { GenExtension } from '@bufbuild/protobuf/codegenv2';

import webClient from '../WebClient';
import { Response_ResponseCode, type Response } from 'generated/proto/response_pb';
import { SessionCommandSchema, type SessionCommand } from 'generated/proto/session_commands_pb';
import { GameCommandSchema, type GameCommand } from 'generated/proto/game_commands_pb';
import { RoomCommandSchema, type RoomCommand } from 'generated/proto/room_commands_pb';
import { ModeratorCommandSchema, type ModeratorCommand } from 'generated/proto/moderator_commands_pb';
import { AdminCommandSchema, type AdminCommand } from 'generated/proto/admin_commands_pb';

interface CommandOptionsBase {
  onError?: (responseCode: number, raw: Response) => void;
  onResponseCode?: { [code: number]: (raw: Response) => void };
  onResponse?: (raw: Response) => void;
}

export interface CommandOptionsWithResponse<R> extends CommandOptionsBase {
  responseExt: GenExtension<Response, R>;
  onSuccess?: (response: R, raw: Response) => void;
}

export interface CommandOptionsWithoutResponse extends CommandOptionsBase {
  responseExt?: undefined;
  onSuccess?: () => void;
}

export type CommandOptions<R = unknown> = CommandOptionsWithResponse<R> | CommandOptionsWithoutResponse;

function hasResponseExt<R>(options: CommandOptions<R>): options is CommandOptionsWithResponse<R> {
  return options.responseExt !== undefined;
}

export class BackendService {
  static sendGameCommand<V, R>(gameId: number, ext: GenExtension<GameCommand, V>, value: V, options?: CommandOptions<R>): void {
    const cmd = create(GameCommandSchema);
    setExtension(cmd, ext, value);
    webClient.protobuf.sendGameCommand(gameId, cmd, (raw: Response) => {
      if (options) {
        BackendService.handleResponse(ext.typeName, raw, options);
      }
    });
  }

  static sendSessionCommand<V, R>(ext: GenExtension<SessionCommand, V>, value: V, options?: CommandOptions<R>): void {
    const cmd = create(SessionCommandSchema);
    setExtension(cmd, ext, value);
    webClient.protobuf.sendSessionCommand(cmd, raw => {
      if (options) {
        BackendService.handleResponse(ext.typeName, raw, options);
      }
    });
  }

  static sendRoomCommand<V, R>(roomId: number, ext: GenExtension<RoomCommand, V>, value: V, options?: CommandOptions<R>): void {
    const cmd = create(RoomCommandSchema);
    setExtension(cmd, ext, value);
    webClient.protobuf.sendRoomCommand(roomId, cmd, raw => {
      if (options) {
        BackendService.handleResponse(ext.typeName, raw, options);
      }
    });
  }

  static sendModeratorCommand<V, R>(ext: GenExtension<ModeratorCommand, V>, value: V, options?: CommandOptions<R>): void {
    const cmd = create(ModeratorCommandSchema);
    setExtension(cmd, ext, value);
    webClient.protobuf.sendModeratorCommand(cmd, raw => {
      if (options) {
        BackendService.handleResponse(ext.typeName, raw, options);
      }
    });
  }

  static sendAdminCommand<V, R>(ext: GenExtension<AdminCommand, V>, value: V, options?: CommandOptions<R>): void {
    const cmd = create(AdminCommandSchema);
    setExtension(cmd, ext, value);
    webClient.protobuf.sendAdminCommand(cmd, raw => {
      if (options) {
        BackendService.handleResponse(ext.typeName, raw, options);
      }
    });
  }

  private static handleResponse<R>(typeName: string, raw: Response, options: CommandOptions<R>): void {
    if (options.onResponse) {
      options.onResponse(raw);
      return;
    }

    const { responseCode } = raw;

    if (responseCode === Response_ResponseCode.RespOk) {
      if (hasResponseExt(options)) {
        options.onSuccess?.(getExtension(raw, options.responseExt), raw);
      } else {
        options.onSuccess?.();
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
      console.error(`${typeName} failed with response code: ${responseCode}`);
    }
  }
}

