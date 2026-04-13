import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_GetWarnList_ext, Command_GetWarnListSchema } from 'generated/proto/moderator_commands_pb';
import { ModeratorPersistence } from '../../persistence';
import { Response_WarnList_ext } from 'generated/proto/response_warn_list_pb';

export function getWarnList(modName: string, userName: string, userClientid: string): void {
  BackendService.sendModeratorCommand(Command_GetWarnList_ext, create(Command_GetWarnListSchema, { modName, userName, userClientid }), {
    responseExt: Response_WarnList_ext,
    onSuccess: (response) => {
      ModeratorPersistence.warnListOptions(response.warning);
    },
  });
}
