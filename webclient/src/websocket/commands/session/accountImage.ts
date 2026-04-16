import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function accountImage(image: Uint8Array): void {
  WebClient.instance.protobuf.sendSessionCommand(Data.Command_AccountImage_ext, create(Data.Command_AccountImageSchema, { image }), {
    onSuccess: () => {
      WebClient.instance.response.session.accountImageChanged(image);
    },
  });
}
