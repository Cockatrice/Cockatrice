import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_ReplayGetCodeSchema, Command_ReplayGetCode_ext } from 'generated/proto/command_replay_get_code_pb';
import { Response_ReplayGetCode_ext } from 'generated/proto/response_replay_get_code_pb';

export function replayGetCode(gameId: number, onCodeReceived: (code: string) => void): void {
  webClient.protobuf.sendSessionCommand(Command_ReplayGetCode_ext, create(Command_ReplayGetCodeSchema, { gameId }), {
    responseExt: Response_ReplayGetCode_ext,
    onSuccess: (response) => {
      onCodeReceived(response.replayCode);
    },
  });
}
