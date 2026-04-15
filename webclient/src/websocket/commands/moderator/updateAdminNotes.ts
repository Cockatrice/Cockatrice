import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { ModeratorPersistence } from '../../persistence';
import { Data } from '@app/types';

export function updateAdminNotes(userName: string, notes: string): void {
  webClient.protobuf.sendModeratorCommand(
    Data.Command_UpdateAdminNotes_ext,
    create(Data.Command_UpdateAdminNotesSchema, { userName, notes }),
    {
      onSuccess: () => {
        ModeratorPersistence.updateAdminNotes(userName, notes);
      },
    }
  );
}
