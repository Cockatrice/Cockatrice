import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';
import { WebClient } from '../../WebClient';
export function updateServerMessage(): void {
  WebClient.instance.protobuf.sendAdminCommand(Data.Command_UpdateServerMessage_ext, create(Data.Command_UpdateServerMessageSchema), {
    onSuccess: () => {
      WebClient.instance.response.admin.updateServerMessage();
    },
  });
}
