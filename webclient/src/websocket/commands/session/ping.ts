import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';
import { Data } from '@app/types';

export function ping(pingReceived: () => void): void {
  WebClient.instance.protobuf.sendSessionCommand(Data.Command_Ping_ext, create(Data.Command_PingSchema), {
    onResponse: () => pingReceived(),
  });
}
