import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_AdjustMod_ext, Command_AdjustModSchema } from 'generated/proto/admin_commands_pb';
import { AdminPersistence } from '../../persistence';

export function adjustMod(userName: string, shouldBeMod?: boolean, shouldBeJudge?: boolean): void {
  webClient.protobuf.sendAdminCommand(Command_AdjustMod_ext, create(Command_AdjustModSchema, { userName, shouldBeMod, shouldBeJudge }), {
    onSuccess: () => {
      AdminPersistence.adjustMod(userName, shouldBeMod, shouldBeJudge);
    },
  });
}
