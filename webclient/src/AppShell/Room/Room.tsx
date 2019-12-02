// eslint-disable-next-line
import React, { Component } from "react";
import { connect } from "react-redux";
import { withRouter /*, RouteComponentProps */ } from "react-router-dom";
import ListItem from "@material-ui/core/ListItem";
import Paper from "@material-ui/core/Paper";

import { RoomsStateMessages, RoomsStateRooms, Selectors } from "store/rooms";

import AuthGuard from "AppShell/common/guards/AuthGuard";
import { RoomsService } from "AppShell/common/services";

import ScrollToBottomOnChanges from "AppShell/common/components/ScrollToBottomOnChanges/ScrollToBottomOnChanges";
import ThreePaneLayout from "AppShell/common/components/ThreePaneLayout/ThreePaneLayout";
import UserDisplay from "AppShell/common/components/UserDisplay/UserDisplay";
import VirtualList from "AppShell/common/components/VirtualList/VirtualList";

import Games from "./Games/Games";
import Messages from "./Messages/Messages";
import SayMessage from "./SayMessage/SayMessage";

import "./Room.css";

// @TODO (3)
class Room extends Component<any> {
  constructor(props) {
    super(props);
    this.handleRoomSay = this.handleRoomSay.bind(this);
  }

  handleRoomSay({ message }) {
    if (message) {
      const { roomId } = this.props.match.params;
      RoomsService.roomSay(roomId, message);
    }
  }

  render() {
    const { match, rooms} = this.props;
    const { roomId } = match.params;
    const room = rooms[roomId];

    const messages = this.props.messages[roomId];
    const users = room.userList;

    return (
      <div className="room-view">
        <AuthGuard />
        <ThreePaneLayout
          fixedHeight

          top={(
            <Paper className="room-view__games overflow-scroll">
              <Games room={room} />
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
              <VirtualList
                className="room-view__side-list"
                itemKey={(index, data) => users[index].name }
                items={ users.map(user => (
                  <ListItem button className="room-view__side-list__item">
                    <UserDisplay user={user} />
                  </ListItem>
                ) ) }
              />
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