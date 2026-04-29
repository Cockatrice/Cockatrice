import { create } from '@bufbuild/protobuf';
import { Command_AdjustMod_ext, Command_AdjustModSchema } from '@app/generated';
import { WebClient } from '../../WebClient';
export function adjustMod(userName: string, shouldBeMod?: boolean, shouldBeJudge?: boolean): void {
  WebClient.instance.protobuf.sendAdminCommand(
    Command_AdjustMod_ext,
    create(Command_AdjustModSchema, { userName, shouldBeMod, shouldBeJudge }),
    {
      onSuccess: () => {
        WebClient.instance.response.admin.adjustMod(userName, shouldBeMod, shouldBeJudge);
      },
    }
  );
}
