import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_GetWarnHistory_ext, Command_GetWarnHistorySchema } from 'generated/proto/moderator_commands_pb';
import { ModeratorPersistence } from '../../persistence';
import { Response_WarnHistory_ext } from 'generated/proto/response_warn_history_pb';

export function getWarnHistory(userName: string): void {
  webClient.protobuf.sendModeratorCommand(Command_GetWarnHistory_ext, create(Command_GetWarnHistorySchema, { userName }), {
    responseExt: Response_WarnHistory_ext,
    onSuccess: (response) => {
      ModeratorPersistence.warnHistory(userName, response.warnList);
    },
  });
}
