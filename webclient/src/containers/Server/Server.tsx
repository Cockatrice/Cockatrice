// eslint-disable-next-line
import React, { Component } from "react";
import { connect } from 'react-redux';
import { withRouter } from 'react-router-dom';

import ListItem from '@material-ui/core/ListItem';
import Paper from '@material-ui/core/Paper';

import { RoomsSelectors, ServerSelectors } from 'store';

import { AuthGuard, ThreePaneLayout, UserDisplay, VirtualList } from 'components';
import { Room, User } from 'types';
import Rooms from './Rooms';

import './Server.css';

class Server extends Component<ServerProps, ServerState> {
  render() {
    const { message, rooms, joinedRooms, history, users } = this.props;

    return (
      <div className="server-rooms">
        <AuthGuard />

        <ThreePaneLayout
          top={(
            <Paper className="serverRoomWrapper overflow-scroll">
              <Rooms rooms={rooms} joinedRooms={joinedRooms} history={history} />
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
      </div>
    );
  }
}

interface ServerProps {
  message: string;
  rooms: Room[];
  joinedRooms: Room[];
  users: User[];
  history: any;
}

interface ServerState {

}

const mapStateToProps = state => ({
  message: ServerSelectors.getMessage(state),
  rooms: RoomsSelectors.getRooms(state),
  joinedRooms: RoomsSelectors.getJoinedRooms(state),
  users: ServerSelectors.getUsers(state)
});

export default withRouter(connect(mapStateToProps)(Server));
