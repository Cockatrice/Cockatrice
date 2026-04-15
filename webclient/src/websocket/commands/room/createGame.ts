import { BackendService } from '../../services/BackendService';
import { RoomPersistence } from '../../persistence';
import { GameConfig } from 'types';

export function createGame(roomId: number, gameConfig: GameConfig): void {
  BackendService.sendRoomCommand(roomId, 'Command_CreateGame', gameConfig, {
    onSuccess: () => {
      RoomPersistence.gameCreated(roomId);
    },
  });
}
