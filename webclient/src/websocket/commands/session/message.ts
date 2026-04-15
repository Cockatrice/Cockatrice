import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Data } from '@app/types';

export function message(userName: string, message: string): void {
  webClient.protobuf.sendSessionCommand(Data.Command_Message_ext, create(Data.Command_MessageSchema, { userName, message }));
}
