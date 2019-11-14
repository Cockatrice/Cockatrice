// eslint-disable-next-line
import React, { Component } from 'react';
import { connect } from 'react-redux';
import { withRouter /*, RouteComponentProps */ } from "react-router-dom";

import { RoomsStateMessages, RoomsStateRooms, Selectors } from 'store/rooms';
import { User } from 'types';

import { RoomsService } from 'AppShell/common/services';

import SayMessage from 'AppShell/common/components/SayMessage/SayMessage';
import ScrollToBottomOnChanges from 'AppShell/common/components/ScrollToBottomOnChanges/ScrollToBottomOnChanges';
import ThreePaneLayout from 'AppShell/common/components/ThreePaneLayout/ThreePaneLayout';
import UserDisplay from 'AppShell/common/components/UserDisplay/UserDisplay';


import Games from './Games/Games';
import Messages from './Messages/Messages';

import './Room.css';

// @TODO figure out how to properly type this component
// Component<RouteComponentProps<???, ???, ???>>
class Room extends Component<any> {
  gametypeMap = {};

  constructor(props) {
    super(props);

    const { roomId } = this.props.match.params;

    this.gametypeMap = this.props.rooms[roomId].gametypeList.reduce((map, type) => {
      map[type.gameTypeId] = type.description;
      return map;
    })

    this.handleRoomSay = this.handleRoomSay.bind(this);
  }

  handleRoomSay({ message }) {
    if (message) {
      const { roomId } = this.props.match.params;
      RoomsService.roomSay(roomId, message);
    }
  }

  isUnavailableGame({ started, maxPlayers, playerCount }) {
    return !started && playerCount < maxPlayers;
  }

  isPasswordProtectedGame({ withPassword }) {
    return !withPassword;
  }

  isBuddiesOnlyGame({ onlyBuddies }) {
    return !onlyBuddies;
  }

  render() {
    const { match, rooms} = this.props;
    const { roomId } = match.params;
    const room = rooms[roomId];

    const messages = this.props.messages[roomId];
    const users = room.userList;
    const games = room.gameList.filter(game => (
      this.isUnavailableGame(game) &&
      this.isPasswordProtectedGame(game) &&
      this.isBuddiesOnlyGame(game)
    ));

    return (
      <div className="room-view">
        <ThreePaneLayout
          top={(
            <div className="room-view__games overflow-scroll">
              <Games games={games} gameTypesMap={this.gametypeMap} />
            </div>    
          )}

          bottom={(
            <div className="room-view__messages">
              <div className="room-view__messages-content overflow-scroll">
                <ScrollToBottomOnChanges changes={messages} content={(
                  <Messages messages={messages} />
                )} />
              </div>
              <div className="room-view__messages-sayMessage">
                <SayMessage onSubmit={this.handleRoomSay} />
              </div>
            </div>
          )}

          side={(
            <div className="room-view__side">
              Users in this room: {users.length}
              { users.map(user => <UserDisplay user={user} key={user.name} />) }
            </div>
          )}
        />
      </div>
    );
  }
}

interface RoomProps {
  messages: RoomsStateMessages;
  rooms: RoomsStateRooms;
}

const mapStateToProps = state => ({
  messages: Selectors.getMessages(state),
  rooms: Selectors.getRooms(state)
});

export default withRouter(connect(mapStateToProps)(Room));