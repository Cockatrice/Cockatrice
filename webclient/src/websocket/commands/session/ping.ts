import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_Ping_ext, Command_PingSchema } from 'generated/proto/session_commands_pb';

export function ping(pingReceived: () => void): void {
  webClient.protobuf.sendSessionCommand(Command_Ping_ext, create(Command_PingSchema), {
    onResponse: () => pingReceived(),
  });
}
