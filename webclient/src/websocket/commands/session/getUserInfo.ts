import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_GetUserInfo_ext, Command_GetUserInfoSchema } from 'generated/proto/session_commands_pb';
import { SessionPersistence } from '../../persistence';
import { Response_GetUserInfo_ext } from 'generated/proto/response_get_user_info_pb';

export function getUserInfo(userName: string): void {
  BackendService.sendSessionCommand(Command_GetUserInfo_ext, create(Command_GetUserInfoSchema, { userName }), {
    responseExt: Response_GetUserInfo_ext,
    onSuccess: (response) => {
      SessionPersistence.getUserInfo(response.userInfo);
    },
  });
}
