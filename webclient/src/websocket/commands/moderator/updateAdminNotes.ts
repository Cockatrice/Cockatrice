import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_UpdateAdminNotes_ext, Command_UpdateAdminNotesSchema } from '@app/generated';

export function updateAdminNotes(userName: string, notes: string): void {
  WebClient.instance.protobuf.sendModeratorCommand(
    Command_UpdateAdminNotes_ext,
    create(Command_UpdateAdminNotesSchema, { userName, notes }),
    {
      onSuccess: () => {
        WebClient.instance.response.moderator.updateAdminNotes(userName, notes);
      },
    }
  );
}
