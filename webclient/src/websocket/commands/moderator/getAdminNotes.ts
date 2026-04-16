import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function getAdminNotes(userName: string): void {
  WebClient.instance.protobuf.sendModeratorCommand(Data.Command_GetAdminNotes_ext, create(Data.Command_GetAdminNotesSchema, { userName }), {
    responseExt: Data.Response_GetAdminNotes_ext,
    onSuccess: (response) => {
      WebClient.instance.response.moderator.getAdminNotes(userName, response.notes);
    },
  });
}
