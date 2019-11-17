// eslint-disable-next-line
import React, { Component } from 'react';
import { connect } from 'react-redux';
import { withRouter /*, RouteComponentProps */ } from "react-router-dom";
import List from '@material-ui/core/List';
import ListItem from '@material-ui/core/ListItem';
import Paper from '@material-ui/core/Paper';

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

  // @TODO clear input onSubmit
  handleRoomSay(raw) {
    const { message } = raw;
    console.log('raw', raw);
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
          fixedHeight

          top={(
            <Paper className="room-view__games overflow-scroll">
              <Games games={games} gameTypesMap={this.gametypeMap} />
            </Paper>    
          )}

          bottom={(
            <div className="room-view__messages">
              <Paper className="room-view__messages-content overflow-scroll">
                <ScrollToBottomOnChanges changes={messages} content={(
                  <Messages messages={messages} />
                )} />
              </Paper>
              <Paper className="room-view__messages-sayMessage">
                <SayMessage onSubmit={this.handleRoomSay} />
              </Paper>
            </div>
          )}

          side={(
            <Paper className="room-view__side overflow-scroll">
              <div className="room-view__side-label">
                Users in this room: {users.length}
              </div>
              <List dense={true}>
                { users.map(user => (
                  <ListItem button key={user.name}>
                    <UserDisplay user={user} key={user.name} />
                  </ListItem>
                ) ) }
              </List>
            </Paper>
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