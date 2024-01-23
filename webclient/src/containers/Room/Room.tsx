import React, { useEffect } from 'react';
import { connect } from 'react-redux';
import { useNavigate, useParams, generatePath } from 'react-router-dom';

import ListItem from '@mui/material/ListItem';
import Paper from '@mui/material/Paper';

import { RoomsService } from 'api';
import { ScrollToBottomOnChanges, ThreePaneLayout, UserDisplay, VirtualList, AuthGuard } from 'components';
import { RoomsStateMessages, RoomsStateRooms, JoinedRooms, RoomsSelectors, RoomsTypes } from 'store';
import { RouteEnum } from 'types';
import Layout from 'containers/Layout/Layout';

import OpenGames from './OpenGames';
import Messages from './Messages';
import SayMessage from './SayMessage';

import './Room.css';

// @TODO (3)
const Room = (props) => {
  const { joined, rooms, messages } = props;
  const navigate = useNavigate();
  const params = useParams();

  const roomId = parseInt(params.roomId, 0);
  const room = rooms[roomId];
  const roomMessages = messages[roomId];
  const users = room.userList;

  useEffect(() => {
    if (!joined.find(({ roomId: id }) => id === roomId)) {
      navigate(generatePath(RouteEnum.SERVER));
    }
  }, [joined]);

  const handleRoomSay = ({ message }) => {
    if (message) {
      RoomsService.roomSay(roomId, message);
    }
  }

  return (
    <Layout className="room-view">
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
    </Layout>
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

export default connect(mapStateToProps)(Room);
