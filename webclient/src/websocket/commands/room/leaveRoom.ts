import { BackendService } from '../../services/BackendService';
import { RoomPersistence } from '../../persistence';

export function leaveRoom(roomId: number): void {
  BackendService.sendRoomCommand(roomId, 'Command_LeaveRoom', {}, {
    onSuccess: () => {
      RoomPersistence.leaveRoom(roomId);
    },
  });
}
