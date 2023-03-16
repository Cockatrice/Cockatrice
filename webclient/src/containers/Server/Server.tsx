// eslint-disable-next-line
import React, { Component } from "react";
import { connect } from 'react-redux';
import { generatePath, useNavigate } from 'react-router-dom';

import ListItem from '@mui/material/ListItem';
import Paper from '@mui/material/Paper';

import { AuthGuard, ThreePaneLayout, UserDisplay, VirtualList } from 'components';
import { useReduxEffect } from 'hooks';
import { RoomsSelectors, RoomsTypes, ServerSelectors } from 'store';
import { Room, RouteEnum, User } from 'types';
import Rooms from './Rooms';
import Layout from 'containers/Layout/Layout';

import './Server.css';

const Server = ({ message, rooms, joinedRooms, users }: ServerProps) => {
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
              itemKey={(index) => users[index].name }
              items={ users.map(user => (
                <ListItem button dense>
                  <UserDisplay user={user} />
                </ListItem>
              )) }
            />
          </Paper>
        )}
      />
    </Layout>
  );
}

interface ServerProps {
  message: string;
  rooms: Room[];
  joinedRooms: Room[];
  users: User[];
}

const mapStateToProps = state => ({
  message: ServerSelectors.getMessage(state),
  rooms: RoomsSelectors.getRooms(state),
  joinedRooms: RoomsSelectors.getJoinedRooms(state),
  users: ServerSelectors.getUsers(state)
});

export default connect(mapStateToProps)(Server);
