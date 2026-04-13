import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import {
  Command_UpdateAdminNotes_ext, Command_UpdateAdminNotesSchema,
} from 'generated/proto/moderator_commands_pb';
import { ModeratorPersistence } from '../../persistence';

export function updateAdminNotes(userName: string, notes: string): void {
  BackendService.sendModeratorCommand(Command_UpdateAdminNotes_ext, create(Command_UpdateAdminNotesSchema, { userName, notes }), {
    onSuccess: () => {
      ModeratorPersistence.updateAdminNotes(userName, notes);
    },
  });
}
