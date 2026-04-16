import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function updateAdminNotes(userName: string, notes: string): void {
  WebClient.instance.protobuf.sendModeratorCommand(
    Data.Command_UpdateAdminNotes_ext,
    create(Data.Command_UpdateAdminNotesSchema, { userName, notes }),
    {
      onSuccess: () => {
        WebClient.instance.response.moderator.updateAdminNotes(userName, notes);
      },
    }
  );
}
