// eslint-disable-next-line
import React, { Component } from 'react';
import { connect } from 'react-redux';
import { withRouter, RouteComponentProps } from "react-router-dom";

import { RoomsStateMessages, RoomsStateRooms, Selectors } from 'store/rooms';

import { RoomsService } from 'AppShell/common/services';
import SayMessage from 'AppShell/common/components/SayMessage/SayMessage';

import ScrollToBottomOnChanges from '../common/components/ScrollToBottomOnChanges/ScrollToBottomOnChanges';
import ThreePaneLayout from '../common/components/ThreePaneLayout/ThreePaneLayout';

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

    this.handleSubmit = this.handleSubmit.bind(this);
  }

  handleSubmit({ message }) {
    if (message) {
      const { roomId } = this.props.match.params;
      RoomsService.roomSay(roomId, message);
    }
  }

  hideUnavailableGame({ started, maxPlayers, playerCount }) {
    return !started && playerCount < maxPlayers;
  }

  hidePasswordProtectedGame({ withPassword }) {
    return !withPassword;
  }

  hideBuddiesOnlyGame({ onlyBuddies }) {
    return !onlyBuddies;
  }

  render() {
    const { roomId } = this.props.match.params;
    const room = this.props.rooms[roomId];

    const messages = this.props.messages[roomId];
    const games = room.gameList.filter(game => (
      this.hideUnavailableGame(game) &&
      this.hidePasswordProtectedGame(game) &&
      this.hideBuddiesOnlyGame(game)
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
                <SayMessage onSubmit={this.handleSubmit} />
              </div>
            </div>
          )}

          side={(
            <div className="room-view__side">
              USERS PANEL
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
  rooms: Selectors.getRooms(state),
});

export default withRouter(connect(mapStateToProps)(Room));