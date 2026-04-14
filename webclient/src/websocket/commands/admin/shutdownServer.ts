import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_ShutdownServer_ext, Command_ShutdownServerSchema } from 'generated/proto/admin_commands_pb';
import { AdminPersistence } from '../../persistence';

export function shutdownServer(reason: string, minutes: number): void {
  webClient.protobuf.sendAdminCommand(Command_ShutdownServer_ext, create(Command_ShutdownServerSchema, { reason, minutes }), {
    onSuccess: () => {
      AdminPersistence.shutdownServer();
    },
  });
}
