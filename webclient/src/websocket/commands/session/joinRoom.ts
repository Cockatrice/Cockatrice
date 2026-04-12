import { BackendService } from '../../services/BackendService';
import { RoomPersistence } from '../../persistence';

export function joinRoom(roomId: number): void {
  BackendService.sendSessionCommand('Command_JoinRoom', { roomId }, {
    responseName: 'Response_JoinRoom',
    onSuccess: (response) => {
      RoomPersistence.joinRoom(response.roomInfo);
    },
  });
}
