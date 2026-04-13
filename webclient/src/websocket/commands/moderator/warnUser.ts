import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_WarnUser_ext, Command_WarnUserSchema } from 'generated/proto/moderator_commands_pb';
import { ModeratorPersistence } from '../../persistence';

export function warnUser(userName: string, reason: string, clientid?: string, removeMessages?: number): void {
  const cmd = create(Command_WarnUserSchema, { userName, reason, clientid, removeMessages });
  BackendService.sendModeratorCommand(Command_WarnUser_ext, cmd, {
    onSuccess: () => {
      ModeratorPersistence.warnUser(userName);
    },
  });
}
