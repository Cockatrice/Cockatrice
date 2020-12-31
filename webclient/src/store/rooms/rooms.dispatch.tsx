import { reset } from 'redux-form';
import { Actions } from "./rooms.actions";
import { store } from "../store";

// const history = useHistory();

export const Dispatch = {
  clearStore: () => {
    store.dispatch(Actions.clearStore());
  },
  
  updateRooms: rooms => {
    store.dispatch(Actions.updateRooms(rooms));
  },

  joinRoom: roomInfo => {
    store.dispatch(Actions.joinRoom(roomInfo));

  },

  leaveRoom: roomId => {
    store.dispatch(Actions.leaveRoom(roomId));
  },

  addMessage: (roomId, message) => {
    if (message.name) {
      store.dispatch(reset('sayMessage'));
    }
    
    store.dispatch(Actions.addMessage(roomId, message));
  },

  updateGames: (roomId, games) => {
    store.dispatch(Actions.updateGames(roomId, games));
  },

  userJoined: (roomId, user) => {
    store.dispatch(Actions.userJoined(roomId, user));
  },

  userLeft: (roomId, name) => {
    store.dispatch(Actions.userLeft(roomId, name));
  },

  sortGames: (roomId, field, order) => {
    store.dispatch(Actions.sortGames(roomId, field, order));
  }
}