import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_GetUserInfo_ext, Command_GetUserInfoSchema, Response_GetUserInfo_ext } from '@app/generated';

export function getUserInfo(userName: string): void {
  WebClient.instance.protobuf.sendSessionCommand(Command_GetUserInfo_ext, create(Command_GetUserInfoSchema, { userName }), {
    responseExt: Response_GetUserInfo_ext,
    onSuccess: (response) => {
      WebClient.instance.response.session.getUserInfo(response.userInfo);
    },
  });
}
