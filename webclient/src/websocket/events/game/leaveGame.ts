import { LeaveGameReason } from 'types';
import { GamePersistence } from '../../persistence';


export function leaveGame(reason: LeaveGameReason): void {
  GamePersistence.leaveGame(reason);
}
