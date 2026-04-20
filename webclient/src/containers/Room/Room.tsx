import React, { useEffect } from 'react';
import { useNavigate, useParams, generatePath } from 'react-router-dom';

import ListItemButton from '@mui/material/ListItemButton';
import Paper from '@mui/material/Paper';

import { ScrollToBottomOnChanges, ThreePaneLayout, UserDisplay, VirtualList, AuthGuard } from '@app/components';
import { useWebClient } from '@app/hooks';
import { RoomsSelectors } from '@app/store';
import { useAppSelector } from '@app/store';
import { App } from '@app/types';
import Layout from '../Layout/Layout';

import GameSelector from './GameSelector/GameSelector';
import Messages from './Messages';
import SayMessage from './SayMessage';

import './Room.css';

const Room = () => {
  const joined = useAppSelector(state => RoomsSelectors.getJoinedRooms(state));
  const rooms = useAppSelector(state => RoomsSelectors.getRooms(state));
  const messages = useAppSelector(state => RoomsSelectors.getMessages(state));
  const navigate = useNavigate();
  const params = useParams();

  const roomId = parseInt(params.roomId, 10);
  const room = rooms[roomId];
  const roomMessages = messages[roomId];
  const users = useAppSelector(state => RoomsSelectors.getSortedRoomUsers(state, roomId));
  const webClient = useWebClient();

  useEffect(() => {
    if (!joined.find(r => r.info.roomId === roomId)) {
      navigate(generatePath(App.RouteEnum.SERVER));
    }
  }, [joined]);

  if (!room) {
    return null;
  }

  const handleRoomSay = ({ message }) => {
    if (message) {
      webClient.request.rooms.roomSay(roomId, message);
    }
  }

  return (
    <Layout className="room-view">
      <AuthGuard />

      <div className="room-view__main">
        <ThreePaneLayout
          fixedHeight

          top={(
            <div className="room-view__games">
              <GameSelector room={room} />
            </div>
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
                items={ users.map(user => (
                  <ListItemButton key={user.name} className="room-view__side-list__item">
                    <UserDisplay user={user} />
                  </ListItemButton>
                )) }
              />
            </Paper>
          )}
        />
      </div>
    </Layout>
  );
}

export default Room;
