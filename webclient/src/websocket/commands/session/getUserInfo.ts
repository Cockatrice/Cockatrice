import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_GetUserInfo_ext, Command_GetUserInfoSchema } from 'generated/proto/session_commands_pb';
import { SessionPersistence } from '../../persistence';
import { Response_GetUserInfo_ext } from 'generated/proto/response_get_user_info_pb';

export function getUserInfo(userName: string): void {
  webClient.protobuf.sendSessionCommand(Command_GetUserInfo_ext, create(Command_GetUserInfoSchema, { userName }), {
    responseExt: Response_GetUserInfo_ext,
    onSuccess: (response) => {
      SessionPersistence.getUserInfo(response.userInfo);
    },
  });
}
