import { PlayerGamePropertiesData } from '../events/session/interfaces';
import { LeaveGameReason } from '../../types';

export class GamePersistence {
  static joinGame(playerGamePropertiesData: PlayerGamePropertiesData) {
    console.log('joinGame', playerGamePropertiesData);
  }

  static leaveGame(reason: LeaveGameReason) {
    console.log('leaveGame', reason);
  }
}
