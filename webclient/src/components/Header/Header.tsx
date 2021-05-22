import React, { Component } from "react";
import { connect } from "react-redux";
import { NavLink, withRouter, generatePath } from "react-router-dom";
import AppBar from "@material-ui/core/AppBar";
import Chip from "@material-ui/core/Chip";
import IconButton from "@material-ui/core/IconButton";
import Menu from "@material-ui/core/Menu";
import MenuItem from "@material-ui/core/MenuItem";
import Toolbar from "@material-ui/core/Toolbar";
import MenuRoundedIcon from '@material-ui/icons/MenuRounded';
import * as _ from "lodash";

import { AuthenticationService, RoomsService } from "api";
import {  RoomsSelectors, ServerSelectors } from "store";
import { Room, RouteEnum, User } from "types";

import "./Header.css";
import logo from "./logo.png";

class Header extends Component<HeaderProps> {
  state: HeaderState;
  options: string[] = [
    'Account',
    'Decks',
    'Replays',
  ];

  constructor(props) {
    super(props);

    this.state = {
      anchorEl: null,
    };

    this.handleMenuClick = this.handleMenuClick.bind(this);
    this.handleMenuItemClick = this.handleMenuItemClick.bind(this);
    this.handleMenuClose = this.handleMenuClose.bind(this);
  }

  componentDidUpdate(prevProps) {
    const currentRooms = this.props.joinedRooms;
    const previousRooms = prevProps.joinedRooms;

    if (currentRooms > previousRooms) {
      const { roomId } = _.difference(currentRooms, previousRooms)[0];
      this.props.history.push(generatePath(RouteEnum.ROOM, { roomId }));
    }
  }

  handleMenuClick({ target }) {
    this.setState({ anchorEl: target });
  }

  handleMenuItemClick(option: string) {
    const route = RouteEnum[option.toUpperCase()];
    this.props.history.push(generatePath(route));

    this.handleMenuClose();
  }

  handleMenuClose() {
    this.setState({ anchorEl: null });
  }

  render() {
    const { joinedRooms, server, state, user } = this.props;
    const anchorEl = this.state.anchorEl;
    let options = [ ...this.options ];

    if (user && AuthenticationService.isModerator(user)) {
      options = [
        ...options,
        'Administration',
        'Logs'
      ];
    }

    return (
      <div>
        {/*<header className="Header">*/}
        <AppBar position="static">
          <Toolbar variant="dense">
            <div className="Header__logo">
              <NavLink to={RouteEnum.SERVER}>
                <img src={logo} alt="logo" />
              </NavLink>
              { AuthenticationService.isConnected(state) && (
                <span className="Header-server__indicator"></span>
              ) }
            </div>
            { AuthenticationService.isConnected(state) && (
              <div className="Header-content">
                <nav className="Header-nav">
                  <div className="Header-nav__menu">
                    <IconButton
                      aria-label="more"
                      aria-controls="long-menu"
                      aria-haspopup="true"
                      onClick={this.handleMenuClick}
                    >
                      <MenuRoundedIcon />
                    </IconButton>
                    <Menu
                      id="long-menu"
                      anchorEl={anchorEl}
                      keepMounted
                      open={!!anchorEl}
                      onClose={this.handleMenuClose}
                      PaperProps={{
                        style: {
                          marginTop: '32px',
                          width: '20ch',
                        },
                      }}
                    >
                      {options.map((option) => (
                        <MenuItem key={option} onClick={() => this.handleMenuItemClick(option)}>
                          {option}
                        </MenuItem>
                      ))}
                    </Menu>
                  </div>
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

const Rooms = props => {

  const onLeaveRoom = (event, roomId) => {
    event.preventDefault();
    RoomsService.leaveRoom(roomId);
  };

  return <div className="temp-subnav__rooms">
    <span>Rooms: </span>
    {
      _.reduce(props.rooms, (rooms, { name, roomId}) => {
        rooms.push(
          <NavLink to={generatePath(RouteEnum.ROOM, { roomId })} className="temp-chip" key={roomId}>
            <Chip label={name} color="primary" onDelete={(event) => onLeaveRoom(event, roomId)} />
          </NavLink>
        );
        return rooms;
      }, [])
    }
  </div>
};

interface HeaderProps {
  state: number;
  server: string;
  user: User;
  joinedRooms: Room[];
  history: any;
}

interface HeaderState {
  anchorEl: Element;
}

const mapStateToProps = state => ({
  state: ServerSelectors.getState(state),
  server: ServerSelectors.getName(state),
  user: ServerSelectors.getUser(state),
  joinedRooms: RoomsSelectors.getJoinedRooms(state),
});

export default withRouter(connect(mapStateToProps)(Header));
