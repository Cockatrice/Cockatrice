import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_AccountImage_ext, Command_AccountImageSchema } from '@app/generated';

export function accountImage(image: Uint8Array): void {
  WebClient.instance.protobuf.sendSessionCommand(Command_AccountImage_ext, create(Command_AccountImageSchema, { image }), {
    onSuccess: () => {
      WebClient.instance.response.session.accountImageChanged(image);
    },
  });
}
