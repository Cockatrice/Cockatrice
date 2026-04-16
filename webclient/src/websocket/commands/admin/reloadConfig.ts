import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';
import { WebClient } from '../../WebClient';
export function reloadConfig(): void {
  WebClient.instance.protobuf.sendAdminCommand(Data.Command_ReloadConfig_ext, create(Data.Command_ReloadConfigSchema), {
    onSuccess: () => {
      WebClient.instance.response.admin.reloadConfig();
    },
  });
}
