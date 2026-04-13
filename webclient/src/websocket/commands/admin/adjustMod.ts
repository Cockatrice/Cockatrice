import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_AdjustMod_ext, Command_AdjustModSchema } from 'generated/proto/admin_commands_pb';
import { AdminPersistence } from '../../persistence';

export function adjustMod(userName: string, shouldBeMod?: boolean, shouldBeJudge?: boolean): void {
  BackendService.sendAdminCommand(Command_AdjustMod_ext, create(Command_AdjustModSchema, { userName, shouldBeMod, shouldBeJudge }), {
    onSuccess: () => {
      AdminPersistence.adjustMod(userName, shouldBeMod, shouldBeJudge);
    },
  });
}
