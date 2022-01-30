import React from 'react';
import { connect } from 'react-redux';
import { withRouter, generatePath } from 'react-router-dom';

import ListItem from '@material-ui/core/ListItem';
import Paper from '@material-ui/core/Paper';

import { RoomsService } from 'api';
import { ScrollToBottomOnChanges, ThreePaneLayout, UserDisplay, VirtualList, AuthGuard } from 'components';
import { RoomsStateMessages, RoomsStateRooms, JoinedRooms, RoomsSelectors, RoomsTypes } from 'store';
import { RouteEnum } from 'types';

import OpenGames from './OpenGames';
import Messages from './Messages';
import SayMessage from './SayMessage';

import './Room.css';

// @TODO (3)
function Room(props) {

  const { joined, match, history, rooms, messages } = props;
  const roomId = parseInt(match.params.roomId, 0);

  if (!joined.find(({ roomId: id }) => id === roomId)) {
    history.push(generatePath(RouteEnum.SERVER));
  }

  function handleRoomSay({ message }) {
    if (message) {
      RoomsService.roomSay(roomId, message);
    }
  }

  const room = rooms[roomId];

  const roomMessages = messages[roomId];
  const users = room.userList;

  return (
    <div className="room-view">
      <AuthGuard />

      <div className="room-view__main">
        <ThreePaneLayout
          fixedHeight

          top={(
            <Paper className="room-view__games overflow-scroll">
              <OpenGames room={room} />
            </Paper>
          )}

          bottom={(
            <div className="room-view__messages">
              <Paper className="room-view__messages-content overflow-scroll">
                <ScrollToBottomOnChanges changes={roomMessages} content={(
                  <Messages messages={roomMessages} />
                )} />
              </Paper>
              <Paper className="room-view__messages-sayMessage">
                <SayMessage onSubmit={handleRoomSay} />
              </Paper>
            </div>
          )}

          side={(
            <Paper className="room-view__side overflow-scroll">
              <div className="room-view__side-label">
                  Users in this room: {users.length}
              </div>
              <VirtualList
                className="room-view__side-list"
                itemKey={(index, data) => users[index].name }
                items={ users.map(user => (
                  <ListItem button className="room-view__side-list__item">
                    <UserDisplay user={user} />
                  </ListItem>
                )) }
              />
            </Paper>
          )}
        />
      </div>
    </div>
  );
}

interface RoomProps {
  messages: RoomsStateMessages;
  rooms: RoomsStateRooms;
  joined: JoinedRooms;
}

const mapStateToProps = state => ({
  messages: RoomsSelectors.getMessages(state),
  rooms: RoomsSelectors.getRooms(state),
  joined: RoomsSelectors.getJoinedRooms(state),
});

export default withRouter(connect(mapStateToProps)(Room));
