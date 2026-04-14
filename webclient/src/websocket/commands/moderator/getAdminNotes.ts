import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_GetAdminNotes_ext, Command_GetAdminNotesSchema } from 'generated/proto/moderator_commands_pb';
import { ModeratorPersistence } from '../../persistence';
import { Response_GetAdminNotes_ext } from 'generated/proto/response_get_admin_notes_pb';

export function getAdminNotes(userName: string): void {
  webClient.protobuf.sendModeratorCommand(Command_GetAdminNotes_ext, create(Command_GetAdminNotesSchema, { userName }), {
    responseExt: Response_GetAdminNotes_ext,
    onSuccess: (response) => {
      ModeratorPersistence.getAdminNotes(userName, response.notes);
    },
  });
}
