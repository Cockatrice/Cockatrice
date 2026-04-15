import { BackendService } from '../../services/BackendService';
import { RoomPersistence } from '../../persistence';
import { JoinGameParams } from 'types';

export function joinGame(roomId: number, joinGameParams: JoinGameParams): void {
  BackendService.sendRoomCommand(roomId, 'Command_JoinGame', joinGameParams, {
    onSuccess: () => {
      RoomPersistence.joinedGame(roomId, joinGameParams.gameId);
    },
  });
}
