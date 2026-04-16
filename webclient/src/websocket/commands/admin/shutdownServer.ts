import { create } from '@bufbuild/protobuf';
import { Command_ShutdownServer_ext, Command_ShutdownServerSchema } from '@app/generated';
import { WebClient } from '../../WebClient';
export function shutdownServer(reason: string, minutes: number): void {
  WebClient.instance.protobuf.sendAdminCommand(
    Command_ShutdownServer_ext,
    create(Command_ShutdownServerSchema, { reason, minutes }),
    {
      onSuccess: () => {
        WebClient.instance.response.admin.shutdownServer();
      },
    }
  );
}
