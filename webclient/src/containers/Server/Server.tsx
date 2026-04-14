// eslint-disable-next-line
import React from "react";
import { generatePath, useNavigate } from 'react-router-dom';

import ListItemButton from '@mui/material/ListItemButton';
import Paper from '@mui/material/Paper';

import { AuthGuard, ThreePaneLayout, UserDisplay, VirtualList } from 'components';
import { useReduxEffect } from 'hooks';
import { RoomsSelectors, RoomsTypes, ServerSelectors } from 'store';
import { RouteEnum } from 'types';
import { useAppSelector } from 'store/store';
import Rooms from './Rooms';
import Layout from 'containers/Layout/Layout';

import './Server.css';

const Server = () => {
  const message = useAppSelector(state => ServerSelectors.getMessage(state));
  const rooms = useAppSelector(state => RoomsSelectors.getRooms(state));
  const joinedRooms = useAppSelector(state => RoomsSelectors.getJoinedRooms(state));
  const users = useAppSelector(state => ServerSelectors.getUsers(state));
  const navigate = useNavigate();

  useReduxEffect((action: any) => {
    const roomId = action.roomInfo.roomId.toString();
    navigate(generatePath(RouteEnum.ROOM, { roomId }));
  }, RoomsTypes.JOIN_ROOM, []);

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
            <div className="serverMessage__content" dangerouslySetInnerHTML={{ __html: message }} />
          </Paper>
        )}

        side={(
          <Paper className="server-rooms__side overflow-scroll">
            <div className="server-rooms__side-label">
              Users connected to server: {users.length}
            </div>
            <VirtualList
              items={ users.map(user => (
                <ListItemButton dense>
                  <UserDisplay user={user} />
                </ListItemButton>
              )) }
            />
          </Paper>
        )}
      />
    </Layout>
  );
}

export default Server;
