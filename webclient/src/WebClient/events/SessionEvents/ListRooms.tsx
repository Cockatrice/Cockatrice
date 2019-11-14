import * as _ from 'lodash';

export const ListRooms = {
  id: '.Event_ListRooms.ext',
  action: ({ roomList }, webClient) => {
    // @TODO break this update into different chunks to the store
    // updateRooms
    // updateRoomGameList
    // updateRoomUserList
    webClient.services.room.updateRooms(roomList);

    if (webClient.options.autojoinrooms) {
      _.each(roomList, ({ autoJoin, roomId }) => {
        if (autoJoin) {
          webClient.commands.session.joinRoom(roomId);
        }
      });
    }
  }
};