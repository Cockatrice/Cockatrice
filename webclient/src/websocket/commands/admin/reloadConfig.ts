import { create } from '@bufbuild/protobuf';
import { Command_ReloadConfig_ext, Command_ReloadConfigSchema } from '@app/generated';
import { WebClient } from '../../WebClient';
export function reloadConfig(): void {
  WebClient.instance.protobuf.sendAdminCommand(Command_ReloadConfig_ext, create(Command_ReloadConfigSchema), {
    onSuccess: () => {
      WebClient.instance.response.admin.reloadConfig();
    },
  });
}
