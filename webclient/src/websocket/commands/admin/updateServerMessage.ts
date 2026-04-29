import { create } from '@bufbuild/protobuf';
import { Command_UpdateServerMessage_ext, Command_UpdateServerMessageSchema } from '@app/generated';
import { WebClient } from '../../WebClient';
export function updateServerMessage(): void {
  WebClient.instance.protobuf.sendAdminCommand(Command_UpdateServerMessage_ext, create(Command_UpdateServerMessageSchema), {
    onSuccess: () => {
      WebClient.instance.response.admin.updateServerMessage();
    },
  });
}
