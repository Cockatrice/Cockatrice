import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_AccountPassword_ext, Command_AccountPasswordSchema } from 'generated/proto/session_commands_pb';
import { SessionPersistence } from '../../persistence';

export function accountPassword(oldPassword: string, newPassword: string, hashedNewPassword: string): void {
  const cmd = create(Command_AccountPasswordSchema, { oldPassword, newPassword, hashedNewPassword });
  webClient.protobuf.sendSessionCommand(Command_AccountPassword_ext, cmd, {
    onSuccess: () => {
      SessionPersistence.accountPasswordChange();
    },
  });
}
