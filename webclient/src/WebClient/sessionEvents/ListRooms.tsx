import * as _ from 'lodash';

export const ListRooms = {
  id: '.Event_ListRooms.ext',
  action: ({ roomList }, webClient) => {
    webClient.services.rooms.updateRooms(roomList);
  	
    if (webClient.options.autojoinrooms) {
	    _.each(roomList, ({ autoJoin, roomId }) => {
	    	if (autoJoin) {
		    	webClient.services.server.joinRoom(roomId);
	    	}
	    });
    }
  }
};