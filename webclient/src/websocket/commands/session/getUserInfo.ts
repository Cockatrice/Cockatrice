import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function getUserInfo(userName: string): void {
  WebClient.instance.protobuf.sendSessionCommand(Data.Command_GetUserInfo_ext, create(Data.Command_GetUserInfoSchema, { userName }), {
    responseExt: Data.Response_GetUserInfo_ext,
    onSuccess: (response) => {
      WebClient.instance.response.session.getUserInfo(response.userInfo);
    },
  });
}
