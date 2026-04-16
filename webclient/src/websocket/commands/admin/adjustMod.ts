import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';
import { WebClient } from '../../WebClient';
export function adjustMod(userName: string, shouldBeMod?: boolean, shouldBeJudge?: boolean): void {
  WebClient.instance.protobuf.sendAdminCommand(
    Data.Command_AdjustMod_ext,
    create(Data.Command_AdjustModSchema, { userName, shouldBeMod, shouldBeJudge }),
    {
      onSuccess: () => {
        WebClient.instance.response.admin.adjustMod(userName, shouldBeMod, shouldBeJudge);
      },
    }
  );
}
