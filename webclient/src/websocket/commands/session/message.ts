import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_Message_ext, Command_MessageSchema } from '@app/generated';

export function message(userName: string, message: string): void {
  WebClient.instance.protobuf.sendSessionCommand(Command_Message_ext, create(Command_MessageSchema, { userName, message }));
}
