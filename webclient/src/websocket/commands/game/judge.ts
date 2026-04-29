import { create } from '@bufbuild/protobuf';
import { Command_Judge_ext, Command_JudgeSchema, type GameCommand } from '@app/generated';
import { WebClient } from '../../WebClient';

export function judge(gameId: number, targetId: number, innerGameCommand: GameCommand): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_Judge_ext, create(Command_JudgeSchema, {
    targetId,
    gameCommand: [innerGameCommand],
  }));
}
