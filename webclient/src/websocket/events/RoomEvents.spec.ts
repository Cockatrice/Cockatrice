import { Message } from 'types';

import {
  RoomEvents,
  RoomEvent,
  JoinRoomData,
  LeaveRoomData,
  ListGamesData,
} from './RoomEvents';
import { RoomPersistence } from '../persistence/RoomPersistence';

describe('RoomEvents', () => {
  it('.Event_JoinRoom.ext should call RoomPersistence.userJoined', () => {
    spyOn(RoomPersistence, 'userJoined');
    const data: JoinRoomData = { userInfo: {} as any };
    const event: RoomEvent = { roomEvent: { roomId: 1 } };

    RoomEvents['.Event_JoinRoom.ext'](data, event);

    expect(RoomPersistence.userJoined).toHaveBeenCalledWith(
      event.roomEvent.roomId,
      data.userInfo
    );
  });

  it('.Event_LeaveRoom.ext should call RoomPersistence.userLeft', () => {
    spyOn(RoomPersistence, 'userLeft');
    const data: LeaveRoomData = { name: '' };
    const event: RoomEvent = { roomEvent: { roomId: 1 } };

    RoomEvents['.Event_LeaveRoom.ext'](data, event);

    expect(RoomPersistence.userLeft).toHaveBeenCalledWith(
      event.roomEvent.roomId,
      data.name
    );
  });

  it('.Event_ListGames.ext should call RoomPersistence.updateGames', () => {
    spyOn(RoomPersistence, 'updateGames');
    const data: ListGamesData = { gameList: [] };
    const event: RoomEvent = { roomEvent: { roomId: 1 } };

    RoomEvents['.Event_ListGames.ext'](data, event);

    expect(RoomPersistence.updateGames).toHaveBeenCalledWith(
      event.roomEvent.roomId,
      data.gameList
    );
  });

  it('.Event_RoomSay.ext should call RoomPersistence.addMessage', () => {
    spyOn(RoomPersistence, 'addMessage');
    const data: Message = {} as any;
    const event: RoomEvent = { roomEvent: { roomId: 1 } };

    RoomEvents['.Event_RoomSay.ext'](data, event);

    expect(RoomPersistence.addMessage).toHaveBeenCalledWith(
      event.roomEvent.roomId,
      data
    );
  });
});