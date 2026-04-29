import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';
import { Command_ReplayGetCode_ext, Command_ReplayGetCodeSchema, Response_ReplayGetCode_ext } from '@app/generated';

export function replayGetCode(gameId: number, onCodeReceived: (code: string) => void): void {
  WebClient.instance.protobuf.sendSessionCommand(Command_ReplayGetCode_ext, create(Command_ReplayGetCodeSchema, { gameId }), {
    responseExt: Response_ReplayGetCode_ext,
    onSuccess: (response) => {
      onCodeReceived(response.replayCode);
    },
  });
}
