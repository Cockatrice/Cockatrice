import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';
import webClient from '../../WebClient';
import { AdminPersistence } from '../../persistence';

export function updateServerMessage(): void {
  webClient.protobuf.sendAdminCommand(Data.Command_UpdateServerMessage_ext, create(Data.Command_UpdateServerMessageSchema), {
    onSuccess: () => {
      AdminPersistence.updateServerMessage();
    },
  });
}
