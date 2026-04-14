import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_AccountImage_ext, Command_AccountImageSchema } from 'generated/proto/session_commands_pb';
import { SessionPersistence } from '../../persistence';

export function accountImage(image: Uint8Array): void {
  webClient.protobuf.sendSessionCommand(Command_AccountImage_ext, create(Command_AccountImageSchema, { image }), {
    onSuccess: () => {
      SessionPersistence.accountImageChanged(image);
    },
  });
}
