import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_JudgeSchema, Command_Judge_ext } from 'generated/proto/game_commands_pb';
import type { GameCommand } from 'generated/proto/game_commands_pb';

export function judge(gameId: number, targetId: number, innerGameCommand: GameCommand): void {
  webClient.protobuf.sendGameCommand(gameId, Command_Judge_ext, create(Command_JudgeSchema, {
    targetId,
    gameCommand: [innerGameCommand],
  }));
}

