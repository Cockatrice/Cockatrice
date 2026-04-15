import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';
import webClient from '../../WebClient';
import { AdminPersistence } from '../../persistence';

export function reloadConfig(): void {
  webClient.protobuf.sendAdminCommand(Data.Command_ReloadConfig_ext, create(Data.Command_ReloadConfigSchema), {
    onSuccess: () => {
      AdminPersistence.reloadConfig();
    },
  });
}
