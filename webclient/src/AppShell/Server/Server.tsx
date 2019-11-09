// eslint-disable-next-line
import React, { Component } from 'react';
import { connect } from 'react-redux';

import { Selectors as RoomsSelectors } from 'store/rooms';
import { Selectors as ServerSelectors } from 'store/server';

import { AuthenticationService } from 'AppShell/common/services';

import ConnectForm from './ConnectForm/ConnectForm';
import Rooms from './Rooms/Rooms';

import './Server.css';

class Server extends Component<ServerProps> {
  render() {
    const isConnected = AuthenticationService.isConnected(this.props.state);

    return (
      <div className="server">{
        isConnected
          ? (
              <div className="server-rooms">
                <Rooms rooms={this.props.rooms} />
                <div className="serverMessage" dangerouslySetInnerHTML={{ __html: this.props.message }}></div>
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
}

const mapStateToProps = state => ({
  message: ServerSelectors.getMessage(state),
  state: ServerSelectors.getState(state),
  rooms: RoomsSelectors.getRooms(state)
});

export default connect(mapStateToProps)(Server);