// eslint-disable-next-line
import React, { Component } from "react";
import { connect } from "react-redux";
import { withRouter } from "react-router-dom";
import List from "@material-ui/core/List";
import ListItem from "@material-ui/core/ListItem";
import Paper from "@material-ui/core/Paper";

import { Selectors as RoomsSelectors } from "store/rooms";
import { Selectors as ServerSelectors } from "store/server";
import { Room, StatusEnum, User } from "types";

import ThreePaneLayout from "AppShell/common/components/ThreePaneLayout/ThreePaneLayout";
import UserDisplay from "AppShell/common/components/UserDisplay/UserDisplay";

import { AuthenticationService } from "AppShell/common/services";

import ConnectForm from "./ConnectForm/ConnectForm";
import Rooms from "./Rooms/Rooms";

import "./Server.css";

class Server extends Component<ServerProps> {
  showDescription(state, description) {
    const isDisconnected = state === StatusEnum.DISCONNECTED;
    const hasDescription = description && !!description.length;

    return isDisconnected && hasDescription;
  }

  render() {
    const { message, rooms, joinedRooms, history, state, description, users } = this.props;
    const isConnected = AuthenticationService.isConnected(state);

    return (
      <div className="server">{
        isConnected
          ? (
              <div className="server-rooms">
                <ThreePaneLayout
                  top={(
                    <Paper>
                      <Rooms rooms={rooms} joinedRooms={joinedRooms} history={history} />
                    </Paper>
                  )}

                  bottom={(
                    <Paper className="serverMessage" dangerouslySetInnerHTML={{ __html: message }} />
                  )}

                  side={(
                    <Paper className="server-rooms__side overflow-scroll">
                      <div className="server-rooms__side-label">
                        Users connected to server: {users.length}
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
            )
          : (
            <div className="server-connect">
              <div className="server-connect__form">
                <ConnectForm onSubmit={AuthenticationService.connect} />
              </div>
              {
                this.showDescription(state, description) && (
                  <Paper className="server-connect__description">
                    {description}
                  </Paper>
                )
              }
            </div>
          )
      }</div>
    );
  }
}

export interface ServerProps {
  message: string;
  state: number;
  description: string;
  rooms: Room[];
  joinedRooms: Room[];
  users: User[];
  history: any;
}

const mapStateToProps = state => ({
  message: ServerSelectors.getMessage(state),
  state: ServerSelectors.getState(state),
  description: ServerSelectors.getDescription(state),
  rooms: RoomsSelectors.getRooms(state),
  joinedRooms: RoomsSelectors.getJoinedRooms(state),
  users: ServerSelectors.getUsers(state),
});

export default withRouter(connect(mapStateToProps)(Server));