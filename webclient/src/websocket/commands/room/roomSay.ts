import { BackendService } from '../../services/BackendService';

export function roomSay(roomId: number, message: string): void {
  const trimmed = message.trim();

  if (!trimmed) {
    return;
  }

  BackendService.sendRoomCommand(roomId, 'Command_RoomSay', { message: trimmed }, {});
}
