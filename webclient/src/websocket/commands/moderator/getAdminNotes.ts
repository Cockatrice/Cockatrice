import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_GetAdminNotes_ext, Command_GetAdminNotesSchema, Response_GetAdminNotes_ext } from '@app/generated';

export function getAdminNotes(userName: string): void {
  WebClient.instance.protobuf.sendModeratorCommand(Command_GetAdminNotes_ext, create(Command_GetAdminNotesSchema, { userName }), {
    responseExt: Response_GetAdminNotes_ext,
    onSuccess: (response) => {
      WebClient.instance.response.moderator.getAdminNotes(userName, response.notes);
    },
  });
}
