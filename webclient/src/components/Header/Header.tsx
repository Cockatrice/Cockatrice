import React, { Component } from "react";
import { connect } from "react-redux";
import { NavLink, withRouter, generatePath } from "react-router-dom";
import AppBar from "@material-ui/core/AppBar";
import IconButton from "@material-ui/core/IconButton";
import Menu from "@material-ui/core/Menu";
import MenuItem from "@material-ui/core/MenuItem";
import Toolbar from "@material-ui/core/Toolbar";
import ArrowDropDownIcon from '@material-ui/icons/ArrowDropDown';
import CloseIcon from '@material-ui/icons/Close';
import MailOutlineRoundedIcon from '@material-ui/icons/MailOutline';
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
    'Replays',
  ];

  constructor(props) {
    super(props);

    this.state = { anchorEl: null };

    this.handleMenuOpen = this.handleMenuOpen.bind(this);
    this.handleMenuItemClick = this.handleMenuItemClick.bind(this);
    this.handleMenuClose = this.handleMenuClose.bind(this);
    this.leaveRoom = this.leaveRoom.bind(this);
  }

  componentDidUpdate(prevProps) {
    const currentRooms = this.props.joinedRooms;
    const previousRooms = prevProps.joinedRooms;

    if (currentRooms > previousRooms) {
      const { roomId } = _.difference(currentRooms, previousRooms)[0];
      this.props.history.push(generatePath(RouteEnum.ROOM, { roomId }));
    }
  }

  handleMenuOpen(event) {
    this.setState({ anchorEl: event.target });
  }

  handleMenuItemClick(option: string) {
    const route = RouteEnum[option.toUpperCase()];
    this.props.history.push(generatePath(route));
  }

  handleMenuClose() {
    this.setState({ anchorEl: null });
  }

  leaveRoom(event, roomId) {
    event.preventDefault();
    RoomsService.leaveRoom(roomId);
  };

  render() {
    const { joinedRooms, state, user } = this.props;
    const { anchorEl } = this.state;

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
                  <div className="Header-nav__link">
                    <NavLink
                      className="Header-nav__link-btn"
                      to={ joinedRooms.length ? generatePath(RouteEnum.ROOM, { roomId: joinedRooms[0].roomId }) : RouteEnum.SERVER }
                    >
                      Rooms
                      <ArrowDropDownIcon className="Header-nav__link-btn__icon" fontSize="small" />
                    </NavLink>
                    <div className="Header-nav__link-menu">
                      {joinedRooms.map(({ name, roomId }) => (
                        <MenuItem className="Header-nav__link-menu__item" key={roomId}>
                          <NavLink className="Header-nav__link-menu__btn" to={ generatePath(RouteEnum.ROOM, { roomId: roomId }) }>
                            {name}

                            <IconButton size="small" edge="end" onClick={event => this.leaveRoom(event, roomId)}>
                              <CloseIcon style={{ fontSize: 10, color: 'white' }} />
                            </IconButton>
                          </NavLink>
                        </MenuItem>
                      ))}
                    </div>
                  </div>
                  <div className="Header-nav__link">
                    <NavLink className="Header-nav__link-btn" to={ RouteEnum.GAME }>
                      Games
                      <ArrowDropDownIcon className="Header-nav__link-btn__icon" fontSize="small" />
                    </NavLink>
                  </div>
                  <div className="Header-nav__link">
                    <NavLink className="Header-nav__link-btn" to={ RouteEnum.DECKS }>
                      Decks
                      <ArrowDropDownIcon className="Header-nav__link-btn__icon" fontSize="small" />
                    </NavLink>
                  </div>
                </nav>
                <div className="Header-nav__actions">
                  <div className="Header-nav__action">
                    <IconButton>
                      <MailOutlineRoundedIcon style={{ color: 'inherit' }} />
                    </IconButton>
                  </div>
                  <div className="Header-nav__action">
                    <IconButton onClick={this.handleMenuOpen}>
                      <MenuRoundedIcon style={{ color: 'inherit' }} />
                    </IconButton>
                    <Menu
                      anchorEl={anchorEl}
                      keepMounted
                      open={!!anchorEl}
                      onClose={() => this.handleMenuClose()}
                      PaperProps={{
                        style: {
                          marginTop: '32px',
                          width: '20ch',
                        },
                      }}
                    >
                      {options.map((option) => (
                        <MenuItem key={option} onClick={(event) => this.handleMenuItemClick(option)}>
                          {option}
                        </MenuItem>
                      ))}
                    </Menu>
                  </div>
                </div>
              </nav>
            </div>
          ) }
        </Toolbar>
      </AppBar>
    )
  }
}

interface HeaderProps {
  state: number;
  server: string;
  user: User;
  joinedRooms: Room[];
  history: any;
}

interface HeaderState {
  anchorEl: Element
}

const mapStateToProps = state => ({
  state: ServerSelectors.getState(state),
  server: ServerSelectors.getName(state),
  user: ServerSelectors.getUser(state),
  joinedRooms: RoomsSelectors.getJoinedRooms(state),
});

export default withRouter(connect(mapStateToProps)(Header));
