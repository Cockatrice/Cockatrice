// eslint-disable-next-line
import React, { Component } from "react";
import { connect } from "react-redux";
import { withRouter } from "react-router-dom";

import Button from "@material-ui/core/Button";
import ListItem from "@material-ui/core/ListItem";
import Paper from "@material-ui/core/Paper";

import { Selectors as RoomsSelectors } from "store/rooms";
import { Selectors as ServerSelectors } from "store/server";
import { Room, StatusEnum, User } from "types";

import ThreePaneLayout from "AppShell/common/components/ThreePaneLayout/ThreePaneLayout";
import UserDisplay from "AppShell/common/components/UserDisplay/UserDisplay";
import VirtualList from "AppShell/common/components/VirtualList/VirtualList";

import { AuthenticationService } from "AppShell/common/services";

import ConnectForm from "./ConnectForm/ConnectForm";
import RegisterForm from "./RegisterForm/RegisterForm";
import Rooms from "./Rooms/Rooms";

import "./Server.css";

class Server extends Component<ServerProps, ServerState> {
  constructor(props) {
    super(props);

    this.showDescription = this.showDescription.bind(this);
    this.showRegisterForm = this.showRegisterForm.bind(this);
    this.hideRegisterForm = this.hideRegisterForm.bind(this);
    this.onRegister = this.onRegister.bind(this);

    this.state = {
      register: false
    };
  }

  showDescription(state, description) {
    const isDisconnected = state === StatusEnum.DISCONNECTED;
    const hasDescription = description && !!description.length;

    return isDisconnected && hasDescription;
  }

  showRegisterForm() {
    this.setState({register: true});
  }

  hideRegisterForm() {
    this.setState({register: false});
  }

  onRegister(fields) {
    console.log("register", fields);
  }

  render() {
    const { message, rooms, joinedRooms, history, state, description, users } = this.props;
    const { register } = this.state;
    const isConnected = AuthenticationService.isConnected(state);

    return (
      <div className="server">
            {
              isConnected
                ? ( <ServerRooms rooms={rooms} joinedRooms={joinedRooms} history={history} message={message} users={users} /> )
                : (
                  <div className="server-connect">
                    <Paper className="server-connect__form">
                      {
                        register
                          ? ( <Register connect={this.hideRegisterForm} onRegister={this.onRegister} /> )
                          : ( <Connect register={this.showRegisterForm} /> )
                      }
                    </Paper>
                  </div>
                )
            }
          {
            !isConnected && this.showDescription(state, description) && (
              <Paper className="server-connect__description">
                {description}
              </Paper>
            )
          }
      </div>
    );
  }
}

const ServerRooms = ({ rooms, joinedRooms, history, message, users}) => (
  <div className="server-rooms">
    <ThreePaneLayout
      top={(
        <Paper className="serverRoomWrapper overflow-scroll">
          <Rooms rooms={rooms} joinedRooms={joinedRooms} history={history} />
        </Paper>
      )}

      bottom={(
        <Paper className="serverMessage overflow-scroll" dangerouslySetInnerHTML={{ __html: message }} />
      )}

      side={(
        <Paper className="server-rooms__side overflow-scroll">
          <div className="server-rooms__side-label">
            Users connected to server: {users.length}
          </div>
          <VirtualList
            itemKey={(index, data) => users[index].name }
            items={ users.map(user => (
              <ListItem button dense>
                <UserDisplay user={user} />
              </ListItem>
            ) ) }
          />
        </Paper>
      )}
    />
  </div>
);

const Connect = ({register}) => (
  <div className="form-wrapper">
    <ConnectForm onSubmit={AuthenticationService.connect} />
    {/*{<Button variant="outlined" onClick={register}>Register</Button>}*/}
  </div>
);

const Register = ({ onRegister, connect }) => (
  <div className="form-wrapper">
    <RegisterForm onSubmit={event => onRegister(event)} />
    <Button variant="outlined" onClick={connect}>Connect</Button>
  </div>
);

interface ServerProps {
  message: string;
  state: number;
  description: string;
  rooms: Room[];
  joinedRooms: Room[];
  users: User[];
  history: any;
}

interface ServerState {
  register: boolean;
}

const mapStateToProps = state => ({
  message: ServerSelectors.getMessage(state),
  state: ServerSelectors.getState(state),
  description: ServerSelectors.getDescription(state),
  rooms: RoomsSelectors.getRooms(state),
  joinedRooms: RoomsSelectors.getJoinedRooms(state),
  users: ServerSelectors.getUsers(state)
});

export default withRouter(connect(mapStateToProps)(Server));