import React, { Component } from "react";
import { connect } from "react-redux";
import { NavLink, withRouter, generatePath } from "react-router-dom";
import AppBar from "@material-ui/core/AppBar";
import Chip from "@material-ui/core/Chip";
import Toolbar from "@material-ui/core/Toolbar";
import * as _ from "lodash";

import { Selectors as RoomsSelectors } from "store/rooms";
import { Selectors as ServerSelectors } from "store/server";
import { Room, User } from "types";

import { AuthenticationService } from "AppShell/common/services";
import { RouteEnum } from "AppShell/common/types";

import "./Header.css";
import logo from "./logo.png";

class Header extends Component<HeaderProps> {
  componentDidUpdate(prevProps) {
    const currentRooms = this.props.joinedRooms;
    const previousRooms = prevProps.joinedRooms;

    if (currentRooms > previousRooms) {
      const { roomId } = _.difference(currentRooms, previousRooms)[0];
      this.props.history.push(generatePath(RouteEnum.ROOM, { roomId }));
    }
  }
  render() {
    const { joinedRooms, server, state, user } = this.props;

    return (
      <div>
        {/*<header className="Header">*/}
        <AppBar position="static">
          <Toolbar variant="dense">
            <NavLink to={RouteEnum.SERVER} className="Header__logo">
              <img src={logo} alt="logo" />
            </NavLink>
            { AuthenticationService.isConnected(state) && (
              <div className="Header-content">
                <nav className="Header-nav">
                  <ul className="Header-nav__items">
                    {
                      AuthenticationService.isModerator(user) && (
                        <li className="Header-nav__item">
                          <NavLink to={RouteEnum.LOGS}>
                            <button>Logs</button>
                          </NavLink>
                        </li>
                      )
                    }
                    <li className="Header-nav__item">
                      <NavLink to={RouteEnum.SERVER} className="plain-link">
                        Server ({server})
                      </NavLink>
                    </li>
                    <NavLink to={RouteEnum.ACCOUNT} className="plain-link">
                      <div className="Header-account">
                        <span className="Header-account__name">
                          {user.name}
                        </span>
                        <span className="Header-account__indicator"></span>
                      </div>
                    </NavLink>
                  </ul>
                </nav>
              </div>
            ) }
          </Toolbar>
        </AppBar>
        <div className="temp-subnav">
          {
            !!joinedRooms.length && (
              <Rooms rooms={joinedRooms} />
            )
          }
          <div className="temp-subnav__games">
          </div>
          <div className="temp-subnav__chats">
          </div>
        </div>
      </div>
    )
  }
}

const Rooms = props => (
  <div className="temp-subnav__rooms">
    <span>Rooms: </span>
    {
      _.reduce(props.rooms, (rooms, { name, roomId}) => {
        rooms.push(
          <NavLink to={generatePath(RouteEnum.ROOM, { roomId })} className="temp-chip" key={roomId}>
            <Chip label={name} color="primary" />
          </NavLink>
        );
        return rooms;
      }, [])
    }
  </div>
)

interface HeaderProps {
  state: number;
  server: string;
  user: User;
  joinedRooms: Room[];
  history: any;
}

const mapStateToProps = state => ({
  state: ServerSelectors.getState(state),
  server: ServerSelectors.getName(state),
  user: ServerSelectors.getUser(state),
  joinedRooms: RoomsSelectors.getJoinedRooms(state)
});

export default withRouter(connect(mapStateToProps)(Header));
