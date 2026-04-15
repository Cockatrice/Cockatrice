import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';
import webClient from '../../WebClient';
import { AdminPersistence } from '../../persistence';

export function shutdownServer(reason: string, minutes: number): void {
  webClient.protobuf.sendAdminCommand(Data.Command_ShutdownServer_ext, create(Data.Command_ShutdownServerSchema, { reason, minutes }), {
    onSuccess: () => {
      AdminPersistence.shutdownServer();
    },
  });
}
