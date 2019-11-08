// eslint-disable-next-line
import React, { Component } from 'react';
import { connect } from 'react-redux';
import * as _ from 'lodash';

// import { ServerService } from 'common/services/data';
import webClient, { WebClient } from 'WebClient/WebClient';
import {
  Selectors as RoomsSelectors,
} from 'store/rooms';
import {
  Selectors as ServerSelectors,
} from 'store/server';

import './Server.css';

class Server extends Component<ServerProps> {
  webClient: WebClient = webClient;

  constructor(props) {
    super(props);

    this.joinRoom = this.joinRoom.bind(this);
  }

  joinRoom(roomId) {
    this.webClient.services.server.joinRoom(roomId);
  }

  rooms() {
    return _.map(this.props.rooms, ({ description, gameCount, name, permissionlevel, playerCount, roomId }) => (
      <div className="room" key={roomId}>
        <div className="room__detail name">{name}</div>
        <div className="room__detail description">{description}</div>
        <div className="room__detail permissions">{permissionlevel}</div>
        <div className="room__detail players">{playerCount}</div>
        <div className="room__detail games">{gameCount}</div>
        <div className="room__detail button">
          <button onClick={() => this.joinRoom(roomId)}>Join</button>
        </div>
      </div>
    ));
  }

  render() {
    const hasRooms = Object.keys(this.props.rooms).length > 0;

    return (
      <div>
        {hasRooms && 
          <div>
            <div className="rooms">
              <div className="rooms-header">
                <div className="rooms-header__label name">Name</div>
                <div className="rooms-header__label description">Description</div>
                <div className="rooms-header__label permissions">Permissions</div>
                <div className="rooms-header__label players">Players</div>
                <div className="rooms-header__label games">Games</div>
                <div className="rooms-header__label button"></div>
              </div>
              {
                this.rooms()
              }
            </div>
            <div className="serverMessage" dangerouslySetInnerHTML={{ __html: this.props.message }}></div>
          </div>
        }
      </div>
    )
  }
}

export interface ServerProps {
  message: string;
  rooms: any[];
}

const mapStateToProps = state => ({
  message: ServerSelectors.getMessage(state),
  rooms: RoomsSelectors.getRooms(state)
});

export default connect(mapStateToProps)(Server);