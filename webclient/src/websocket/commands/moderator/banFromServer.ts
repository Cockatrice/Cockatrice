import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_BanFromServer_ext, Command_BanFromServerSchema } from 'generated/proto/moderator_commands_pb';
import { ModeratorPersistence } from '../../persistence';

export function banFromServer(minutes: number, userName?: string, address?: string, reason?: string,
  visibleReason?: string, clientid?: string, removeMessages?: number): void {
  webClient.protobuf.sendModeratorCommand(Command_BanFromServer_ext, create(Command_BanFromServerSchema, {
    minutes, userName, address, reason, visibleReason, clientid, removeMessages
  }), {
    onSuccess: () => {
      ModeratorPersistence.banFromServer(userName);
    },
  });
}
