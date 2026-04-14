import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_UpdateServerMessage_ext, Command_UpdateServerMessageSchema } from 'generated/proto/admin_commands_pb';
import { AdminPersistence } from '../../persistence';

export function updateServerMessage(): void {
  webClient.protobuf.sendAdminCommand(Command_UpdateServerMessage_ext, create(Command_UpdateServerMessageSchema), {
    onSuccess: () => {
      AdminPersistence.updateServerMessage();
    },
  });
}
