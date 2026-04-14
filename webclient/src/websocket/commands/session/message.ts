import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_Message_ext, Command_MessageSchema } from 'generated/proto/session_commands_pb';

export function message(userName: string, message: string): void {
  webClient.protobuf.sendSessionCommand(Command_Message_ext, create(Command_MessageSchema, { userName, message }));
}
