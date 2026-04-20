import { useMemo } from 'react';
import { generatePath, useNavigate } from 'react-router-dom';

import ListItemButton from '@mui/material/ListItemButton';
import Paper from '@mui/material/Paper';

import { AuthGuard, ThreePaneLayout, UserDisplay, VirtualList } from '@app/components';
import { useReduxEffect } from '@app/hooks';
import { RoomsSelectors, RoomsTypes, ServerSelectors, useAppSelector } from '@app/store';
import { App, Data } from '@app/types';
import Rooms from './Rooms';
import Layout from '../Layout/Layout';

import './Server.css';

const Server = () => {
  const message = useAppSelector(state => ServerSelectors.getMessage(state));
  const rooms = useAppSelector(state => RoomsSelectors.getRooms(state));
  const joinedRooms = useAppSelector(state => RoomsSelectors.getJoinedRooms(state));
  const users = useAppSelector(state => ServerSelectors.getSortedUsers(state));
  const navigate = useNavigate();

  useReduxEffect<{ roomInfo: Data.ServerInfo_Room }>((action) => {
    const roomId = action.payload.roomInfo.roomId.toString();
    navigate(generatePath(App.RouteEnum.ROOM, { roomId }));
  }, RoomsTypes.JOIN_ROOM, []);

  const userItems = useMemo(
    () => users.map((user) => (
      <ListItemButton key={user.name} dense>
        <UserDisplay user={user} />
      </ListItemButton>
    )),
    [users],
  );

  return (
    <Layout className="server-rooms">
      <AuthGuard />

      <ThreePaneLayout
        top={(
          <Paper className="serverRoomWrapper overflow-scroll">
            <Rooms rooms={rooms} joinedRooms={joinedRooms} />
          </Paper>
        )}

        bottom={(
          <Paper className="serverMessage overflow-scroll">
            {/* message is sanitized via DOMPurify in websocket/events/session/serverMessage.ts */}
            <div className="serverMessage__content" dangerouslySetInnerHTML={{ __html: message }} />
          </Paper>
        )}

        side={(
          <Paper className="server-rooms__side overflow-scroll">
            <div className="server-rooms__side-label">
              Users connected to server: {users.length}
            </div>
            <VirtualList items={userItems} />
          </Paper>
        )}
      />
    </Layout>
  );
}

export default Server;
