import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';
import webClient from '../../WebClient';
import { AdminPersistence } from '../../persistence';

export function adjustMod(userName: string, shouldBeMod?: boolean, shouldBeJudge?: boolean): void {
  webClient.protobuf.sendAdminCommand(
    Data.Command_AdjustMod_ext,
    create(Data.Command_AdjustModSchema, { userName, shouldBeMod, shouldBeJudge }),
    {
      onSuccess: () => {
        AdminPersistence.adjustMod(userName, shouldBeMod, shouldBeJudge);
      },
    }
  );
}
