import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_GetWarnHistory_ext, Command_GetWarnHistorySchema } from 'generated/proto/moderator_commands_pb';
import { ModeratorPersistence } from '../../persistence';
import { Response_WarnHistory_ext } from 'generated/proto/response_warn_history_pb';

export function getWarnHistory(userName: string): void {
  BackendService.sendModeratorCommand(Command_GetWarnHistory_ext, create(Command_GetWarnHistorySchema, { userName }), {
    responseExt: Response_WarnHistory_ext,
    onSuccess: (response) => {
      ModeratorPersistence.warnHistory(userName, response.warnList);
    },
  });
}
