import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_WarnUser_ext, Command_WarnUserSchema } from 'generated/proto/moderator_commands_pb';
import { ModeratorPersistence } from '../../persistence';

export function warnUser(userName: string, reason: string, clientid?: string, removeMessages?: number): void {
  const cmd = create(Command_WarnUserSchema, { userName, reason, clientid, removeMessages });
  webClient.protobuf.sendModeratorCommand(Command_WarnUser_ext, cmd, {
    onSuccess: () => {
      ModeratorPersistence.warnUser(userName);
    },
  });
}
