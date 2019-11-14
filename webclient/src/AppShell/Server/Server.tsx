// eslint-disable-next-line
import React, { Component } from 'react';
import { connect } from 'react-redux';

import { Selectors as RoomsSelectors } from 'store/rooms';
import { Selectors as ServerSelectors } from 'store/server';
import { User } from 'types';

import ThreePaneLayout from 'AppShell/common/components/ThreePaneLayout/ThreePaneLayout';
import UserDisplay from 'AppShell/common/components/UserDisplay/UserDisplay';

import { AuthenticationService } from 'AppShell/common/services';

import ConnectForm from './ConnectForm/ConnectForm';
import Rooms from './Rooms/Rooms';

import './Server.css';

class Server extends Component<ServerProps> {
  render() {
    const { message, rooms, state, users } = this.props;
    const isConnected = AuthenticationService.isConnected(state);

    return (
      <div className="server">{
        isConnected
          ? (
              <div className="server-rooms">
                <ThreePaneLayout
                  top={(
                    <Rooms rooms={rooms} />
                  )}

                  bottom={(
                    <div className="serverMessage" dangerouslySetInnerHTML={{ __html: message }}></div>
                  )}

                  side={(
                    <div className="room-view__side">
                      <div className="room-view__side-label">
                        Users connected to server: {users.length}
                      </div>
                      <div className="room-view__side-users">
                        { users.map(user => {
                          return (<UserDisplay user={user} key={user.name} />);
                        } ) }
                      </div>
                    </div>
                  )}
                />
                
                
              </div>
            )
          : (
            <div className="server-connect">
              <ConnectForm onSubmit={AuthenticationService.connect} />
            </div>
          )
      }</div>
    );
  }
}

export interface ServerProps {
  message: string;
  state: number;
  rooms: any[];
  users: User[];
}

const mapStateToProps = state => ({
  message: ServerSelectors.getMessage(state),
  state: ServerSelectors.getState(state),
  rooms: RoomsSelectors.getRooms(state),
  users: ServerSelectors.getUsers(state)
});

export default connect(mapStateToProps)(Server);