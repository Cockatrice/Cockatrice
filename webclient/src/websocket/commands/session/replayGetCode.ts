import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Data } from '@app/types';

export function replayGetCode(gameId: number, onCodeReceived: (code: string) => void): void {
  webClient.protobuf.sendSessionCommand(Data.Command_ReplayGetCode_ext, create(Data.Command_ReplayGetCodeSchema, { gameId }), {
    responseExt: Data.Response_ReplayGetCode_ext,
    onSuccess: (response) => {
      onCodeReceived(response.replayCode);
    },
  });
}
