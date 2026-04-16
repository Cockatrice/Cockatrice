import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';
import { Command_Ping_ext, Command_PingSchema } from '@app/generated';

export function ping(pingReceived: () => void): void {
  WebClient.instance.protobuf.sendSessionCommand(Command_Ping_ext, create(Command_PingSchema), {
    onResponse: () => pingReceived(),
  });
}
