import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';
import { WebClient } from '../../WebClient';
export function shutdownServer(reason: string, minutes: number): void {
  WebClient.instance.protobuf.sendAdminCommand(
    Data.Command_ShutdownServer_ext,
    create(Data.Command_ShutdownServerSchema, { reason, minutes }),
    {
      onSuccess: () => {
        WebClient.instance.response.admin.shutdownServer();
      },
    }
  );
}
