import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';
import { Data } from '@app/types';

export function getUserInfo(userName: string): void {
  webClient.protobuf.sendSessionCommand(Data.Command_GetUserInfo_ext, create(Data.Command_GetUserInfoSchema, { userName }), {
    responseExt: Data.Response_GetUserInfo_ext,
    onSuccess: (response) => {
      SessionPersistence.getUserInfo(response.userInfo);
    },
  });
}
