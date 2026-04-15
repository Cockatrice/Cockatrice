import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Data } from '@app/types';

export function judge(gameId: number, targetId: number, innerGameCommand: Data.GameCommand): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_Judge_ext, create(Data.Command_JudgeSchema, {
    targetId,
    gameCommand: [innerGameCommand],
  }));
}

