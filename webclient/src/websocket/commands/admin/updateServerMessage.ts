import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_UpdateServerMessage_ext, Command_UpdateServerMessageSchema } from 'generated/proto/admin_commands_pb';
import { AdminPersistence } from '../../persistence';

export function updateServerMessage(): void {
  BackendService.sendAdminCommand(Command_UpdateServerMessage_ext, create(Command_UpdateServerMessageSchema), {
    onSuccess: () => {
      AdminPersistence.updateServerMessage();
    },
  });
}
