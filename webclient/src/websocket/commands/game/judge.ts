import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_JudgeSchema, Command_Judge_ext } from 'generated/proto/game_commands_pb';
import type { GameCommand } from 'generated/proto/game_commands_pb';

export function judge(gameId: number, targetId: number, innerGameCommand: GameCommand): void {
  BackendService.sendGameCommand(gameId, Command_Judge_ext, create(Command_JudgeSchema, {
    targetId,
    gameCommand: [innerGameCommand],
  }));
}

