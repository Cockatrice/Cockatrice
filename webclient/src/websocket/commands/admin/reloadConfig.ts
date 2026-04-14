import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_ReloadConfig_ext, Command_ReloadConfigSchema } from 'generated/proto/admin_commands_pb';
import { AdminPersistence } from '../../persistence';

export function reloadConfig(): void {
  webClient.protobuf.sendAdminCommand(Command_ReloadConfig_ext, create(Command_ReloadConfigSchema), {
    onSuccess: () => {
      AdminPersistence.reloadConfig();
    },
  });
}
