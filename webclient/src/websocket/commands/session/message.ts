import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';
import { Data } from '@app/types';

export function message(userName: string, message: string): void {
  WebClient.instance.protobuf.sendSessionCommand(Data.Command_Message_ext, create(Data.Command_MessageSchema, { userName, message }));
}
