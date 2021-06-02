import React, { Component } from "react";
import { connect } from "react-redux";
import { NavLink, withRouter, generatePath } from "react-router-dom";
import AppBar from "@material-ui/core/AppBar";
import Chip from "@material-ui/core/Chip";
import IconButton from "@material-ui/core/IconButton";
import Menu from "@material-ui/core/Menu";
import MenuItem from "@material-ui/core/MenuItem";
import Toolbar from "@material-ui/core/Toolbar";
import ArrowDropDownIcon from '@material-ui/icons/ArrowDropDown';
import MenuRoundedIcon from '@material-ui/icons/MenuRounded';
import * as _ from "lodash";

import { AuthenticationService, RoomsService } from "api";
import {  RoomsSelectors, ServerSelectors } from "store";
import { routeWithParams, Room, RouteEnum, User } from "types";

import "./Header.css";
import logo from "./logo.png";

enum HeaderMenu {
  MAIN = 'MAIN',
  ROOMS = 'ROOMS',
  GAMES = 'GAMES',
  DECKS = 'DECKS',
}

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
      anchorEls: {},
    };

    this.handleMenuOpen = this.handleMenuOpen.bind(this);
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

  handleMenuOpen(anchor: HeaderMenu, event) {
    console.log(event);
    console.log(anchor, event.target);

    this.setState({
      anchorEls: { [anchor]: event.target }
    });
  }

  handleMenuItemClick(anchor: HeaderMenu, option: string) {
    const route = RouteEnum[option.toUpperCase()];
    this.props.history.push(generatePath(route));

    this.handleMenuClose(anchor);
  }

  handleMenuClose(anchor: HeaderMenu) {
    this.setState({
      anchorEls: { [anchor]: null }
    });
  }

  render() {
    const { joinedRooms, server, state, user } = this.props;
    const { anchorEls } = this.state;

    let options = [ ...this.options ];

    if (user && AuthenticationService.isModerator(user)) {
      options = [
        ...options,
        'Administration',
        'Logs'
      ];
    }

    return (
      <AppBar className="Header" position="static">
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
                <nav className="Header-nav__links">
                  <div onMouseLeave={(event) => this.handleMenuClose(HeaderMenu.ROOMS) }>
                    <NavLink
                      className={ 'Header-nav__link' + ( !joinedRooms.length ? ' disabled-link' : '' ) }
                      to={ joinedRooms.length ? routeWithParams(RouteEnum.ROOM, { roomId: joinedRooms[0].roomId }) : '' }
                    >
                      Rooms
                      <ArrowDropDownIcon fontSize="small" onMouseOver={(event) => this.handleMenuOpen(HeaderMenu.ROOMS, event) } />
                    </NavLink>
                    <Menu
                      anchorEl={anchorEls[HeaderMenu.ROOMS]}
                      keepMounted
                      open={!!anchorEls[HeaderMenu.ROOMS]}
                      onClose={() => this.handleMenuClose(HeaderMenu.ROOMS)}
                      PaperProps={{
                        style: {
                          marginTop: '32px',
                          marginLeft: '-45px',
                          width: '20ch',
                        },
                      }}
                    >
                      {joinedRooms.map(({ name, roomId }) => (
                        <MenuItem key={roomId} onClick={() => this.handleMenuClose(HeaderMenu.ROOMS)}>
                          <NavLink to={ routeWithParams(RouteEnum.ROOM, { roomId: roomId }) }>
                            {name}
                          </NavLink>
                        </MenuItem>
                      ))}
                    </Menu>
                  </div>
                  <div>
                    <NavLink className="Header-nav__link" to={ RouteEnum.GAME }>
                      Games
                      <ArrowDropDownIcon fontSize="small" />
                    </NavLink>
                  </div>
                  <div>
                    <NavLink className="Header-nav__link" to={ RouteEnum.DECKS }>
                      Decks
                      <ArrowDropDownIcon fontSize="small" />
                    </NavLink>
                  </div>
                </nav>
                <div className="Header-nav__menu">
                  <IconButton onClick={(event) => this.handleMenuOpen(HeaderMenu.MAIN, event)}>
                    <MenuRoundedIcon />
                  </IconButton>
                  <Menu
                    anchorEl={anchorEls[HeaderMenu.MAIN]}
                    keepMounted
                    open={!!anchorEls[HeaderMenu.MAIN]}
                    onClose={() => this.handleMenuClose(HeaderMenu.MAIN)}
                    PaperProps={{
                      style: {
                        marginTop: '32px',
                        width: '20ch',
                      },
                    }}
                  >
                    {options.map((option) => (
                      <MenuItem key={option} onClick={(event) => this.handleMenuItemClick(HeaderMenu.MAIN, option)}>
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
  anchorEls: {
    [key: string]: Element
  };
}

const mapStateToProps = state => ({
  state: ServerSelectors.getState(state),
  server: ServerSelectors.getName(state),
  user: ServerSelectors.getUser(state),
  joinedRooms: RoomsSelectors.getJoinedRooms(state),
});

export default withRouter(connect(mapStateToProps)(Header));
