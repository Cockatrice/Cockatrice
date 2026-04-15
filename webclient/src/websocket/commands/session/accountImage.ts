import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';
import { Data } from '@app/types';

export function accountImage(image: Uint8Array): void {
  webClient.protobuf.sendSessionCommand(Data.Command_AccountImage_ext, create(Data.Command_AccountImageSchema, { image }), {
    onSuccess: () => {
      SessionPersistence.accountImageChanged(image);
    },
  });
}
