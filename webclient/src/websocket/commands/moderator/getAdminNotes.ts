import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { ModeratorPersistence } from '../../persistence';
import { Data } from '@app/types';

export function getAdminNotes(userName: string): void {
  webClient.protobuf.sendModeratorCommand(Data.Command_GetAdminNotes_ext, create(Data.Command_GetAdminNotesSchema, { userName }), {
    responseExt: Data.Response_GetAdminNotes_ext,
    onSuccess: (response) => {
      ModeratorPersistence.getAdminNotes(userName, response.notes);
    },
  });
}
