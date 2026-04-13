import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_AccountEdit_ext, Command_AccountEditSchema } from 'generated/proto/session_commands_pb';
import { SessionPersistence } from '../../persistence';

export function accountEdit(passwordCheck: string, realName?: string, email?: string, country?: string): void {
  const cmd = create(Command_AccountEditSchema, { passwordCheck, realName, email, country });
  BackendService.sendSessionCommand(Command_AccountEdit_ext, cmd, {
    onSuccess: () => {
      SessionPersistence.accountEditChanged(realName, email, country);
    },
  });
}
