import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_ViewLogHistory_ext, Command_ViewLogHistorySchema } from 'generated/proto/moderator_commands_pb';
import { ModeratorPersistence } from '../../persistence';
import { Response_ViewLogHistory_ext } from 'generated/proto/response_viewlog_history_pb';
import { LogFilters } from 'types';

export function viewLogHistory(filters: LogFilters): void {
  BackendService.sendModeratorCommand(Command_ViewLogHistory_ext, create(Command_ViewLogHistorySchema, filters), {
    responseExt: Response_ViewLogHistory_ext,
    onSuccess: (response) => {
      ModeratorPersistence.viewLogs(response.logMessage);
    },
  });
}
