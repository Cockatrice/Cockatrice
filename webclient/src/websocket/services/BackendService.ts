import { create, getExtension, setExtension } from '@bufbuild/protobuf';
import type { GenExtension } from '@bufbuild/protobuf';

import webClient from '../WebClient';
import { Response_ResponseCode, type Response } from 'generated/proto/response_pb';
import { SessionCommandSchema, type SessionCommand } from 'generated/proto/session_commands_pb';
import { GameCommandSchema, type GameCommand } from 'generated/proto/game_commands_pb';
import { RoomCommandSchema, type RoomCommand } from 'generated/proto/room_commands_pb';
import { ModeratorCommandSchema, type ModeratorCommand } from 'generated/proto/moderator_commands_pb';
import { AdminCommandSchema, type AdminCommand } from 'generated/proto/admin_commands_pb';

export interface CommandOptions<R = unknown> {
  responseExt?: GenExtension<Response, R>;
  onSuccess?: (response: R, raw: Response) => void;
  onError?: (responseCode: number, raw: Response) => void;
  onResponseCode?: { [code: number]: (raw: Response) => void };
  onResponse?: (raw: Response) => void;
}

export class BackendService {
  static sendGameCommand<V>(gameId: number, ext: GenExtension<GameCommand, V>, value: V, options: CommandOptions<V> = {}): void {
    const cmd = create(GameCommandSchema);
    setExtension(cmd, ext, value);
    webClient.protobuf.sendGameCommand(gameId, cmd, (raw: Response) => {
      BackendService.handleResponse(ext, raw, options);
    });
  }

  static sendSessionCommand<V>(ext: GenExtension<SessionCommand, V>, value: V, options: CommandOptions<V> = {}): void {
    const cmd = create(SessionCommandSchema);
    setExtension(cmd, ext, value);
    webClient.protobuf.sendSessionCommand(cmd, raw => {
      BackendService.handleResponse(ext, raw, options);
    });
  }

  static sendRoomCommand<V>(roomId: number, ext: GenExtension<RoomCommand, V>, value: V, options: CommandOptions<V> = {}): void {
    const cmd = create(RoomCommandSchema);
    setExtension(cmd, ext, value);
    webClient.protobuf.sendRoomCommand(roomId, cmd, raw => {
      BackendService.handleResponse(ext, raw, options);
    });
  }

  static sendModeratorCommand<V>(ext: GenExtension<ModeratorCommand, V>, value: V, options: CommandOptions<V> = {}): void {
    const cmd = create(ModeratorCommandSchema);
    setExtension(cmd, ext, value);
    webClient.protobuf.sendModeratorCommand(cmd, raw => {
      BackendService.handleResponse(ext, raw, options);
    });
  }

  static sendAdminCommand<V>(ext: GenExtension<AdminCommand, V>, value: V, options: CommandOptions<V> = {}): void {
    const cmd = create(AdminCommandSchema);
    setExtension(cmd, ext, value);
    webClient.protobuf.sendAdminCommand(cmd, raw => {
      BackendService.handleResponse(ext, raw, options);
    });
  }

  private static handleResponse<R>(ext: GenExtension<any, R>, raw: Response, options: CommandOptions<R>): void {
    if (options.onResponse) {
      options.onResponse(raw);
      return;
    }

    const { responseCode } = raw;

    if (responseCode === Response_ResponseCode.RespOk) {
      if (options.onSuccess) {
        const response = options.responseExt
          ? getExtension(raw, options.responseExt)
          : raw as unknown as R;
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
      console.error(`${ext.typeName} failed with response code: ${responseCode}`);
    }
  }
}

