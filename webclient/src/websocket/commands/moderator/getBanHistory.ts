import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_GetBanHistory_ext, Command_GetBanHistorySchema } from 'generated/proto/moderator_commands_pb';
import { ModeratorPersistence } from '../../persistence';
import { Response_BanHistory_ext } from 'generated/proto/response_ban_history_pb';

export function getBanHistory(userName: string): void {
  webClient.protobuf.sendModeratorCommand(Command_GetBanHistory_ext, create(Command_GetBanHistorySchema, { userName }), {
    responseExt: Response_BanHistory_ext,
    onSuccess: (response) => {
      ModeratorPersistence.banHistory(userName, response.banList);
    },
  });
}
